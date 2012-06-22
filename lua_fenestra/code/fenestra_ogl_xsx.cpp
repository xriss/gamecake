/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// update stream value to the given time 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void XSX_stream::update(f32 frame)
{
XSX0_key *a,*b,*c,*d,*k; // pointeres to the 4 pertinant keys or 0 if no key in that slot
//f32 t,t1,t2;


#define KEYINRANGE(k) ( ((k)>=stream->keys) && ((k)<(stream->keys+stream->numof_keys)) )

	if(!KEYINRANGE(key)) // last used key is valid?
	{
		key=stream->keys;
	}

	if( key->frame <= frame ) // search forward for keys
	{
		for( k=key ; KEYINRANGE(k+1) ; k++ )
		{
			if((k+1)->frame>=frame)
			{
				break;
			}
		}
		a=k-1;	b=a+1;	c=b+1;	d=c+1;
	}
	else
	if( key->frame >= frame ) // search backward for keys
	{
		for( k=key ; KEYINRANGE(k-1) ; k-- )
		{
			if((k-1)->frame<=frame)
			{
				break;
			}
		}
		a=k-2;	b=a+1;	c=b+1;	d=c+1;
	}

	if(!KEYINRANGE(a)) { a=0; }
	if(!KEYINRANGE(b)) { b=0; }
	if(!KEYINRANGE(c)) { c=0; }
	if(!KEYINRANGE(d)) { d=0; }

	if( (b==0) && (c!=0) )
	{
		value=c->value;
	}
	else
	if( (b!=0) && (c==0) )
	{
		value=b->value;
	}
	else
	if( (b==0) && (c==0) )
	{
		value=0.0f;
	}
	else // tween it we have 2 keys to play with
	{
		if((c->flags&XSX0_KEY_TYPE_MASK)==XSX0_KEY_TYPE_TCB)
		{
		f32 t,t2,t3,z,h1,h2,h3,h4,dd0,dd0a,dd0b,ds1,ds1a,ds1b,adj0,adj1,d10,tlen;

			tlen=c->frame-b->frame;
			t=(frame-b->frame)/(tlen);
			d10=c->value-b->value;

			t2=t*t;
			t3=t*t2;

			z=3.0f*t2-t3-t3;

			h1=1.0f-z;
			h2=z;

			h3=t3-t2-t2+t;
			h4=t3-t2;

			dd0a=(1.0f-b->t) * ( 1.0f + b->c) * ( 1.0f + b->b) ;
			dd0b=(1.0f-b->t) * ( 1.0f - b->c) * ( 1.0f - b->b) ;

			ds1a=(1.0f-c->t) * ( 1.0f - c->c) * ( 1.0f + c->b) ;
			ds1b=(1.0f-c->t) * ( 1.0f + c->c) * ( 1.0f - c->b) ;

			if(a) { adj0= tlen / ( c->frame - a->frame ) ; } else { adj0=0.0f; }
			if(d) { adj1= tlen / ( d->frame - b->frame ) ; } else { adj1=0.0f; }

			if(a)
			{
				dd0 = adj0 * (dd0a * ( b->value - a->value ) + dd0b * d10);
			}
			else
			{
				dd0 = .5f * (dd0a + dd0b) * d10;
			}

			if(d)
			{
				ds1 = adj1 * (ds1a * d10 + ds1b * ( d->value - c->value ) );
			}
			else
			{
				ds1 = .5f * (ds1a + ds1b) * d10;
			}

			value = b->value * h1 + c->value * h2 + dd0 * h3 + ds1 * h4;

		}
		else
		if((c->flags&XSX0_KEY_TYPE_MASK)==XSX0_KEY_TYPE_LINE)
		{
		f32 t,d;

			t=(frame-b->frame)/(c->frame-b->frame);
			d=c->value-b->value;
			value=b->value + t*d;
		}
		else // default to step
//		if((c->flags&XSX0_KEY_TYPE_MASK)==XSX0_KEY_TYPE_STEP)
		{
			value=b->value;
		}
	}

}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how much memory does an xsx need to fit this info with no other allocations
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 fenestra_ogl::xsx_sizeof(const struct XSX0_info *xsx_info)
{
	s32 mem=0;
	mem+= sizeof(struct XSX)*1;
	mem+= sizeof(struct XSX_stream)*xsx_info->numof_streams;
	mem+= sizeof(struct XSX_item)*xsx_info->numof_items;	
	
	return mem;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xsx_setup( struct XSX *xsx,const struct XSX0_info *xsx_info)
{
XSX_item *item;
XSX_stream *stream;

// we expect all memory needed to already have been allocated following this struct...
	memset((u8*)xsx,0,xsx_sizeof(xsx_info));

	xsx->info=xsx_info;
	
	xsx->numof_streams=xsx_info->numof_streams;
	xsx->numof_items=xsx_info->numof_items;
	
	xsx->items=(struct XSX_item *)(xsx+1);
	xsx->streams=(struct XSX_stream *)(xsx->items+xsx->numof_items);
	
	for( stream=xsx->streams ; stream<xsx->streams+xsx->numof_streams ; stream++ )
	{
		stream->stream=xsx->info->streams + (stream-xsx->streams) ;
	}
	
	for( item=xsx->items ; item<xsx->items+xsx->numof_items ; item++ )
	{
		item->siz->x=1.0f; // default size
		item->siz->y=1.0f;
		item->siz->z=1.0f;
	
		item->item=xsx->info->items + (item-xsx->items) ;
		if(item->item->parent)
		{
			item->parent=xsx->items + (item->item->parent-xsx->info->items) ;
		}
		item->streams=xsx->streams+(item->item->streams-xsx->info->streams);
		item->numof_streams=item->item->numof_streams;
	}
	
// xsx->memory will be 0 if this failed...
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xsx_clean( struct XSX *xsx)
{
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xsx_update( struct XSX *xsx,f32 f)
{
XSX_item *item;
XSX_stream *stream;
XOX_morph *mi;
struct XOX **xox;
f32 nf;
int i;

	for( stream=xsx->streams ; stream<xsx->streams+xsx->numof_streams ; stream++ )
	{
		stream->update(f+xsx->info->start);
	}
	
	for( item=xsx->items ; item<xsx->items+xsx->numof_items ; item++ )
	{

		item->mytfm->vec->x = item->streams[0].value + item->pos->x ;
		item->mytfm->vec->y = item->streams[1].value + item->pos->y ;
		item->mytfm->vec->z = item->streams[2].value + item->pos->z ;

		item->mytfm->rot->reset();

		item->mytfm->rot->rotz(item->streams[5].value);
		item->mytfm->rot->rotx(item->streams[4].value);
		item->mytfm->rot->roty(item->streams[3].value);

		item->mytfm->siz->x = item->streams[6].value * item->siz->x ;
		item->mytfm->siz->y = item->streams[7].value * item->siz->y ;
		item->mytfm->siz->z = item->streams[8].value * item->siz->z ;

	}
	
	for( item=xsx->items ; item<xsx->items+xsx->numof_items ; item++ )
	{
		item->buildmat(item->mat);
		
		
		for(xox=item->xoxs;xox<item->xoxs+4;xox++)
		{
			if(*xox)
			{
				for( mi=(*xox)->morphs , i=0 ; mi<(*xox)->morphs+(*xox)->numof_morphs ; mi++ , i++)
				{
					if(i<4) { nf=item->morphs[i]; } else { nf=0; }
					if(mi->link>0)
					{
						nf+=item->streams[mi->link].value;
					}
					if(nf!=mi->f)
					{
						mi->f=nf;
						(*xox)->need_rebuild=true;
					}
				}
			}
		}
				
	}

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xsx_draw( struct XSX *xsx)
{
XSX_item *item;
struct XOX **xox;

	glMatrixMode (GL_MODELVIEW);
	
//	draw_cube(0.01f);
		
	for( item=xsx->items ; item<xsx->items+xsx->numof_items ; item++ )
	{
		glPushMatrix();
		
		glMultMatrixf(&item->mat->xx);
		
		for(xox=item->xoxs;xox<item->xoxs+4;xox++)
		{
			if(*xox)
			{
				xox_update(*xox);
				xox_draw(*xox);
//		draw_cube(0.1f);
			}
		}
		
//		draw_cube(0.1f);
		
		glPopMatrix();
	}

}
