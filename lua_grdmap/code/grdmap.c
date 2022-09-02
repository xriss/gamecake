/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a gridmap structure
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grdmap * grdmap_alloc()
{
	return 	(struct grdmap *)calloc(sizeof(struct grdmap),1);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free any data in a grdmap and then free the structure
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grdmap_free(struct grdmap *gm)
{
	if(gm)
	{
		grdmap_clean(gm);
		free(gm);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Allocate chars 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_setup(struct grdmap *gm,struct grd *g)
{
s32 x;
s32 y;

	if(!gm) { goto bogus; } // need these parts
	
	grdmap_clean(gm); // remove old stuff first

	gm->g=g;

	return 1;
bogus:
	if(gm) { grdmap_clean(gm); }
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free any allocated data but not the structure
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_clean(struct grdmap *gm)
{
	if(gm)
	{
		if(gm->tiles)
		{
			free(gm->tiles);
		}
		memset(gm,0,sizeof(struct grdmap));	// clear data
	}

	return 1;
bogus:
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// break an image into chars of the given size, this may fail
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_cutup(struct grdmap *gm,s32 pw,s32 ph)
{
s32 x;
s32 y;
s32 id;
const char *err;

	if(!gm) { goto bogus; } // need these parts
	if(!gm->g) { err="missing grd"; goto bogus; } // need these parts
	
	gm->pw=pw;
	gm->ph=ph;
	
	gm->tw=(gm->g->bmap->w+pw-1)/pw; // round up
	gm->th=(gm->g->bmap->h+ph-1)/ph;
	
	gm->tiles=(struct grdmap_tile*)calloc(gm->tw*gm->th,sizeof(struct grdmap_tile));
	if(!gm->tiles) { goto bogus; } // error
	gm->numof_tiles=gm->tw*gm->th;
	
	id=0;
	for(y=0;y<gm->th;y++)
	{
		for(x=0;x<gm->tw;x++)
		{
				struct grdmap_tile *t=gm->tiles + (x+y*gm->tw);
				t->id=id;
				t->x=x*gm->pw;
				t->y=y*gm->ph;
				t->w=gm->pw;
				t->h=gm->ph;
				if( (t->x+t->w) > gm->g->bmap->w ) { t->w=gm->g->bmap->w-t->x; } // clip size of end tiles
				if( (t->y+t->h) > gm->g->bmap->h ) { t->h=gm->g->bmap->h-t->y; }
				t->base=gm;
				
				id++;
		}
	}

	return 1;
bogus:
	if(gm) { grdmap_clean(gm); gm->err=err; }
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce master to master chains down to a single pointer to the final data.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_unchain( struct grdmap *gm )
{
struct grdmap_tile *tp;

	if( ! gm->tiles ) { goto bogus; }

	for( tp=gm->tiles ; tp<gm->tiles+gm->numof_tiles ; tp++ )
	{
		if(tp->master)
		{
			while(tp->master->master)
			{
				tp->master=tp->master->master;
			}
		}
	}

	return 1;
bogus:
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles with all other tiles, and mark duplicates as such.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_merge( struct grdmap *gm )
{
struct grdmap_tile *pa;
struct grdmap_tile *pb;

	if( ! gm->tiles ) { goto bogus; }

	for( pa=gm->tiles ; pa<gm->tiles+gm->numof_tiles ; pa++ )
	{
		if(pa->master) continue;

		for( pb=gm->tiles ; pb<pa ; pb++ )
		{
			if(pb->master) continue;
			if(grdmap_tile_compare(pa,pb)) { pa->master=pb; break; }
		}
	}

	return 1;
bogus:
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Shrink each tile, remove alpha from around the outside.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_shrink( struct grdmap *gm )
{
struct grdmap_tile *pa;

	if( ! gm->tiles ) { goto bogus; }

	for( pa=gm->tiles ; pa<gm->tiles+gm->numof_tiles ; pa++ )
	{
		if(pa->master) continue;

		grdmap_tile_shrink(pa);
	}

	return 1;
bogus:
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare all tiles on one grdmap with all tiles on another grdmap
// and map them acordingly
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_keymap( struct grdmap *a , struct grdmap *b )
{
struct grdmap_tile *pa;
struct grdmap_tile *pb;

	if( ! a->tiles ) { goto bogus; }
	if( ! b->tiles ) { goto bogus; }

	for( pa=a->tiles ; pa<a->tiles+a->numof_tiles ; pa++ )
	{
		for( pb=b->tiles ; pb<b->tiles+b->numof_tiles ; pb++ )
		{
			if(pb->master) continue;
			if(grdmap_tile_compare(pa,pb)) { pa->master=pb; break; }
		}
	}

	return 1;
bogus:
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// layout the contents of one grdmap onto another, skip tiles that have a zero size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_layout( struct grdmap *a , struct grdmap *b , s32 border)
{
#if 0
bool ret;
struct grdmap_tile *pa;
struct grdmap_tile *pb;

s32 count;

// used to layout the tiles
s32 max_width;
s32 max_height;

s32 biggest_y;

s32 x,y;

s32 height_loop;

	ret=false;

	grdmap_update_cache(a);
	grdmap_update_cache(b);
	if(!grdmap_check(a)) goto bogus;
//	if(!grdmap_check(b)) goto bogus; //no need to check b, we will be creating it


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

	grdmap_clean(b);

	b->tiles=(grdmap_tile*)calloc(count,sizeof(struct grdmap_tile));
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


	ret=1;
bogus:
	return ret;
#endif
	return 0;
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save the grdmap info as an XTX data file
//
// pass in the layedout page in grdmap_layout and the original page in grdmap_master in order to generate a mapping table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int grdmap_save_XTX( struct grdmap *grdmap_layout , struct grdmap *grdmap_master ,const char *filename)
{
#if 0

XTX0_header		xtxhead[1];
XTX0_info		xtxinfo[1];
XTX0_area		xtxarea[1];

FILE *fp=0;

int ret;

struct grdmap_tile *tile;
struct grdmap_tile *tile2;


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

	xtxinfo->numof_areas=grdmap_layout->numof_tiles;
	xtxinfo->numof_maps=grdmap_master->numof_tiles;
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
	for( tile=grdmap_layout->tiles ; tile<grdmap_layout->tiles+grdmap_layout->numof_tiles ; tile++ )
	{
		xtxarea->x=((s16)(tile->x));///((f32)(grdmap_layout->w));
		xtxarea->y=((s16)(tile->y));///((f32)(grdmap_layout->h));

		xtxarea->w=((s16)(tile->w));///((f32)(grdmap_layout->w));
		xtxarea->h=((s16)(tile->h));///((f32)(grdmap_layout->h));

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

	for( tile=grdmap_master->tiles ; tile<grdmap_master->tiles+grdmap_master->numof_tiles ; tile++ )
	{
		id=0; // not a tile

		for( tile2=grdmap_layout->tiles ; tile2<grdmap_layout->tiles+grdmap_layout->numof_tiles ; tile2++ )
		{
			if(tile2->master==tile)
			{
				id=1+tile2-grdmap_layout->tiles;
				break;
			}
		}

		if(1!=fwrite((void*)&id,sizeof(id),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
	}


	ret=1;
bogus:
	if(fp) { fclose(fp); }
	return ret;

#endif
	return 1;
}
