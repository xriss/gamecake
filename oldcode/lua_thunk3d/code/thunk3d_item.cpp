/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_item::setup(struct thunk3d *_thunk3d)
{
	DMEM_ZERO(this);

	master=_thunk3d;

	DHEAD_INIT(streams);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reset a item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_item::reset(void)
{
thunk3d *_thunk3d;

	_thunk3d=master;

	clean();
	return setup(_thunk3d);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free a item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void t3d_item::clean(void)
{

t3d_stream	*stream,*streamnext;

	for( stream=streams->first ; streamnext=stream->next ; stream=streamnext )
	{
		master->FreeStream(stream);
	}

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_item *thunk3d::AllocItem(void)
{
t3d_item *ret;


	if(!(ret=(t3d_item *)items->alloc()))
	{
		DBG_Error("Failed to allocate thunk3D.item.\n");
		goto bogus;
	}

	if(!ret->setup(this))
	{
		DBG_Error("Failed to setup thunk3D.item.\n");
		goto bogus;
	}

	DLIST_PASTE(items->atoms->last,ret,0);

	return ret;

bogus:
	FreeItem(ret);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free an item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void thunk3d::FreeItem(t3d_item *item)
{
	if(item)
	{
		DLIST_CUT(item);

		item->clean();

		items->free((llatom*)item);
	}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find a stream
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_stream *t3d_item::find_stream(s32 index)
{
t3d_stream *stream;
s32 i;

// first try to find

	for( stream=streams->first , i=0 ; stream->next ; stream=stream->next , i++ )
	{
		if( i==index )
		{
			return stream;
		}
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// update base stream information for this item
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void t3d_item::fill_rest_values(void)
{
t3d_stream *stream;
s32 i;
t3d_stream *stream_def[9]={0};
t3d_key *key;

	for( stream=streams->first , i=0 ; ( (stream->next) && (i<9) ); stream=stream->next , i++ )
	{
		stream_def[i]=stream;
	}


	for( i=0 ; i<9 ; i++ )
	{
		stream=stream_def[i];
		key=0;

		if(stream)
		{
			key=stream->keys->first;

			if( key->next == 0) { key=0; }
		}


		if(key)
		{
			rest_values[i]=key->value;
		}
		else
		{
			if( i < 6 )
			{
				rest_values[i]=0.0f;
			}
			else
			{
				rest_values[i]=1.0f;
			}
		}
	}


}
