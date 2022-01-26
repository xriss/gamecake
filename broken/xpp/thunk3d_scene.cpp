/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_scene::setup(struct thunk3d *_thunk3d)
{
	DMEM_ZERO(this);

	master=_thunk3d;

	DHEAD_INIT(items);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reset a scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_scene::reset(void)
{
thunk3d *_thunk3d;

	_thunk3d=master;

	clean();
	return setup(_thunk3d);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free a scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void t3d_scene::clean(void)
{
t3d_item	*item,*itemnext;

	for( item=items->first ; itemnext=item->next ; item=itemnext )
	{
		master->FreeItem(item);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_scene *thunk3d::AllocScene(void)
{
t3d_scene *ret;


	if(!(ret=(t3d_scene *)scenes->alloc()))
	{
		DBG_Error("Failed to allocate thunk3D.scene.\n");
		goto bogus;
	}

	if(!ret->setup(this))
	{
		DBG_Error("Failed to setup thunk3D.scene.\n");
		goto bogus;
	}

	DLIST_PASTE(scenes->atoms->last,ret,0);

	return ret;

bogus:
	FreeScene(ret);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free an scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void thunk3d::FreeScene(t3d_scene *item)
{
	if(item)
	{
		DLIST_CUT(item);

		item->clean();

		scenes->free((llatom*)item);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load a scene
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_scene::LoadLWS(const char *fname)
{
char buf[8][1024];

FILE *fp=0;

s32 d[16];
f32 f[16];


enum {

	PSI_NONE=0,

	PSI_DEFAULT,
	PSI_OBJECT,
	PSI_NULL,
	PSI_LIGHT,
	PSI_CAMERA,
	PSI_ENVELOPE,


	PSI_MAX
} state_stack[32];

s32 state_depth=0;


t3d_item *item;
t3d_stream *stream;
t3d_key *key;

s32 object_id=0x10000000;
s32 light_id=0x20000000;
s32 camera_id=0x30000000;

s32 version=3;

	state_stack[state_depth]=PSI_DEFAULT;

	if(!(fp=fopen(fname,"r")))
	{
		DBG_Error("Failed to open LWS file \"%s\".\n",fname);
		goto bogus;
	}

	fgets(buf[0],sizeof(buf[0]),fp);
	sscanf(buf[0]," %s ",buf[1]);
	if( strcmp("LWSC",buf[1])!=0 )
	{
		DBG_Error("Not an LWS file (bad header) \"%s\".\n",fname);
		goto bogus;
	}

	fgets(buf[0],sizeof(buf[0]),fp);
	sscanf(buf[0]," %d ",d+0);
	if( d[0]<3 )
	{
		DBG_Error("Not an LWS file (wrong version) \"%s\".\n",fname);
		goto bogus;
	}
	version=d[0];

	numof_items=0;
	numof_streams=0;
	numof_keys=0;

	while(fgets(buf[0],sizeof(buf[0]),fp)) // scan file a line at a time
	{

		if(state_depth>=30) // we've bollocked up some how, best say so
		{
			DBG_Error("Internal error, LWS stack parse overflow.\n");
			goto bogus;
		}

		switch( state_stack[state_depth] )
		{
	 		case PSI_DEFAULT:
			{
				if( 1 == sscanf(buf[0]," FirstFrame %d ",d+0) )
				{
					first_frame=(f32)d[0];
				}
				else
				if( 1 == sscanf(buf[0]," LastFrame %d ",d+0) )
				{
					last_frame=(f32)d[0];
				}
				else
				if( 1 == sscanf(buf[0]," FramesPerSecond %d ",d+0) )
				{
					frames_per_second=(f32)d[0];
				}
				else
				{
					if ( version > 3 )
					{
						if( 3 == sscanf(buf[0]," LoadObjectLayer %d %x %s ",d+0,d+1,buf[1]) )
						{
							state_depth++;
							state_stack[state_depth]=PSI_OBJECT;

							if( ! ( item = master->AllocItem() ) ) { goto bogus; }
							DLIST_CUTPASTE(items->last,item,0);
							numof_items++;

							strncpy(item->name,buf[1],sizeof(item->name)); item->name[sizeof(item->name)-1]=0;

							item->id=object_id++;

							if(d[0]==2)
							{
								item->flags=T3D_ITEM_FLAG_FLIPX;
							}

							printf("+object\t%d %08x %s\n",d[0],item->id,item->name);
						}
						else
						if( 2 == sscanf(buf[0]," AddNullObject %x %s ",d+1,buf[1]) )
						{
							state_depth++;
							state_stack[state_depth]=PSI_NULL;

							if( ! ( item = master->AllocItem() ) ) { goto bogus; }
							DLIST_CUTPASTE(items->last,item,0);
							numof_items++;

							strncpy(item->name,buf[1],sizeof(item->name)); item->name[sizeof(item->name)-1]=0;

							item->id=object_id++;

							printf("+null\t  %08x %s\n",item->id,item->name);
						}
					}
					else
					{
						if( 2 == sscanf(buf[0]," LoadObjectLayer %d %s ",d+0,buf[1]) )
						{
							state_depth++;
							state_stack[state_depth]=PSI_OBJECT;

							if( ! ( item = master->AllocItem() ) ) { goto bogus; }
							DLIST_CUTPASTE(items->last,item,0);
							numof_items++;

							strncpy(item->name,buf[1],sizeof(item->name)); item->name[sizeof(item->name)-1]=0;

							item->id=object_id++;

							if(d[0]==2)
							{
								item->flags=T3D_ITEM_FLAG_FLIPX;
							}

							printf("+object\t%d %08x %s\n",d[0],item->id,item->name);
						}
						else
						if( 1 == sscanf(buf[0]," AddNullObject %s ",buf[1]) )
						{
							state_depth++;
							state_stack[state_depth]=PSI_NULL;

							if( ! ( item = master->AllocItem() ) ) { goto bogus; }
							DLIST_CUTPASTE(items->last,item,0);
							numof_items++;

							strncpy(item->name,buf[1],sizeof(item->name)); item->name[sizeof(item->name)-1]=0;

							item->id=object_id++;

							printf("+null\t  %08x %s\n",item->id,item->name);
						}
					}
				}
			}
			break;

	 		case PSI_OBJECT:
	 		case PSI_NULL:
	 		case PSI_LIGHT:
	 		case PSI_CAMERA:
			{
				if( 1 == sscanf(buf[0]," ParentItem %x ",d+0) )
				{
					item->parentid=d[0];
				}
				else
				if( 1 == sscanf(buf[0]," ShadowOptions %d ",d+0) ) // use this as terminator
				{
					state_depth--;
				}
				else
				if( 1 == sscanf(buf[0]," Channel %d ",d+0) ) // start envelope
				{
					state_depth++;
					state_stack[state_depth]=PSI_ENVELOPE;

					if( ! ( stream = master->AllocStream() ) ) { goto bogus; }
					DLIST_CUTPASTE(item->streams->last,stream,0);
					numof_streams++;
					item->numof_streams++;

					stream->id=d[0];


//					printf("+envelope\n");
				}
				else
				if( 1 == sscanf(buf[0]," { %s ",buf[1]) ) // start morph envelope ?
				{
					if( (strcmp("MorfForm",buf[1])==0) || (strcmp("KeyedMorph",buf[1])==0) )
					{
						state_depth++;
						state_stack[state_depth]=PSI_ENVELOPE;

						if( ! ( stream = master->AllocStream() ) ) { goto bogus; }
						DLIST_CUTPASTE(item->streams->last,stream,0);
						numof_streams++;
						item->numof_streams++;
					}
				}


			}
			break;

	 		case PSI_ENVELOPE:
			{
				if( 1 == sscanf(buf[0]," \"%s\" ",buf[1]) )
				{
					buf[1][strlen(buf[1])-1]=0; //kill that last "
					strncpy(stream->name,buf[1],sizeof(stream->name)); stream->name[sizeof(stream->name)-1]=0;
				}
				else
				if( 9 == sscanf(buf[0]," Key %f %f %d %f %f %f %f %f %f ",f+0,f+1,d+2,f+3,f+4,f+5,f+6,f+7,f+8) )
				{
					if( ! ( key = master->AllocKey() ) ) { goto bogus; }
					DLIST_CUTPASTE(stream->keys->last,key,0);
					numof_keys++;
					stream->numof_keys++;

					key->value=f[0];
					key->time=f[1];
					key->type=d[2];
					key->t=f[3];
					key->c=f[4];
					key->b=f[5];

//					printf("+key\n");

				}
				else
				if( 2 == sscanf(buf[0]," Behaviors %d %d ",d+0,d+1) ) // use this as terminator
				{
					stream->behaviour[0]=d[0];
					stream->behaviour[1]=d[1];

					state_depth--;
				}
			}
			break;

			default:
			{
			}
			break;
		}
	}

// step through and link items via parents using the item IDs read in


	for( item=items->first ; item->next ; item=item->next )
	{
		if(item->parentid)
		{
			item->parent=find_item(item->parentid);
		}
	}




	if(fp) { fclose(fp); fp=0; }
	return true;
bogus:
	if(fp) { fclose(fp); fp=0; }
	return false;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find a scene item by its ID thingy
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_item *t3d_scene::find_item(s32 id)
{
t3d_item *item;


	for( item=items->first ; item->next ; item=item->next )
	{
		if(item->id==id)
		{
			return item;
		}
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sort items using parents, suh that if renderd in order the parents will alwyas have been rendered first
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_scene::sort_items(void)
{
t3d_item *item,*itemnext;
t3d_item *itemb;


// assign scene depth to each item

	for( item=items->first ; item->next ; item=item->next )
	{
		item->depth=0;

		for ( itemb=item->parent; itemb ; itemb=itemb->parent )
		{
			item->depth++;
		}
	}

// sort by depth

	for( item=items->first ; itemnext=item->next ; item=itemnext )
	{
		for( itemb=item->prev ; ( itemb->prev && ( itemb->depth > item->depth ) ) ; itemb=itemb->prev )
		{
		}
		DLIST_CUTPASTE(itemb,item,0);

	}

	return true;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save data as an XOX output object format data
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_scene::SaveXSX(const char *filename)
{
XSX0_header		xsxhead[1];
XSX0_info		xsxinfo[1];
XSX0_item		xsxitem[1];
XSX0_stream		xsxstream[1];
XSX0_key		xsxkey[1];

FILE *fp=0;

bool ret;

t3d_item	*item;
t3d_stream	*stream;
t3d_key		*key;



s32 chunk_info		;
s32 chunk_items		;
s32 chunk_streams	;
s32 chunk_keys		;

s32 chunk_pos;

char *cp;

s32 id;

//XOX0_morph_point *mpoff;

	ret=false;

	sort_items();

//
// assign output IDs to each wossname
//
	id=0;
	for( item=items->first ; item->next ; item=item->next )
	{
		item->output_id=id++;
	}
	id=0;
	for( item=items->first ; item->next ; item=item->next )
	for( stream=item->streams->first ; stream->next ; stream=stream->next )
	{
		stream->output_id=id++;
	}
	id=0;
	for( item=items->first ; item->next ; item=item->next )
	for( stream=item->streams->first ; stream->next ; stream=stream->next )
	for( key=stream->keys->first ; key->next ; key=key->next )
	{
		key->output_id=id++;
	}

// open file

	if(!(fp=fopen(filename,"wb")))
	{
		DBG_Error("Failed to open output file \"%s\".\n",filename);
		goto bogus;
	}


	xsxinfo->numof_items=numof_items;
	xsxinfo->numof_streams=numof_streams;
	xsxinfo->numof_keys=numof_keys;

	chunk_info			=	( 1								*	sizeof(XSX0_info)		) ;
	chunk_items			=	( xsxinfo->numof_items			*	sizeof(XSX0_item)		) ;
	chunk_streams		=	( xsxinfo->numof_streams		*	sizeof(XSX0_stream)		) ;
	chunk_keys			=	( xsxinfo->numof_keys			*	sizeof(XSX0_key)		) ;
	chunk_pos			=	0;

	xsxhead->id=U32_ID4_XSX0;
	xsxhead->version=XSX0_VERSION;
	xsxhead->filesize= chunk_info ;


	xsxinfo->items=(XSX0_item*)					(	chunk_info																				);
	xsxinfo->streams=(XSX0_stream*)				(	chunk_info+chunk_items																	);
	xsxinfo->keys=(XSX0_key*)					(	chunk_info+chunk_items+chunk_streams													);


	xsxinfo->start=((f32)(first_frame))/((f32)(frames_per_second));
	xsxinfo->length=((f32)(last_frame+1-first_frame))/((f32)(frames_per_second));

//
// dump out header
//
	xsxhead->twiddle();
	if(1!=fwrite((void*)xsxhead,12,1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xsxhead->twiddle();

//
// dump out info
//
	xsxinfo->twiddle();
	if(1!=fwrite((void*)xsxinfo,sizeof(xsxinfo),1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xsxinfo->twiddle();

//
// dump out items
//
	for( item=items->first ; item->next ; item=item->next )
	{
		for( cp=xsxitem->name ; cp < xsxitem->name+sizeof(xsxitem->name) ; cp ++ ) { *cp=0; }
		strncpy(xsxitem->name,item->name,sizeof(xsxitem->name)-1);

		xsxitem->numof_streams = item->numof_streams;
		xsxitem->streams=xsxinfo->streams + item->streams->first->output_id;

		if(item->parent)
		{
			xsxitem->parent= xsxinfo->items + item->parent->output_id ;
		}
		else
		{
			xsxitem->parent=0;
		}

		xsxitem->flags=0;

		if(item->flags&T3D_ITEM_FLAG_FLIPX)
		{
			xsxitem->flags|=XSX0_ITEM_FLAG_FLIPX;
		}



		xsxitem->twiddle();
		if(1!=fwrite((void*)xsxitem,sizeof(xsxitem),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xsxitem->twiddle();
	}

//
// dump out streams
//
	for( item=items->first ; item->next ; item=item->next )
	for( stream=item->streams->first ; stream->next ; stream=stream->next )
	{
		for( cp=xsxstream->name ; cp < xsxstream->name+sizeof(xsxstream->name) ; cp ++ ) { *cp=0; }
		strncpy(xsxstream->name,stream->name,sizeof(xsxstream->name)-1);

		xsxstream->numof_keys=stream->numof_keys;
		xsxstream->keys=xsxinfo->keys + stream->keys->first->output_id;

		xsxstream->twiddle();
		if(1!=fwrite((void*)xsxstream,sizeof(xsxstream),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xsxstream->twiddle();
	}

//
// dump out keys
//
	for( item=items->first ; item->next ; item=item->next )
	for( stream=item->streams->first ; stream->next ; stream=stream->next )
	for( key=stream->keys->first ; key->next ; key=key->next )
	{
		xsxkey->frame=key->time;
		xsxkey->value=key->value;
		xsxkey->flags=key->type;
		xsxkey->t=key->t;
		xsxkey->c=key->c;
		xsxkey->b=key->b;


		xsxkey->twiddle();
		if(1!=fwrite((void*)xsxkey,sizeof(xsxkey),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xsxkey->twiddle();
	}


	ret=true;
bogus:
	if(fp) { fclose(fp); }
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// to find the master item, there should only be one of these
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/

t3d_item* t3d_scene::master_item(void)
{
t3d_item*item;

	for( item=items->first ; item->next ; item=item->next )
	{
		if( item->parent==0 )
		{
			return item;
		}
	}

	return 0;
}

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// to loop over all children, pass in a null item on first iteration
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/

t3d_item* t3d_scene::child_item(t3d_item *parent , t3d_item *item)
{

	if( item==0 ) { item=items->first; }
	else		  { if(item->next) { item=item->next; } }

	while( item->next )
	{
		if( item->parent==parent )
		{
			return item;
		}
		item=item->next;
	}

	return 0;
}
