/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
heightmap * hmap_alloc( void )
{
heightmap *hmap;

	if(!(hmap=(heightmap*)calloc(sizeof(heightmap),1))) goto bogus;

	hmap_setup(hmap);
	
bogus:
	return hmap;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void hmap_free( heightmap *hmap )
{
	if(hmap)
	{
		hmap_clean(hmap);
	}

	return;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// setup the basic structures so they are ready to be filled in
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool hmap_setup( heightmap *hmap )
{
bool ret;

	ret=true;

	memset(hmap,0,sizeof(heightmap));	// clear data

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean/free anything that may have been allocated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool hmap_clean( heightmap *hmap )
{
bool ret;

	ret=true;

	if(hmap->tiles)
	{
		free(hmap->tiles);
		hmap->tiles=0;
		hmap->numof_tiles=0;
		hmap->th=0;
		hmap->tw=0;
	}

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate the actual map memory
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool hmap_build( heightmap *hmap , s32 width , s32 height )
{
bool ret;
hmap_tile *t;

	ret=false;

	if( ! ( hmap->tiles = (hmap_tile*) calloc( width*height , sizeof(hmap_tile) ) ) ) goto bogus;

	hmap->numof_tiles=width*height;
	hmap->th=height;
	hmap->tw=width;

	for( t=hmap->tiles ; t<hmap->tiles+hmap->numof_tiles ; t++ )
	{
		t->height=0.0f;
		t->x=0.0f;
		t->y=0.0f;
	}

	ret=true;
bogus:
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create a heightmap using the given metamap as source
//
// this meta map is expected to have been mapped onto a key with the given properties
//
// 8 cells wide, each representing one of 8 downward directions
// as high as you wish, each cell down represenst a steeper slope, controled by input steepness to this function
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool hmap_build_from_mmap( heightmap *hmap , metamap *mmap , f32 steepness )
{
bool ret;

s32 i;
s32 cx,cy;
mmap_tile *m;
hmap_tile *h;
s32 x,y;
f32 t;

	ret=false;

	if( ! ( hmap_build(hmap,mmap->tw+1,mmap->th*2+1) ) ) goto bogus;


	for(cy=0;cy<mmap->th;cy++)
	{
		for(cx=0;cx<mmap->tw;cx++)
		{
			h=hmap->tiles + cy*hmap->tw*2 + hmap->tw + cx;	// center point for each input tile
			m=mmap->tiles + cy*mmap->tw + cx;

			h->x=0.0f;
			h->y=0.0f;

			if( m->master->base != m->base ) // we are mapped to the key
			{
				x=m->master->x / m->master->w ;	// turn into cell coord
				y=m->master->y / m->master->h ;

				switch(x) // build downward vector of the required amplitude
				{
					case 0: h->x= 0.0f; h->y= 1.0f; break;
					case 1: h->x=-1.0f; h->y= 1.0f; break;
					case 2: h->x=-1.0f; h->y= 0.0f; break;
					case 3: h->x=-1.0f; h->y=-1.0f; break;
					case 4: h->x= 0.0f; h->y=-1.0f; break;
					case 5: h->x= 1.0f; h->y=-1.0f; break;
					case 6: h->x= 1.0f; h->y= 0.0f; break;
					case 7: h->x= 1.0f; h->y= 1.0f; break;

					default: h->x=0.0f; h->y=0.0f; break;
				}

				h->x*=steepness*y;
				h->y*=steepness*y;
			}
		}
	}

// now we need to go through the entier thing a few times and push corner heights such that they end up at the desired slope

	for(i=0;i<32;i++) // number of iterations
	{
		for(cy=( i&1 ? 0 : mmap->th-1 );( i&1 ? cy<mmap->th : cy>=0 );cy+=( i&1 ? 1 : -1) )
		{
			for(cx=( i&1 ? 0 : mmap->tw-1 );( i&1 ? cx<mmap->tw : cx>=0 );cx+=( i&1 ? 1 : -1) )
			{
				h=hmap->tiles + cy*hmap->tw*2 + hmap->tw + cx;	// center point for each input tile

				t=(h->y - ( h[hmap->tw].height - h[-hmap->tw].height ))*0.5f;
				h[-hmap->tw].height-=t;
				h[ hmap->tw].height+=t;

				t=(h->y - ( h[hmap->tw+1].height - h[-hmap->tw+1].height ))*0.5f;
				h[-hmap->tw+1].height-=t;
				h[ hmap->tw+1].height+=t;

				t=(h->x - ( h[-hmap->tw+1].height - h[-hmap->tw].height ))*0.5f;
				h[-hmap->tw].height-=t;
				h[-hmap->tw+1].height+=t;

				t=(h->x - ( h[hmap->tw+1].height - h[hmap->tw].height ))*0.5f;
				h[ hmap->tw].height-=t;
				h[ hmap->tw+1].height+=t;
			}
		}
	}

// go through agaon and set the center height to the average of the four corners

	for(cy=0;cy<mmap->th;cy++)
	{
		for(cx=0;cx<mmap->tw;cx++)
		{
			h=hmap->tiles + cy*hmap->tw*2 + hmap->tw + cx;	// center point for each input tile

			t=0;
			t+=h[hmap->tw].height;
			t+=h[hmap->tw+1].height;
			t+=h[-hmap->tw].height;
			t+=h[-hmap->tw+1].height;
			t*=0.25f;

			h->height=t;
		}
	}


	ret=true;
bogus:
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill the current DevIL image with the current map, any errors will probably be devil ones
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool hmap_draw(heightmap *hmap)
{
float min,max;

hmap_tile *t;

u8 *p,*pb;
s32 b;

s32 x,y;

	if(!hmap->tiles) return false;

	if( ilTexImage( (hmap->tw)*2 , hmap->th , 1 , 1 , IL_LUMINANCE, IL_UNSIGNED_BYTE, 0 ) != IL_TRUE ) return false;

	if( ! (pb=ilGetData() ) ) return false;

	min= (65546.0f*65536.0f);
	max=-(65546.0f*65536.0f);

	for( t=hmap->tiles ; t<hmap->tiles+hmap->numof_tiles ; t++ )
	{
		if( t->height > max) max=t->height;
		if( t->height < min) min=t->height;
	}

	for(y=0;y<hmap->th;y++)
	{
		for( t=hmap->tiles + y*hmap->tw , p=pb + y*(hmap->tw*2) , x=0 ; x<hmap->tw ; t++ , x++ )
		{
			b=(s32) ( ( 4.0f * (t->height - min) ) /* (max-min)*/ );
			if(b<0)   b=0;
			if(b>255) b=255;

			if(y&1) // odd
			{
				if(x<=(hmap->tw-2))
				{
					*p++=b;
					*p++=b;
				}
				if( (x==(hmap->tw-2)) || (x==0) )
				{
					*p++=b;
				}
			}
			else // even
			{
				*p++=b;
				*p++=b;
			}
		}
	}

	return true;
}
