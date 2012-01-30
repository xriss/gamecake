/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
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
#if 0
bool ret;

	ret=true;

	memset(mmap,0,sizeof(metamap));	// clear data

	return ret;
#endif
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean/free anything that may have been allocated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_clean( metamap *mmap )
{
#if 0
bool ret;

	ret=true;

	if(mmap->tiles)
	{
		free(mmap->tiles);
		mmap->tiles=0;
		mmap->numof_tiles=0;
	}

	return ret;
#endif
	return true;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// update the image info cache
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void mmap_update_cache( metamap *mmap )
{
#if 0
	ilBindImage(mmap->image);

	mmap->w=ilGetInteger(IL_IMAGE_WIDTH);
	mmap->h=ilGetInteger(IL_IMAGE_HEIGHT);

	mmap->format=ilGetInteger(IL_IMAGE_FORMAT);
	mmap->type=ilGetInteger(IL_IMAGE_TYPE);

	mmap->sizeof_pix=ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

	mmap->span=mmap->sizeof_pix*mmap->w;

	mmap->data=ilGetData();
#endif
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check we have tiles and image data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_check( metamap *mmap )
{
#if 0
	if(!mmap->tiles) return false;
	if(!mmap->data) return false;

	return true;
#endif
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
#if 0
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
#endif
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce master to master chains down to a single pointer to the final data.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_unchain( metamap *mmap )
{
#if 0
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
#endif
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles with all other tiles, and mark duplicates as such.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_merge( metamap *mmap )
{
#if 0
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
#endif
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Shrink each tile, remove alpha from around the outside.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_shrink( metamap *mmap )
{
#if 0
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
#endif
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles on one metamap with all tiles on another metamap
// and map them acordingly
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_keymap( metamap *a , metamap *b )
{
#if 0
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
#endif
	return true;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// layout the contents of one metamap onto another, skip tiles that have a zero size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool mmap_layout( metamap *a , metamap *b , s32 border)
{
#if 0
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
#endif
	return true;
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
#if 0

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

#endif
	return true;
}
