/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// setup the basic structures so they are ready to be filled in
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_setup( metamap *mmap )
{
bool ret;

	ret=true;

	memset(mmap,0,sizeof(metamap));	// clear data

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean/free anything that may have been allocated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_clean( metamap *mmap )
{
bool ret;

	ret=true;

	if(mmap->tiles)
	{
		free(mmap->tiles);
		mmap->tiles=0;
		mmap->numof_tiles=0;
	}

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// update the image info cache
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void mmap_update_cache( metamap *mmap )
{
	ilBindImage(mmap->image);

	mmap->w=ilGetInteger(IL_IMAGE_WIDTH);
	mmap->h=ilGetInteger(IL_IMAGE_HEIGHT);

	mmap->format=ilGetInteger(IL_IMAGE_FORMAT);
	mmap->type=ilGetInteger(IL_IMAGE_TYPE);

	mmap->sizeof_pix=ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

	mmap->span=mmap->sizeof_pix*mmap->w;

	mmap->data=ilGetData();
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check we have tiles and image data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_check( metamap *mmap )
{
	if(!mmap->tiles) return false;
	if(!mmap->data) return false;

	return true;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate and fill in a tilemap using tiles of the given size
// doesnt reduce in anyway so all it does is produce an array which can later be collapsed
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_cutup( metamap *mmap , s32 w, s32 h)
{
bool ret;
s32 x,y;
mmap_tile *tp;

	ret=false;

	mmap_update_cache(mmap);

	mmap->tw=mmap->w/w;
	mmap->th=mmap->h/h;


// alloc tiles
	mmap->tiles=(mmap_tile*)calloc(mmap->tw*mmap->th,sizeof(mmap_tile));
	if(!mmap->tiles) goto bogus;


	mmap->numof_tiles=mmap->tw*mmap->th;


// fill in tiles
	tp=mmap->tiles;
	for(y=0;y<mmap->th;y++)
	{
		for(x=0;x<mmap->tw;x++,tp++)
		{
			tp->w=w;
			tp->h=h;
			tp->x=x*tp->w;
			tp->y=y*tp->h;
			tp->base=mmap;
		}
	}


	ret=true;

bogus:
	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce master to master chains down to a single pointer to the final data.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_unchain( metamap *mmap )
{
bool ret;
mmap_tile *tp;

	ret=false;

	mmap_update_cache(mmap);
	if(!mmap_check(mmap)) goto bogus;

	for( tp=mmap->tiles ; tp<mmap->tiles+mmap->numof_tiles ; tp++ )
	{
		if(tp->master)
		{
			while(tp->master->master)
			{
				tp->master=tp->master->master;
			}
		}
	}

	ret=true;
bogus:
	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles with all other tiles, and mark duplicates as such.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_merge( metamap *mmap )
{
bool ret;
mmap_tile *pa;
mmap_tile *pb;

	ret=false;

	mmap_update_cache(mmap);
	if(!mmap_check(mmap)) goto bogus;

	for( pa=mmap->tiles ; pa<mmap->tiles+mmap->numof_tiles ; pa++ )
	{
		if(pa->master) continue;

		for( pb=mmap->tiles ; pb<pa ; pb++ )
		{
			if(pb->master) continue;
			if(metamap_char_compare(pa,pb)) { pa->master=pb; break; }
		}
	}

	ret=true;
bogus:
	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Shrink each tile, remove alpha from around the outside.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_shrink( metamap *mmap )
{
bool ret;
mmap_tile *pa;

	ret=false;

	mmap_update_cache(mmap);
	if(!mmap_check(mmap)) goto bogus;

	for( pa=mmap->tiles ; pa<mmap->tiles+mmap->numof_tiles ; pa++ )
	{
		if(pa->master) continue;

		metamap_char_shrink(pa);
	}

	ret=true;
bogus:
	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles on one metamap with all tiles on another metamap
// and map them acordingly
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_keymap( metamap *a , metamap *b )
{
bool ret;
mmap_tile *pa;
mmap_tile *pb;

	ret=false;

	mmap_update_cache(a);
	mmap_update_cache(b);
	if(!mmap_check(a)) goto bogus;
	if(!mmap_check(b)) goto bogus;


	for( pa=a->tiles ; pa<a->tiles+a->numof_tiles ; pa++ )
	{
		for( pb=b->tiles ; pb<b->tiles+b->numof_tiles ; pb++ )
		{
			if(pb->master) continue;
			if(metamap_char_compare(pa,pb)) { pa->master=pb; break; }
		}
	}

	ret=true;
bogus:
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// layout the contents of one metamap onto another, skip tiles that have a zero size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_layout( metamap *a , metamap *b , s32 border)
{
bool ret;
mmap_tile *pa;
mmap_tile *pb;

s32 count;

// used to layout the tiles
s32 max_width;
s32 max_height;

s32 biggest_y;

s32 x,y;

s32 height_loop;

	ret=false;

	mmap_update_cache(a);
	mmap_update_cache(b);
	if(!mmap_check(a)) goto bogus;
//	if(!mmap_check(b)) goto bogus; //no need to check b, we will be creating it


//	base_tile_width=a->w/a->tw;
//	base_tile_height=a->h/a->th;


// first count number of active chars in a and the max tile widths/height

	count=0;
	biggest_y=0;
	for( pa=a->tiles ; pa<a->tiles+a->numof_tiles ; pa++ )
	{
		if(pa->w&&pa->h) // active
		{
			if(pa->h>biggest_y) { biggest_y=pa->h; }
			count++;
		}
	}

	if(!count) goto bogus;

// free? then allocate tiles in b

	mmap_clean(b);

	b->tiles=(mmap_tile*)calloc(count,sizeof(mmap_tile));
	if(!b->tiles) goto bogus;
	b->numof_tiles=count;
	b->tw=0;
	b->th=0;

// place tiles from a onto b, in a retarded and simple way... will do for the now

	max_height=0;
	max_width=0;
	x=0;
	y=0;
	pb=b->tiles;
	for( height_loop=biggest_y ; height_loop > 0 ; height_loop-- ) // loop from bigest to smallest
	for( pa=a->tiles ; pa<a->tiles+a->numof_tiles ; pa++ )
	{
		if(pa->w) // active
		{
			if(pa->h==height_loop) // only do items of this height
			{

				if( (x+pa->w+border*2) > b->w ) // new line
				{
					x=0;
					y+=max_height+border*2;

					max_height=0;
					max_width=0;
				}

				if( (y+pa->h+border*2) > b->h ) // overflow
				{
					goto bogus;
				}

				if(pa->w>max_width) { max_width=pa->w; }
				if(pa->h>max_height) { max_height=pa->h; }

				pb->master=pa;
				pb->base=b;
				pb->hx=pa->hx;
				pb->hy=pa->hy;
				pb->w=pa->w;
				pb->h=pa->h;
				pb->x=x+border;
				pb->y=y+border;

				x+=pa->w+border*2;

				pb++;
			}
		}
	}

// go through again and blit the tiles across as well

	ilBindImage(b->image);
	for( pb=b->tiles ; pb<b->tiles+b->numof_tiles ; pb++ )
	{
		if(pb->w)
		{
			ilBlit(a->image,pb->x,pb->y,0,pb->master->x,pb->master->y,0,pb->w,pb->h,1);
		}
	}


	ret=true;
bogus:
	return ret;
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save the metamap info as an XTX data file
//
// pass in the layedout page in mmap_layout and the original page in mmap_master in order to generate a mapping table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool mmap_save_XTX( metamap *mmap_layout , metamap *mmap_master ,const char *filename)
{

XTX0_header		xtxhead[1];
XTX0_info		xtxinfo[1];
XTX0_area		xtxarea[1];

FILE *fp=0;

bool ret;

mmap_tile *tile;
mmap_tile *tile2;


s32 chunk_info			;
s32 chunk_area			;
s32 chunk_map			;
s32 chunk_kern			;

s32 chunk_pos;
u16 id;

	ret=false;

	if(!(fp=fopen(filename,"wb")))
	{
		DBG_Error("Failed to open output file \"%s\".\n",filename);
		goto bogus;
	}

	xtxinfo->numof_areas=mmap_layout->numof_tiles;
	xtxinfo->numof_maps=mmap_master->numof_tiles;
	xtxinfo->numof_kerns=0;


	chunk_info			=	( 1								*	sizeof(XTX0_info)			) ;
	chunk_area			=	( xtxinfo->numof_areas			*	sizeof(XTX0_area)			) ;
	chunk_map			=	( xtxinfo->numof_maps			*	sizeof(u16)					) ;
	chunk_kern			=	( xtxinfo->numof_kerns			*	sizeof(u8)					) ;
	chunk_pos			=	0;

	xtxhead->id=U32_ID4_XTX0;
	xtxhead->version=XTX0_VERSION;
	xtxhead->filesize= chunk_info + chunk_area + chunk_map + chunk_kern ;


	xtxinfo->areas=(XTX0_area*)			(	chunk_info																		);
	xtxinfo->maps=(u16*)				(	chunk_info+chunk_area															);
	xtxinfo->kerns=(u8*)				(	chunk_info+chunk_area+chunk_map													);

//
// dump out header
//
	xtxhead->twiddle();
	if(1!=fwrite((void*)xtxhead,12,1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xtxhead->twiddle();

//
// dump out info
//
	xtxinfo->twiddle();
	if(1!=fwrite((void*)xtxinfo,sizeof(xtxinfo),1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xtxinfo->twiddle();

//
// dump out area infos
//
	for( tile=mmap_layout->tiles ; tile<mmap_layout->tiles+mmap_layout->numof_tiles ; tile++ )
	{
		xtxarea->x=((s16)(tile->x));///((f32)(mmap_layout->w));
		xtxarea->y=((s16)(tile->y));///((f32)(mmap_layout->h));

		xtxarea->w=((s16)(tile->w));///((f32)(mmap_layout->w));
		xtxarea->h=((s16)(tile->h));///((f32)(mmap_layout->h));

		xtxarea->hx=((s16)(tile->hx));
		xtxarea->hy=((s16)(tile->hy));

		xtxarea->twiddle();
		if(1!=fwrite((void*)xtxarea,sizeof(xtxarea),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xtxarea->twiddle();
	}

	for( tile=mmap_master->tiles ; tile<mmap_master->tiles+mmap_master->numof_tiles ; tile++ )
	{
		id=0; // not a tile

		for( tile2=mmap_layout->tiles ; tile2<mmap_layout->tiles+mmap_layout->numof_tiles ; tile2++ )
		{
			if(tile2->master==tile)
			{
				id=1+tile2-mmap_layout->tiles;
				break;
			}
		}

		if(1!=fwrite((void*)&id,sizeof(id),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
	}


	ret=true;
bogus:
	if(fp) { fclose(fp); }
	return ret;

}

































































































#if 0


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// setup the basic structures so they are ready to be filled in
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_setup( metamap *mmap )
{
bool ret;
metamap_layer *laya;


	ret=true;

	memset(mmap,0,sizeof(metamap));	// clear data


	for(laya=mmap->layers;laya<mmap->layers+MAX_METAMAP_LAYERS;laya++)
	{
		ilGenImages(1, laya->ImgId);
	}

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean/free anything that may have been allocated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_clean( metamap *mmap )
{
bool ret;
metamap_layer *laya;

	ret=true;

	for(laya=mmap->layers;laya<mmap->layers+MAX_METAMAP_LAYERS;laya++)
	{
		ilDeleteImages(1, laya->ImgId);
	}

	if(mmap->map)
	{
		free(mmap->map);
		mmap->map=0;
	}
	if(mmap->map8)
	{
		free(mmap->map8);
		mmap->map8=0;
	}

	if(mmap->indexmap)
	{
		free(mmap->indexmap);
		mmap->indexmap=0;
	}

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load and check all the active layers
// and allocate and extra information that depends on the sizeof the input bitmaps
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_load( metamap *mmap )
{
bool ret;
metamap_layer *laya;
s32 lid;

s32 tx,ty;
metamap_tile *tp;


	ret=false;

	for(laya=mmap->layers , lid=0 ; laya<mmap->layers+mmap->num_layers ; laya++ , lid++)
	{
		ilBindImage(laya->ImgId[0]);

		if(!ilLoadImage(laya->fname->path)) { printf("Failed to load %s\n",laya->fname->path); goto bogus; }

		if(lid==0)											// get width,height
		{
			mmap->width=ilGetInteger(IL_IMAGE_WIDTH);
			mmap->height=ilGetInteger(IL_IMAGE_HEIGHT);
			mmap->width_in_tiles=(mmap->width+mmap->tile_width-1)/mmap->tile_width;
			mmap->height_in_tiles=(mmap->height+mmap->tile_height-1)/mmap->tile_height;
			mmap->width_in_8tiles=(mmap->width+8-1)/8;
			mmap->height_in_8tiles=(mmap->height+8-1)/8;

			mmap->map=(struct metamap_tile *)calloc(sizeof(metamap_tile),mmap->width_in_tiles*mmap->height_in_tiles);
			mmap->map8=(struct metamap_tile *)calloc(sizeof(metamap_tile)*mmap->num_layers,mmap->width_in_8tiles*mmap->height_in_8tiles);
		}
		else															//check width,height
		{
			if(mmap->width!=ilGetInteger(IL_IMAGE_WIDTH))
			{
				printf("\n**** \"%s\" ****\n\n",laya->fname->path);
				printf("\n**** ERROR all input files must be same width ****\n\n");
				goto bogus;
			}
			if(mmap->height!=ilGetInteger(IL_IMAGE_HEIGHT))
			{
				printf("\n**** \"%s\" ****\n\n",laya->fname->path);
				printf("\n**** ERROR all input files must be same height ****\n\n");
				goto bogus;
			}
		}

		laya->data=(u8*)ilGetData();

		for(ty=0;ty<mmap->height_in_8tiles;ty++)
		{
			for( tx=0 , tp=mmap->map8+(lid*mmap->width_in_8tiles*mmap->height_in_8tiles)+(ty*mmap->width_in_8tiles) ; tx<mmap->width_in_8tiles ; tx++ , tp++)
			{
				tp->flip_x=false;
				tp->flip_y=false;
				tp->mapID=0; // do not reference till after we got us a pal
				tp->layaID=lid;
				tp->master=0;
				tp->x=tx;
				tp->y=ty;
				tp->pal=metamap_getchar_0f_pal(mmap,tp);
				tp->mapID=mmap->ID;
			}
		}

	}

	for(ty=0;ty<mmap->height_in_tiles;ty++)
	{
		for( tx=0 , tp=mmap->map+(ty*mmap->width_in_tiles) ; tx<mmap->width_in_tiles ; tx++ , tp++)
		{
			tp->flip_x=false;
			tp->flip_y=false;
			tp->mapID=mmap->ID;
			tp->layaID=lid;
			tp->master=0;
			tp->x=tx;
			tp->y=ty;
		}
	}


	ret=true;
bogus:
	return ret;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set any totally obscured 8x8 tile to 0, cut down on hidden map data
//
// you cant trust artists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool mmap_killhidden(metamap *mp , s32 lay_bot , s32 lay_top)
{
s32 tx,ty;
metamap_tile *tp;
s32 lay_siz;


	lay_siz=mp->width_in_8tiles*mp->height_in_8tiles;

	for(ty=0;ty<mp->height_in_8tiles;ty++)
	{
		for( tx=0 , tp=mp->map8+(ty*mp->width_in_8tiles) ; tx<mp->width_in_8tiles ; tx++ , tp++)
		{
			if( 2 == metamap_testchar_0f(mp,tp+(lay_siz*lay_top)) ) // if top is totally opaque?
			{
				metamap_clearchar(mp,tp+(lay_siz*lay_bot)); // kill bot
			}
		}
	}

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce character pointers into unique ones for this map, using fliping if allowed
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_map8( metamap *mmap )
{
bool ret;

metamap_tile *tp;
metamap_tile *tp1;
metamap_tile *tp2;

s32 chunk_size;

s32 full_size;

metamap_tile *tmin,*tmax;

	ret=true;

	chunk_size=8192;
	full_size=(mmap->width_in_8tiles*mmap->height_in_8tiles*mmap->num_layers);

	for( tp1=mmap->map8 ; tp1 < mmap->map8+full_size ; tp1+=chunk_size )
	{
		tmin=tp1;
		tmax=tp1+chunk_size;

		if(tmax>mmap->map8+full_size)
		{
			tmax=mmap->map8+full_size;
		}

		for(tp=tmin;tp<tmax;tp++)
		{
			if(tp->master) continue;

			for( tp2=tmin ; tp2 < tp ; tp2++ )
			{
				if(tp2->master) continue;
				if(mmap_tile_compare_and_map8(mmap,tp,tp2,8,8)) break;
			}
		}
	}

	tmin=mmap->map8;
	tmax=mmap->map8+full_size;
	for(tp=tmin;tp<tmax;tp++)
	{
		if(tp->master) continue;

		for( tp2=tmin ; tp2 < tp ; tp2++ )
		{
			if(tp2->master) continue;
			if(mmap_tile_compare_and_map8(mmap,tp,tp2,8,8)) break;
		}
	}

// merge em so we dont point to something that points to something that points to etc etc etc

	tmin=mmap->map8;
	tmax=mmap->map8+full_size;
	for(tp=tmin;tp<tmax;tp++)
	{
		if(tp->master)
		for( tp2=tp->master ; tp2->master ; tp2=tp2->master )
		{
			tp->flip_x^=tp2->flip_x;		// do multi flipy flips
			tp->flip_y^=tp2->flip_y;
			tp->layaID=tp2->layaID;
			tp->mapID=tp2->mapID;
			tp->master=tp2->master;
			tp->x=tp2->x;
			tp->y=tp2->y;
		}
	}

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce character pointers into unique ones for this map, using fliping if allowed
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_map( metamap *mmap )
{
bool ret;

metamap_tile *tp;

metamap_tile *tp2;

	ret=true;

	for( tp=mmap->map ; tp < mmap->map+(mmap->width_in_tiles*mmap->height_in_tiles) ; tp++ )
	{
		for( tp2=mmap->map ; tp2 < tp ; tp2++ )
		{
			if( (tp2->master) || (tp->master) )	continue;// only bitcompare masters
			if(mmap_tile_compare_and_map(mmap,tp,tp2,mmap->tile_width,mmap->tile_height)) break;
		}
	}

	return ret;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// merge an array of metamaps so that there are no unique characters in it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_merge( metamap *mmap, s32 numof_mmaps )
{
bool ret;

metamap_tile *tp;
//metamap_tile *tp1;
metamap_tile *tp2;


//metamap_tile *tmin,*tmax;

metamap *mp1;
s32 mp1_full_size;
metamap *mp2;
s32 mp2_full_size;

	ret=true;

// set the map ID of all tiles correctly for this array
	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_8tiles*mp1->height_in_8tiles*mp1->num_layers);

		for(tp=mp1->map8;tp<mp1->map8+mp1_full_size;tp++)
		{
			tp->mapID=mp1-mmap;
		}
	}
	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_tiles*mp1->height_in_tiles);

		for(tp=mp1->map;tp<mp1->map+mp1_full_size;tp++)
		{
			tp->mapID=mp1-mmap;
		}
	}

// map the 8tiles

	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_8tiles*mp1->height_in_8tiles*mp1->num_layers);

		for(tp=mp1->map8;tp<mp1->map8+mp1_full_size;tp++)
		{
			if(tp->master) continue;

			for( mp2=mmap ; mp2<=mp1 ; mp2++ )
			{
				if(tp->master) break;

				mp2_full_size=(mp2->width_in_8tiles*mp2->height_in_8tiles*mp2->num_layers);

				for( tp2=mp2->map8 ; tp2 < mp2->map8+mp2_full_size ; tp2++ )
				{
					if(mp1==mp2) if(tp2>=tp) break;
					if(tp2->master) continue;
					if(mmap_tile_compare_and_map8(mmap,tp,tp2,8,8)) break;
				}
			}
		}
	}

// merge em so we dont point to something that points to something that points to etc etc etc

	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_8tiles*mp1->height_in_8tiles*mp1->num_layers);

		for(tp=mp1->map8;tp<mp1->map8+mp1_full_size;tp++)
		{
			if(tp->master)
			for( tp2=tp->master ; tp2->master ; tp2=tp2->master )
			{
				tp->flip_x^=tp2->flip_x;		// do multi flipy flips
				tp->flip_y^=tp2->flip_y;
				tp->layaID=tp2->layaID;
				tp->mapID=tp2->mapID;
				tp->master=tp2->master;
				tp->x=tp2->x;
				tp->y=tp2->y;
			}
		}
	}

// now map the meta tiles

	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_tiles*mp1->height_in_tiles);

		for(tp=mp1->map;tp<mp1->map+mp1_full_size;tp++)
		{
			if(tp->master) continue;

			for( mp2=mmap ; mp2<=mp1 ; mp2++ )
			{
				if(tp->master) break;

				mp2_full_size=(mp2->width_in_tiles*mp2->height_in_tiles);

				for( tp2=mp2->map ; tp2 < mp2->map+mp2_full_size ; tp2++ )
				{
					if(mp1==mp2) if(tp2>=tp) break;
					if(tp2->master) continue;
					if(mmap_tile_compare_and_map(mmap,tp,tp2,mp1->tile_width,mp1->tile_height)) break;
				}
			}
		}
	}

// merge em so we dont point to something that points to something that points to etc etc etc

	for( mp1=mmap ; mp1<mmap+numof_mmaps ; mp1++ )
	{
		mp1_full_size=(mp1->width_in_tiles*mp1->height_in_tiles);

		for(tp=mp1->map;tp<mp1->map+mp1_full_size;tp++)
		{
			if(tp->master)
			for( tp2=tp->master ; tp2->master ; tp2=tp2->master )
			{
				tp->flip_x^=tp2->flip_x;		// do multi flipy flips
				tp->flip_y^=tp2->flip_y;
				tp->mapID=tp2->mapID;
				tp->master=tp2->master;
				tp->x=tp2->x;
				tp->y=tp2->y;
			}
		}
	}



	return ret;
}






/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill in stats info
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void mmap_buildstats( metamap *mmap , s32 numof_mmaps)
{

s32 tx,ty;
metamap_tile *tp;

metamap *mp;

s32 lay;

	mmap->stats_tiles_master=0;
	mmap->stats_tiles_slave=0;
	mmap->stats_tiles_slave_noflip=0;
	mmap->stats_tiles_slave_xflip=0;
	mmap->stats_tiles_slave_yflip=0;
	mmap->stats_tiles_slave_xyflip=0;

	mmap->stats_8tiles_master=0;
	mmap->stats_8tiles_slave=0;
	mmap->stats_8tiles_slave_noflip=0;
	mmap->stats_8tiles_slave_xflip=0;
	mmap->stats_8tiles_slave_yflip=0;
	mmap->stats_8tiles_slave_xyflip=0;


	for( mp=mmap ; mp<mmap+numof_mmaps ; mp++ )
	{
		for(ty=0;ty<mp->height_in_tiles;ty++)
		{
			for( tx=0 , tp=mp->map+(ty*mp->width_in_tiles) ; tx<mp->width_in_tiles ; tx++ , tp++)
			{
				if(tp->master)
				{
					mmap->stats_tiles_slave++;

					if(tp->flip_x&&tp->flip_y)
					{
						mmap->stats_tiles_slave_xyflip++;
					}
					else
					if(tp->flip_x)
					{
						mmap->stats_tiles_slave_xflip++;
					}
					else
					if(tp->flip_y)
					{
						mmap->stats_tiles_slave_yflip++;
					}
					else
					{
						mmap->stats_tiles_slave_noflip++;
					}
				}
				else
				{
					mmap->stats_tiles_master++;
				}

			}
		}

		for(lay=0;lay<mp->num_layers;lay++)
		{
			for(ty=0;ty<mp->height_in_8tiles;ty++)
			{
				for( tx=0 , tp=mp->map8+(lay*mp->width_in_8tiles*mp->height_in_8tiles)+(ty*mp->width_in_8tiles) ; tx<mp->width_in_8tiles ; tx++ , tp++)
				{
					if(tp->master)
					{
						mmap->stats_8tiles_slave++;

						if(tp->flip_x&&tp->flip_y)
						{
							mmap->stats_8tiles_slave_xyflip++;
						}
						else
						if(tp->flip_x)
						{
							mmap->stats_8tiles_slave_xflip++;
						}
						else
						if(tp->flip_y)
						{
							mmap->stats_8tiles_slave_yflip++;
						}
						else
						{
							mmap->stats_8tiles_slave_noflip++;
						}
					}
					else
					{
						mmap->stats_8tiles_master++;
					}

				}
			}
		}
	}

}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int DO_MMap(void)
{
int ret;
char *filename_in;
char *filename_out;

metamap	mmap[1];

s32 i,l;

	ret=20;


	mmap_setup(mmap);


	filename_in=CMD_Word(0);
	filename_out=CMD_Word(1);

	if(filename_in==0)
	{
		printf("\n**** ERROR no input filename specified, please try again ****\n\n");
		goto bogus;
	}


	for( l=0 ; l<4 ; l++ )
//	l=3;
	{
		mmap->ID=0;


		for(i=0;i<4;i++)
		{
			mmap->layers[i].fname->set_path(filename_in);
			mmap->layers[i].fname->fname[strlen(mmap->layers[i].fname->fname)-1]='0'+i;
			mmap->layers[i].fname->build();
		}
		mmap->layers[0].fname->set_path(mmap->layers[l].fname->path);
		mmap->num_layers=1;

//		mmap->num_layers=4;

		mmap->index_mask=0x0f;
		mmap->tile_width=16;
		mmap->tile_height=16;

		mmap_load(mmap);

		mmap_buildstats(mmap,1);

		printf("\n");
		printf("\n");
		printf("LayerTest=%d\n",mmap->num_layers);
		printf("StartChars=%d\n",mmap->stats_8tiles_master);
		printf("StartTiles=%d\n",mmap->stats_tiles_master);

		mmap_map8(mmap);

		mmap_map(mmap);

		mmap_buildstats(mmap,1);

		printf("\n");
		printf("UniqueChars=%d\n",mmap->stats_8tiles_master);
		printf("ReusedChars=%d\n",mmap->stats_8tiles_slave);
		printf("\tNOFlipChars=%d\n",mmap->stats_8tiles_slave_noflip);
		printf("\tXFlipChars=%d\n",mmap->stats_8tiles_slave_xflip);
		printf("\tYFlipChars=%d\n",mmap->stats_8tiles_slave_yflip);
		printf("\tXYFlipChars=%d\n",mmap->stats_8tiles_slave_xyflip);

		printf("\n");
		printf("UniqueTiles=%d\n",mmap->stats_tiles_master);
		printf("ReusedTiles=%d\n",mmap->stats_tiles_slave);
		printf("\tNOFlipTiles=%d\n",mmap->stats_tiles_slave_noflip);
		printf("\tXFlipTiles=%d\n",mmap->stats_tiles_slave_xflip);
		printf("\tYFlipTiles=%d\n",mmap->stats_tiles_slave_yflip);
		printf("\tXYFlipTiles=%d\n",mmap->stats_tiles_slave_xyflip);

		printf("\n");

		mmap_clean(mmap);

	}

	ret=0;
bogus:

	mmap_clean(mmap);

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int DO_RaceMap(void)
{
int ret;

path_bits	filename_in[1];
path_bits	filename_out[1];

s32 l;


TiXmlDocument doc[1];

TiXmlHandle hand(doc);

TiXmlElement	*layers;
TiXmlElement	*segments;

s32 num;
s32	numof_layers;
s32	numof_segments;

TiXmlElement	*item;

const char *postfixes[MAX_METAMAP_LAYERS];
s32 charsets[MAX_METAMAP_LAYERS];
s32 ignorecharlys[MAX_METAMAP_LAYERS];

metamap	*mmap;
metamap	*mp;

s32		index_mask;
s32		tile_width;
s32		tile_height;

FILE *fpc;
FILE *fph;


datachunk * chars[MAX_METAMAP_LAYERS]={0};

datachunk * tiles=0;

s32 i;

s32 mi,mc;

s32 total_output_maps;

	mmap=0;
	numof_segments=0;

	doc->SetCondenseWhiteSpace(true);


	ret=20;

	fpc=0;
	fph=0;


	filename_in->set_path( CMD_Word(0) );
	filename_out->set_path( CMD_Word(1) );

	if(filename_in->path[0]==0)
	{
		printf("\n**** ERROR no input filename specified, please try again ****\n\n");
		goto bogus;
	}


	if(filename_out->path[0]==0)
	{
		filename_out->set_path( filename_in->path );
	}



	filename_in->set_ext("xml");
	filename_in->build();

	if( ! doc->LoadFile(filename_in->path) )	{	goto bogus;	}


	hand=doc;


// Scan options info

	if(! ( layers = hand.FirstChildElement("BiRace").FirstChildElement("Options").Element() ) )
	{
		printf("\n**** ERROR failed to find options data ****\n\n");
		goto bogus;
	}

	{
	const char *tilewidth;
	const char *tileheight;
	const char *indexmask;

		printf("***OPTIONS***\n");

		tilewidth=layers->Attribute("tilewidth");tilewidth=tilewidth?tilewidth:"16";
		tileheight=layers->Attribute("tileheight");tileheight=tileheight?tileheight:"16";
		indexmask=layers->Attribute("indexmask");indexmask=indexmask?indexmask:"0x0f";

		printf("tilewidth=%s\n",tilewidth);
		printf("tileheight=%s\n",tileheight);
		printf("indexmask=%s\n",indexmask);
		printf("\n");

		tile_width=atoi(tilewidth);
		tile_height=atoi(tileheight);
		index_mask=atoi(indexmask);
	}


// Scan layer info

	if(! ( layers = hand.FirstChildElement("BiRace").FirstChildElement("Layers").Element() ) )
	{
		printf("\n**** ERROR failed to find layer data ****\n\n");
		goto bogus;
	}

	numof_layers=0;
	for( item=layers->FirstChildElement("Layer") ; item ; item=item->NextSiblingElement("Layer") )
	{
		numof_layers++;
	}

	if(numof_layers>MAX_METAMAP_LAYERS)
	{
		numof_layers=MAX_METAMAP_LAYERS;
	}

	printf("***LAYERS***\n");
	for( item=layers->FirstChildElement("Layer") , num=0 ; item ; item=item->NextSiblingElement("Layer") , num++)
	{
	const char *postfix;
	const char *charset;
	const char *ignorechars;

		printf("***LAYER%d***\n",num);

		postfix=item->Attribute("postfix");postfix=postfix?postfix:"";
		charset=item->Attribute("charset");charset=charset?charset:"0";
		ignorechars=item->Attribute("ignorechars");ignorechars=ignorechars?ignorechars:"0";

		printf("postfix=%s\n",postfix);
		printf("charset=%s\n",charset);
		printf("ignorechars=%s\n",ignorechars);
		printf("\n");

		postfixes[num]=postfix;
		charsets[num]=atoi(charset);
		ignorecharlys[num]=atoi(ignorechars);
	}
	printf("\n");

// Scan segment info

	printf("***SEGMENTS***\n");
	if(! ( segments = hand.FirstChildElement("BiRace").FirstChildElement("Segments").Element() ) )
	{
		printf("\n**** ERROR failed to find Segment data ****\n\n");
		goto bogus;
	}

	numof_segments=0;
	for( item=segments->FirstChildElement("Segment") ; item ; item=item->NextSiblingElement("Segment") )
	{
		numof_segments++;
	}

	if(!(mmap=(metamap*)calloc(sizeof(metamap),numof_segments)))
	{
		printf("\n**** bad mmap calloc ****\n\n");
		goto bogus;
	}

	for( item=segments->FirstChildElement("Segment") , num=0 ; item ; item=item->NextSiblingElement("Segment") , num++ )
	{
	const char *type;
	const char *fname;
	const char *ignoretiles;

		printf("***SEGMENT%d***\n",num);

		mmap_setup(mmap+num);


		type =item->Attribute("type");	type=type?type:"XXXX";
		fname=item->Attribute("fname");	fname=fname?fname:"";
		ignoretiles=item->Attribute("ignoretiles");	ignoretiles=ignoretiles?ignoretiles:"0";

		printf("type =%s\n",type);
		printf("fname=%s\n",fname);
		printf("ignoretiles=%s\n",ignoretiles);
		printf("\n");

		mp=mmap+num;


		if(stricmp(type,"XXXX")==0) mp->maptype=0x0; else
		if(stricmp(type,"XXXI")==0) mp->maptype=0x1; else
		if(stricmp(type,"XXIX")==0) mp->maptype=0x2; else
		if(stricmp(type,"XXII")==0) mp->maptype=0x3; else
		if(stricmp(type,"XIXX")==0) mp->maptype=0x4; else
		if(stricmp(type,"XIXI")==0) mp->maptype=0x5; else
		if(stricmp(type,"XIIX")==0) mp->maptype=0x6; else
		if(stricmp(type,"XIII")==0) mp->maptype=0x7; else
		if(stricmp(type,"IXXX")==0) mp->maptype=0x8; else
		if(stricmp(type,"IXXI")==0) mp->maptype=0x9; else
		if(stricmp(type,"IXIX")==0) mp->maptype=0xa; else
		if(stricmp(type,"IXII")==0) mp->maptype=0xb; else
		if(stricmp(type,"IIXX")==0) mp->maptype=0xc; else
		if(stricmp(type,"IIXI")==0) mp->maptype=0xd; else
		if(stricmp(type,"IIIX")==0) mp->maptype=0xe; else
		if(stricmp(type,"IIII")==0) mp->maptype=0xf; else mp->maptype=-1;

		mp->fname->set_path(filename_in->path);
		sprintf(mp->fname->fname,"%s",fname);
		mp->fname->build();

		for(l=0;l<numof_layers;l++)
		{
			mp->layers[l].fname->set_path(filename_in->path);
			sprintf(mp->layers[l].fname->fname,"%s%s",fname,postfixes[l]);
			mp->layers[l].fname->set_ext("bmp");
			mp->layers[l].fname->build();
			mp->layers[l].charsetID=charsets[l];
			mp->layers[l].ignore_chars=ignorecharlys[l]?true:false;

		}
		mp->num_layers=numof_layers;

		mp->index_mask=0x0f;
		mp->tile_width=tile_width;
		mp->tile_height=tile_height;
		mp->ignore_tiles=atoi(ignoretiles)?true:false;

		if(!mmap_load(mp))
		{
			goto bogus;
		}

		mmap_killhidden(mp,0,1);

		mmap_buildstats(mp,1);

		printf("\n");
		printf("bmps=%s\n",mp->layers[0].fname->path);
		printf("StartChars=%d\n",mp->stats_8tiles_master);
		printf("StartTiles=%d\n",mp->stats_tiles_master);

		mmap_map8(mp);

		mmap_map(mp);

		mmap_buildstats(mp,1);

		printf("...\n");
		printf("UniqueChars=%d\n",mp->stats_8tiles_master);
		printf("ReusedChars=%d\n",mp->stats_8tiles_slave);
		printf("\tNOFlipChars=%d\n",mp->stats_8tiles_slave_noflip);
		printf("\tXFlipChars=%d\n",mp->stats_8tiles_slave_xflip);
		printf("\tYFlipChars=%d\n",mp->stats_8tiles_slave_yflip);
		printf("\tXYFlipChars=%d\n",mp->stats_8tiles_slave_xyflip);
		printf("...\n");
		printf("UniqueTiles=%d\n",mp->stats_tiles_master);
		printf("ReusedTiles=%d\n",mp->stats_tiles_slave);
//		printf("\tNOFlipTiles=%d\n",mp->stats_tiles_slave_noflip);
//		printf("\tXFlipTiles=%d\n",mp->stats_tiles_slave_xflip);
//		printf("\tYFlipTiles=%d\n",mp->stats_tiles_slave_yflip);
//		printf("\tXYFlipTiles=%d\n",mp->stats_tiles_slave_xyflip);

		printf("\n");

	}
	printf("\n");

// merge all the maps together
	mmap_merge(mmap,numof_segments);

	mmap_buildstats(mmap,numof_segments);

	printf("***MERGED***\n");
	printf("\n");
	printf("UniqueChars=%d\n",mmap->stats_8tiles_master);
	printf("ReusedChars=%d\n",mmap->stats_8tiles_slave);
	printf("\tNOFlipChars=%d\n",mmap->stats_8tiles_slave_noflip);
	printf("\tXFlipChars=%d\n",mmap->stats_8tiles_slave_xflip);
	printf("\tYFlipChars=%d\n",mmap->stats_8tiles_slave_yflip);
	printf("\tXYFlipChars=%d\n",mmap->stats_8tiles_slave_xyflip);
	printf("...\n");
	printf("UniqueTiles=%d\n",mmap->stats_tiles_master);
	printf("ReusedTiles=%d\n",mmap->stats_tiles_slave);


// dump out chars

	sprintf(filename_out->fname+strlen(filename_out->fname),"_race");

	filename_out->set_ext("c");
	filename_out->build();
	if( ! (fpc=fopen(filename_out->path,"w")) )
	{
		goto bogus;
	}

	filename_out->set_ext("h");
	filename_out->build();
	if( ! (fph=fopen(filename_out->path,"w")) )
	{
		goto bogus;
	}

// build
	chars[0]=mmap_buildcharset( mmap, numof_segments , 0 );
	chars[1]=mmap_buildcharset( mmap, numof_segments , 1 );
	chars[2]=mmap_buildcharset( mmap, numof_segments , 2 );

	tiles=mmap_buildtileset( mmap, numof_segments );

	for(num=0;num<numof_segments;num++)
	{
		mmap[num].indexmap=mmap_buildtilemap(mmap+num);
	}

// compress, will be easy to add later honest ;)

// dump

	fprintf(fpc,"const unsigned char %s_chars0",filename_in->fname);
	chars[0]=datachunk_pucrunch(chars[0]);
	datachunk_dump(chars[0],fpc,"");

	fprintf(fpc,"const unsigned char %s_chars1",filename_in->fname);
	chars[1]=datachunk_pucrunch(chars[1]);
	datachunk_dump(chars[1],fpc,"");

	fprintf(fpc,"const unsigned char %s_tiles",filename_in->fname);
	tiles=datachunk_pucrunch(tiles);
	datachunk_dump(tiles,fpc,""/*"__attribute__ ((aligned (2)))"*/);

	for(num=0;num<numof_segments;num++)
	{
		if(mmap[num].maptype!=-1)
		{
			fprintf(fpc,"const unsigned char %s_%s_map",filename_in->fname,mmap[num].fname->fname);
			mmap[num].indexmap=datachunk_pucrunch(mmap[num].indexmap);
			datachunk_dump(mmap[num].indexmap,fpc,"");
		}
	}

// output structures describing this


// list of maps
	fprintf(fpc,"const struct race_map %s_maps[]=\n",filename_in->fname);
	fprintf(fpc,"{\n");
	mi=0;
	for(i=0;i<18;i++)
	for(num=0;num<numof_segments;num++)
	{
		if(mmap[num].maptype==i)
		{
			fprintf(fpc,"\t{\t%d,%d,%s_%s_map\t},\t\t//width,height in tiles,tilemap to use\n",mmap[num].width_in_tiles,mmap[num].height_in_tiles,filename_in->fname,mmap[num].fname->fname);
			mmap[num].mapindex=mi++;
		}
	}
	fprintf(fpc,"};\n\n");

	total_output_maps=mi;
	
	fprintf(fpc,"const unsigned char %s_maps_counts[18][2]=\n",filename_in->fname);
	fprintf(fpc,"{\n");
	mi=0;mc=0;
	for(i=0;i<18;i++)
	{
		for(num=0;num<numof_segments;num++)
		{
			if(mmap[num].maptype==i)
			{
				mc++;
			}
		}
		fprintf(fpc,"\t{%d,%d},\n",mi,mc);
		mi+=mc;
		mc=0;
	}
	fprintf(fpc,"};\n\n");

	fprintf(fpc,"const struct race_info %s_info=\n",filename_in->fname);
	fprintf(fpc,"{\n");
	fprintf(fpc,"\t%s_chars0,%d,\t\t\t\t//charset0\n",filename_in->fname,chars[0]->totalsize);
	fprintf(fpc,"\t%s_chars1,%d,\t\t\t\t//charset1\n",filename_in->fname,chars[1]->totalsize);
	fprintf(fpc,"\t%s_tiles,%d,\t\t\t\t//tileset\n",filename_in->fname,tiles->totalsize);
	fprintf(fpc,"\t%s_maps,%d,\t\t\t\t//list of maps\n",filename_in->fname,total_output_maps);
	fprintf(fpc,"\t%s_maps_counts\t\t//list indexs into maps for each piece type\n",filename_in->fname);
	fprintf(fpc,"};\n\n");

	fprintf(fph,"extern const struct race_info %s_info;\n",filename_in->fname);

	ret=0;
bogus:

	if(doc->Error())
	{
		printf("%s\n",doc->ErrorDesc());
	}

	if(mmap)
	{
		for(num=0;num<numof_segments;num++)
		{
			mmap_clean(mmap+num);
		}
		free(mmap);
	}

	for(i=0;i<MAX_METAMAP_LAYERS;i++)
	{
		if(chars[i])
		{
			free(chars[i]);
			chars[i]=0;
		}
	}

	if(tiles)
	{
		free(tiles);
		tiles=0;
	}

	if(fpc)
	{
		fclose(fpc);
		fpc=0;
	}

	if(fph)
	{
		fclose(fph);
		fph=0;
	}


	return ret;
}


#endif


