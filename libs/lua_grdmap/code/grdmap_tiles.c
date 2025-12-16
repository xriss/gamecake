/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"






/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shrink the given tile, adjust the posiion and size such that only the non transparent potion is within the area
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grdmap_tile_shrink(struct grdmap_tile *a )
{

s32 sa,sp; // spans for tile a and a pixel

u8 *pa; // data pointers
s32 x,y,p;	// x,y pointers

u32 pix;

int clear;

s32 delta;

	sa=a->base->g->bmap->yscan;
	sp=a->base->g->bmap->xscan;


// push up from bottom
	delta=0;
	for(y=a->h-1;y>=0;y--)
	{
		pa = grdinfo_get_data(a->base->g->bmap,a->x,a->y+y,0);

		clear=1;
		for(x=0;x<a->w;x++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=0; break; }
		}
		if(clear) delta++; else break;
	}
	a->h-=delta;

// push down from top
	delta=0;
	for(y=0;y<a->h;y++)
	{
		pa = grdinfo_get_data(a->base->g->bmap,a->x,a->y+y,0);

		clear=1;
		for(x=0;x<a->w;x++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=0; break; }
		}
		if(clear) delta++; else break;
	}
	a->h-=delta;
	a->y+=delta;
	a->hy+=delta;

// check we have something left

	if(a->h==0) // all gone, nothing left to do
	{
		a->w=0;
		return;
	}

// push left from right
	delta=0;
	for(x=a->w-1;x>=0;x--)
	{
		pa = grdinfo_get_data(a->base->g->bmap,a->x+x,a->y,0);

		clear=1;
		for(y=0;y<a->h;y++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=0; break; }
			pa+=sa-sp;
		}
		if(clear) delta++; else break;
	}
	a->w-=delta;

// push right from left
	delta=0;
	for(x=0;x<a->w;x++)
	{
		pa = grdinfo_get_data(a->base->g->bmap,a->x+x,a->y,0);

		clear=1;
		for(y=0;y<a->h;y++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=0; break; }
			pa+=sa-sp;
		}
		if(clear) delta++; else break;
	}
	a->w-=delta;
	a->x+=delta;
	a->hx+=delta;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare two tiles 
//
// return true if they are exactly the same
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grdmap_tile_compare(struct grdmap_tile *a , struct grdmap_tile *b)
{

s32 w,h; //in bytes for both tiles (if tiles are diferent size then they aint equal)
s32 sp; // spans for tiles a and b and a pixel

u8 *pa,*pb; // data pointers
s32 x,y;	// x,y pointers


	if(a==b) return 1; // pass in the same pointer twice?

	if(a->w!=b->w) return 0;
	if(a->h!=b->h) return 0;

	if(a->base!=b->base) // only need to check if not pointing to same image
	{
		if(a->base->g->bmap->fmt!=b->base->g->bmap->fmt) return 0;
	}

	sp=a->base->g->bmap->xscan;

	w=a->w*sp;
	h=a->h;

	for(y=0;y<h;y++)
	{
		pa = grdinfo_get_data(a->base->g->bmap,a->x,a->y+y,0);
		pb = grdinfo_get_data(b->base->g->bmap,b->x,b->y+y,0);

		for(x=0;x<w;x++)
		{
			if(*pa++!=*pb++) return 0;
		}
	}

	return 1; // if we got here then tiles are the same
}
