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
void metamap_char_shrink(mmap_tile *a )
{

s32 sa,sp; // spans for tile a and a pixel

u8 *pa; // data pointers
s32 x,y,p;	// x,y pointers

u32 pix;

bool clear;

s32 delta;

	sa=a->base->span;
	sp=a->base->sizeof_pix;


// push up from bottom
	delta=0;
	for(y=a->h-1;y>=0;y--)
	{
		pa = a->base->data + (a->x*sp) + ((a->y+y)*sa) ;

		clear=true;
		for(x=0;x<a->w;x++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=false; break; }
		}
		if(clear) delta++; else break;
	}
	a->h-=delta;

// push down from top
	delta=0;
	for(y=0;y<a->h;y++)
	{
		pa = a->base->data + (a->x*sp) + ((a->y+y)*sa) ;

		clear=true;
		for(x=0;x<a->w;x++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=false; break; }
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
		pa = a->base->data + ((a->x+x)*sp) + (a->y*sa) ;

		clear=true;
		for(y=0;y<a->h;y++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=false; break; }
			pa+=sa-sp;
		}
		if(clear) delta++; else break;
	}
	a->w-=delta;

// push right from left
	delta=0;
	for(x=0;x<a->w;x++)
	{
		pa = a->base->data + ((a->x+x)*sp) + (a->y*sa) ;

		clear=true;
		for(y=0;y<a->h;y++)
		{
			pix=0;
			for(p=0;p<sp;p++)
			{
				pix=(pix<<8)+*pa++;
			}
			if(pix!=0) { clear=false; break; }
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
bool metamap_char_compare(mmap_tile *a , mmap_tile *b)
{

s32 w,h; //in bytes for both tiles (if tiles are diferent size then they aint equil)
s32 sa,sb,sp; // spans for tiles a and b and a pixel

u8 *pa,*pb; // data pointers
s32 x,y;	// x,y pointers


	if(a==b) return true; // pass in the same pointer twice?

	if(a->w!=b->w) return false;
	if(a->h!=b->h) return false;

	if(a->base!=b->base) // only need to check if not pointing to same image
	{
		if(a->base->format!=b->base->format) return false;
		if(a->base->type!=b->base->type) return false;
		if(a->base->sizeof_pix!=b->base->sizeof_pix) return false;
	}

	sa=a->base->span;
	sb=b->base->span;
	sp=a->base->sizeof_pix;

	w=a->w*sp;
	h=a->h;

	for(y=0;y<h;y++)
	{
		pa = a->base->data + (a->x*sp) + ((a->y+y)*sa) ;
		pb = b->base->data + (b->x*sp) + ((b->y+y)*sb) ;

		for(x=0;x<w;x++)
		{
			if(*pa++!=*pb++) return false;
		}
	}

	return true; // if we got here then tiles are the same
}



#if 0

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// is a 16 color char
// fully solid 2
// partially solid	1
// or completly transparent 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void metamap_clearchar(metamap *mmap, metamap_tile *tp)
{

u32 dat;

metamap *mp;
metamap_layer *lay;
u8 *ip;
s32 tx,ty;

bool got_solid;
bool got_trans;

const s32 th=8;
const s32 tw=8;

	mp=mmap+tp->mapID;
	lay=mp->layers+tp->layaID;

	ip= lay->data + mp->width*tp->y*th + tp->x*tw;

	got_solid=false;
	got_trans=false;

	for(ty=0;ty<th;ty++)
	{
		dat=0;

		for(tx=0;tx<tw;tx++)
		{
			*(ip++)=0;
		}
		ip+=mp->width-tw;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// is a 16 color char
// fully solid 2
// partially solid	1
// or completly transparent 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 metamap_testchar_0f(metamap *mmap, metamap_tile *tp)
{

u32 dat;

metamap *mp;
metamap_layer *lay;
u8 *ip;
s32 tx,ty;

bool got_solid;
bool got_trans;

const s32 th=8;
const s32 tw=8;

	mp=mmap+tp->mapID;
	lay=mp->layers+tp->layaID;

	ip= lay->data + mp->width*tp->y*th + tp->x*tw;

	got_solid=false;
	got_trans=false;

	for(ty=0;ty<th;ty++)
	{
		dat=0;

		for(tx=0;tx<tw;tx++)
		{
			if((*(ip++))&0x0f)
			{
				got_solid=true;
			}
			else
			{
				got_trans=true;
			}
		}
		ip+=mp->width-tw;
	}

	if( got_solid && got_trans )
	{
		return 1;
	}

	if( got_solid && (!got_trans)  )
	{
		return 2;
	}

	if( got_trans && (!got_solid) )
	{
		return 0;
	}

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab a 16 color char
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 metamap_getchar_0f(metamap *mmap, metamap_tile *tp, u8 *data)
{
u32 dat;

metamap *mp;
metamap_layer *lay;
u8 *ip;
s32 tx,ty;

const s32 th=8;
const s32 tw=8;

	mp=mmap+tp->mapID;
	lay=mp->layers+tp->layaID;

	ip= lay->data + mp->width*tp->y*th + tp->x*tw;

	for(ty=0;ty<th;ty++)
	{
		dat=0;

		for(tx=0;tx<tw;tx++)
		{
			dat = (dat>>4) | (((*(ip++))&0x0f)<<28) ;
		}

		*data++=(u8)((dat>> 0)&0xff);
		*data++=(u8)((dat>> 8)&0xff);
		*data++=(u8)((dat>>16)&0xff);
		*data++=(u8)((dat>>24)&0xff);

		ip+=mp->width-tw;
	}


	return 32;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get the pallete of a 16 color tile
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 metamap_getchar_0f_pal(metamap *mmap, metamap_tile *tp )
{
u32 dat;

metamap *mp;
metamap_layer *lay;
u8 *ip;
s32 tx,ty;

const s32 th=8;
const s32 tw=8;

	mp=mmap+tp->mapID;
	lay=mp->layers+tp->layaID;

	ip= lay->data + mp->width*tp->y*th + tp->x*tw;

	for(ty=0;ty<th;ty++)
	{
		for(tx=0;tx<tw;tx++)
		{
			dat=*(ip++);

			if(dat&0x0f) // a solid color
			{
				return (dat>>4)&0x0f;
			}
		}
		ip+=mp->width-tw;
	}

	return (dat>>4)&0x0f;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab a metatile, oh so many layers.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 metamap_gettile(metamap *mmap, metamap_tile *tp, u8 *data)
{

metamap *mp;
s32 l;
s32 tx,ty;

s32 th;
s32 tw;

metamap_tile *t8p;

metamap_tile *tmp;

u8 *orig_data;

	orig_data=data;

	mp=mmap+tp->mapID;


	th=mp->tile_height/8;
	tw=mp->tile_width/8;


u8 dat;

	dat=0;

	for( l=0 ; l<mp->num_layers ; l++ )
	{
		for(ty=0;ty<th;ty++)
		{
			t8p= mp->map8 + ((tp->y*th)+ty)*mp->width_in_8tiles + (tp->x*tw);
			t8p+=mp->width_in_8tiles*mp->height_in_8tiles*l;

			for(tx=0;tx<tw;tx++,t8p++)
			{
				for( tmp=t8p ; tmp->master ; tmp=tmp->master ) ; // get master for this time

				if(mp->layers[l].ignore_chars) // just put out 1 byte of flags
				{
					dat>>=1;
					dat|=(tmp->index ? 1 : 0 ) <<7;

//					data[0]=((tmp->index)&0xff);
//					data+=1;
				}
				else
				{
					data[0]=((tmp->index)&0xff);
					data[1]=((tmp->index>>8)&0xff) | (t8p->flip_y?0x08:0) | (t8p->flip_x?0x04:0) | (t8p->pal<<4) ;
					data+=2;
				}
			}
		}
		if(mp->layers[l].ignore_chars) // just put out 1 byte flags, no flippy floppy
		{
			data[0]=dat;
			data+=1;
		}
	}


	return data-orig_data;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// bitcompare two tiles, say yes if they are the same
// both tiles should be master tiles, otherwise we ramp up till we hit the master tiles
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

s32 mmap_tile_compare8( metamap *mmap, metamap_tile *tpa , metamap_tile *tpb , s32 tw , s32 th )
{
metamap *mpa,*mpb;
//s32 lid;
metamap_layer *laya,*layb;
u8 *ipa,*ipb;
s32 tx,ty;


//	while(tpa->master) { tpa=tpa->master; }
//	while(tpb->master) { tpa=tpb->master; }

/*
	if(tpa==tpb) // compare pointers at this point
	{
		return 1;
	}
*/


// now we have to do a bit test on the data

	mpa=mmap+tpa->mapID;
	mpb=mmap+tpb->mapID;


// do non flipped test

//	for( laya=mpa->layers , layb=mpb->layers , lid=0 ; lid<mpa->num_layers ; laya++ , layb++ , lid++ )
	laya=mpa->layers+tpa->layaID;
	layb=mpb->layers+tpb->layaID;

	if(laya->charsetID!=layb->charsetID) // not in same charset?
	{
		return 0;
	}

	{
		ipa= laya->data + mpa->width*tpa->y*th + tpa->x*tw;
		ipb= layb->data + mpb->width*tpb->y*th + tpb->x*tw;

		for(ty=0;ty<th;ty++)
		{
			for(tx=0;tx<tw;tx++)
			{
				if( ((*(ipa++))&mpa->index_mask) != ((*(ipb++))&mpa->index_mask) )
				{
					goto not_noflip;
				}
			}
			ipa+=mpa->width-tw;
			ipb+=mpb->width-tw;
		}
	}
	return 1+0;

not_noflip:


// do x flipped test

//	for( laya=mpa->layers , layb=mpb->layers , lid=0 ; lid<mpa->num_layers ; laya++ , layb++ , lid++ )
	laya=mpa->layers+tpa->layaID;
	layb=mpb->layers+tpb->layaID;
	{
		ipa= laya->data + mpa->width*tpa->y*th + tpa->x*tw;
		ipb= layb->data + mpb->width*tpb->y*th + tpb->x*tw;

		for(ty=0;ty<th;ty++)
		{
			ipa+=tw;
			for(tx=0;tx<tw;tx++)
			{
				if( ((*(--ipa))&mpa->index_mask) != ((*(ipb++))&mpa->index_mask) )
				{
					goto not_xflip;
				}
			}
			ipa+=mpa->width;
			ipb+=mpb->width-tw;
		}
	}
	return 1+1;

not_xflip:

// do y flipped test

//	for( laya=mpa->layers , layb=mpb->layers , lid=0 ; lid<mpa->num_layers ; laya++ , layb++ , lid++ )
	laya=mpa->layers+tpa->layaID;
	layb=mpb->layers+tpb->layaID;
	{
		ipa= laya->data + mpa->width*tpa->y*th + tpa->x*tw;
		ipb= layb->data + mpb->width*tpb->y*th + tpb->x*tw;

		ipa+=(mpa->width*(th-1));

		for(ty=0;ty<th;ty++)
		{
			for(tx=0;tx<tw;tx++)
			{
				if( ((*(ipa++))&mpa->index_mask) != ((*(ipb++))&mpa->index_mask) )
				{
					goto not_yflip;
				}
			}
			ipa-=mpa->width+tw;
			ipb+=mpb->width-tw;
		}
	}
	return 1+2;

not_yflip:

// do xy flipped test

//	for( laya=mpa->layers , layb=mpb->layers , lid=0 ; lid<mpa->num_layers ; laya++ , layb++ , lid++ )
	laya=mpa->layers+tpa->layaID;
	layb=mpb->layers+tpb->layaID;
	{
		ipa= laya->data + mpa->width*tpa->y*th + tpa->x*tw;
		ipb= layb->data + mpb->width*tpb->y*th + tpb->x*tw;

		ipa+=(mpa->width*(th-1))+tw;

		for(ty=0;ty<th;ty++)
		{
			for(tx=0;tx<tw;tx++)
			{
				if( ((*(--ipa))&mpa->index_mask) != ((*(ipb++))&mpa->index_mask) )
				{
					goto not_xyflip;
				}
			}
			ipa-=mpa->width-tw;
			ipb+=mpb->width-tw;
		}
	}
	return 1+3;

not_xyflip:

	return 0;
}
s32 mmap_tile_compare_and_map8( metamap *mmap, metamap_tile *tpa , metamap_tile *tpb , s32 tw , s32 th )
{
s32 ret;

	ret=mmap_tile_compare8(mmap,tpa,tpb,tw,th);

	switch(ret)
	{
		case 0:	// not the same
		break;

		case 1:	// the same

			tpa->master=tpb;
			tpa->flip_x=false;
			tpa->flip_y=false;
			tpa->mapID=tpb->mapID;
			tpa->layaID=tpb->layaID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 2:	// xflip the same

			tpa->master=tpb;
			tpa->flip_x=true;
			tpa->flip_y=false;
			tpa->mapID=tpb->mapID;
			tpa->layaID=tpb->layaID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 3:	// yflip the same

			tpa->master=tpb;
			tpa->flip_x=false;
			tpa->flip_y=true;
			tpa->mapID=tpb->mapID;
			tpa->layaID=tpb->layaID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 4:	// xyflip the same

			tpa->master=tpb;
			tpa->flip_x=true;
			tpa->flip_y=true;
			tpa->mapID=tpb->mapID;
			tpa->layaID=tpb->layaID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;
	}

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// compare the sub 8x8 tiles of this metatile, say yes if they are the same
// both tiles should be master tiles, otherwise we ramp up till we hit the master tiles
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

s32 mmap_tile_compare( metamap *mmap, metamap_tile *tpa , metamap_tile *tpb , s32 tw , s32 th )
{
metamap *mpa,*mpb;
s32 lid;
s32 tx,ty;
metamap_tile *t8pa,*t8pb;



	mpa=mmap+tpa->mapID;
	mpb=mmap+tpb->mapID;


	for( lid=0 ; lid<mpa->num_layers ; lid++ )
	{
		for( ty=0 ; ty<(th/8) ; ty++ )
		{
			t8pa= mpa->map8 + ((tpa->y*th/8)+ty)*mpa->width_in_8tiles + (tpa->x*tw/8);
			t8pb= mpb->map8 + ((tpb->y*th/8)+ty)*mpb->width_in_8tiles + (tpb->x*tw/8);

			t8pa+=mpa->width_in_8tiles*mpa->height_in_8tiles*lid;
			t8pb+=mpb->width_in_8tiles*mpb->height_in_8tiles*lid;

			for( tx=0 ; tx<(tw/8) ; tx++ , t8pa++ , t8pb++ )
			{
				if( t8pa->x			!= t8pb->x )			{	goto not_noflip;	}
				if( t8pa->y			!= t8pb->y )			{	goto not_noflip;	}
				if( t8pa->pal		!= t8pb->pal )			{	goto not_noflip;	}
				if( t8pa->flip_x	!= t8pb->flip_x )		{	goto not_noflip;	}
				if( t8pa->flip_y	!= t8pb->flip_y )		{	goto not_noflip;	}
				if( t8pa->layaID	!= t8pb->layaID )		{	goto not_noflip;	}
				if( t8pa->mapID		!= t8pb->mapID )		{	goto not_noflip;	}
			}
		}
	}
	return 1+0;

not_noflip:


	return 0;
}
s32 mmap_tile_compare_and_map( metamap *mmap, metamap_tile *tpa , metamap_tile *tpb , s32 tw , s32 th )
{
s32 ret;

	ret=mmap_tile_compare(mmap,tpa,tpb,tw,th);

	switch(ret)
	{
		case 0:	// not the same
		break;

		case 1:	// the same

			tpa->master=tpb;
			tpa->flip_x=false;
			tpa->flip_y=false;
			tpa->mapID=tpb->mapID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 2:	// xflip the same

			tpa->master=tpb;
			tpa->flip_x=true;
			tpa->flip_y=false;
			tpa->mapID=tpb->mapID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 3:	// yflip the same

			tpa->master=tpb;
			tpa->flip_x=false;
			tpa->flip_y=true;
			tpa->mapID=tpb->mapID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;

		case 4:	// xyflip the same

			tpa->master=tpb;
			tpa->flip_x=true;
			tpa->flip_y=true;
			tpa->mapID=tpb->mapID;
			tpa->x=tpb->x;
			tpa->y=tpb->y;
		break;
	}

	return ret;
}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// build charset of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

datachunk * mmap_buildcharset( metamap *mmap, s32 numof_mmaps , s32 charsetID )
{
datachunk *ret;

s32 tx,ty;
metamap_tile *tp;
metamap *mp;
s32 lay;

s32 presize;
s32 numofchars;


u8* data;

//u32 dat;

//u8 *pp;

s32 siz;

s32 index;

// count number of chars we need to allocate

	numofchars=0;

	for( mp=mmap ; mp<mmap+numof_mmaps ; mp++ )
	{
		for(lay=0;lay<mp->num_layers;lay++)
		{
			for(ty=0;ty<mp->height_in_8tiles;ty++)
			{
				for( tx=0 , tp=mp->map8+(lay*mp->width_in_8tiles*mp->height_in_8tiles)+(ty*mp->width_in_8tiles) ; tx<mp->width_in_8tiles ; tx++ , tp++)
				{
					if(!tp->master)
					{
						if(mp->layers[lay].charsetID==charsetID) //we point at ourself so this is ok
						{
							numofchars++;
						}
					}

				}
			}
		}
	}

	presize=sizeof(datachunk) + (numofchars*32);

	ret=(datachunk*) calloc( presize , 1);

	index=0;

	if(ret)
	{
		ret->totalsize=presize-sizeof(datachunk);
		ret->size=0;

		data=ret->data;

		for( mp=mmap ; mp<mmap+numof_mmaps ; mp++ )
		{
			for(lay=0;lay<mp->num_layers;lay++)
			{
				for(ty=0;ty<mp->height_in_8tiles;ty++)
				{
					for( tx=0 , tp=mp->map8+(lay*mp->width_in_8tiles*mp->height_in_8tiles)+(ty*mp->width_in_8tiles) ; tx<mp->width_in_8tiles ; tx++ , tp++)
					{
						if(!tp->master)
						{
							if(mp->layers[lay].charsetID==charsetID) //we point at ourself so this is ok
							{
								siz=metamap_getchar_0f(mmap,tp,data);
								ret->size+=siz;
								data+=siz;

								tp->index=index++;
							}
						}
					}
				}
			}
		}
	}

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// build charset of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

datachunk * mmap_buildtileset( metamap *mmap, s32 numof_mmaps )
{
datachunk *ret;

s32 tx,ty;
metamap_tile *tp;
metamap *mp;

s32 presize;
s32 numofchars;


u8* data;

//u32 dat;

//u8 *pp;

s32 siz;

s32 index;


// count number of chars we need to allocate

	numofchars=0;

	for( mp=mmap ; mp<mmap+numof_mmaps ; mp++ )
	{
			for(ty=0;ty<mp->height_in_tiles;ty++)
			{
				for( tx=0 , tp=mp->map+(ty*mp->width_in_tiles) ; tx<mp->width_in_tiles ; tx++ , tp++)
				{
					if(!tp->master)
					{
						numofchars++;
					}
				}
			}
	}

	presize=sizeof(datachunk) + (numofchars*(8+8+1));

	ret=(datachunk*) calloc( presize , 1);

	index=0;

	if(ret)
	{
		ret->totalsize=presize-sizeof(datachunk);
		ret->size=0;

		data=ret->data;

		for( mp=mmap ; mp<mmap+numof_mmaps ; mp++ )
		{
				for(ty=0;ty<mp->height_in_tiles;ty++)
				{
					for( tx=0 , tp=mp->map+(ty*mp->width_in_tiles) ; tx<mp->width_in_tiles ; tx++ , tp++)
					{
						if(!tp->master)
						{
							siz=metamap_gettile(mmap,tp,data);
							ret->size+=siz;
							data+=siz;

							tp->index=index++;
						}
					}
				}
		}
	}

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// build tilemap
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

datachunk * mmap_buildtilemap( metamap *mp )
{
datachunk *ret;

s32 tx,ty;
metamap_tile *tp;

s32 presize;
//s32 numofchars;


u8* data;

//u32 dat;

//u8 *pp;

//s32 siz;

metamap_tile *tmp;


	presize=sizeof(datachunk) + (2*mp->width_in_tiles*mp->height_in_tiles);

	ret=(datachunk*) calloc( presize , 1);

	if(ret)
	{
		ret->totalsize=presize-sizeof(datachunk);
		ret->size=(2*mp->width_in_tiles*mp->height_in_tiles);

		data=ret->data;

		for(ty=0;ty<mp->height_in_tiles;ty++)
		{
			for( tx=0 , tp=mp->map+(ty*mp->width_in_tiles) ; tx<mp->width_in_tiles ; tx++ , tp++)
			{
				for( tmp=tp ; tmp->master ; tmp=tmp->master ) ; // get master for this tile

				data[0]=((tmp->index>>0)&0xff);
				data[1]=((tmp->index>>8)&0xff);

				data+=2;
			}
		}

	}

	return ret;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// pu crunch a datachunk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

datachunk * datachunk_pucrunch( datachunk *dp )
{
u8 *out;
s32 siz;

s32 presize;

datachunk *ret;

	main_pucrunch(dp->data,dp->size,&out,&siz);

	if(!siz) return 0;

	if(siz>dp->totalsize) // need to realloc?
	{
		presize=sizeof(datachunk) + (siz);

		ret=(datachunk*) calloc( presize , 1);

		if(!ret)
		{
			return 0;
		}

		ret->size=dp->size;
		ret->totalsize=dp->totalsize;
	}
	else
	{
		ret=dp;
	}

	memcpy(ret->data,out,siz);
	ret->size=siz;

	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// dump a datachunk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void datachunk_dump( datachunk *dp , FILE *fp , const char *str)
{
u8 *p;

	fprintf(fp,"[%d] %s =\n{",dp->size,str);


	for(p=dp->data ; p<dp->data+dp->size ; p++)
	{
		if(((p-dp->data)&0x1f)==0)
		{
			fprintf(fp,"\n");
		}
		fprintf(fp,"0x%02x,",*p);
	}
	if(((p-dp->data)&0x1f)!=1)
	{
		fprintf(fp,"\n");
	}
	fprintf(fp,"};\n\n");
}


#endif

