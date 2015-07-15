/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// current version information for read/write
//
#define U32_ID4_XSX0  U32_ID4('X','S','X','0')
#define XSX0_VERSION 19


#define XSX0_RELOCATE(target,type,base)	if(target) { target=(type*) ( ((u32)(base)) + ((u32)(target)) ); }


// the twiddle function converts into little endian if this code is run on a big endian system


struct XSX0_header
{
	u32 id;
	u32 version;
	u32 filesize; // not including this header

	void twiddle(void)
	{
	}
	struct XSX0_info *data(void)
	{
		return (struct XSX0_info *) (((u8*)(this))+12);
	}
};

#define XSX0_KEY_TYPE_MASK	0x0000000f
#define XSX0_KEY_TYPE_TCB	0x00000000
#define XSX0_KEY_TYPE_LINE	0x00000003
#define XSX0_KEY_TYPE_STEP	0x00000004

struct XSX0_key
{
	f32		frame;		// 1.0 == 1 sec

	f32		value;

	s32		flags;

	f32		t;
	f32		c;
	f32		b;

	void twiddle(void)
	{
	}
};

struct XSX0_stream
{
	char name[32]; // name of stream if its a morph stream 

	s32 numof_keys;

	XSX0_key		*keys;

	void twiddle(void)
	{
	}
};


enum XSX0_item_type
{

XSX0_item_type_none=0,

XSX0_item_type_max,

};



//
// draw object flipped along the x,y,z axis
//
#define XSX0_ITEM_FLAG_FLIPX 0x00000001
#define XSX0_ITEM_FLAG_FLIPY 0x00000002
#define XSX0_ITEM_FLAG_FLIPZ 0x00000004

struct XSX0_item
{
	char name[256]; // possibly file nameish, maybe even with some path 

	s32 type;

	s32 flags;

	s32 numof_streams;

	XSX0_item		*parent;

	XSX0_stream		*streams; //first 9 are x,y,z h,p,b and x,y,z scales

	void twiddle(void)
	{
	}
};

struct XSX0_info
{
	s32 numof_items;
	s32 numof_streams;
	s32 numof_keys;

	XSX0_item		*items;
	XSX0_stream		*streams;
	XSX0_key		*keys;

	f32 start;		// in secs
	f32 length;		// in secs

	void twiddle(void)
	{
	}

//
// Relocate the file offsets into pointers
//
	void relocate(void)
	{
	XSX0_item  *item;
	XSX0_stream  *stream;

		XSX0_RELOCATE(	items		,	XSX0_item		,	this	)
		XSX0_RELOCATE(	streams		,	XSX0_stream		,	this	)
		XSX0_RELOCATE(	keys		,	XSX0_key		,	this	)

		for( item=items ; item<items+numof_items ; item++ )
		{
			XSX0_RELOCATE(	item->parent	,	XSX0_item		,	this	)
			XSX0_RELOCATE(	item->streams	,	XSX0_stream		,	this	)
		}

		for( stream=streams ; stream<streams+numof_streams ; stream++ )
		{
			XSX0_RELOCATE(	stream->keys	,	XSX0_key		,	this	)
		}
	}
};
