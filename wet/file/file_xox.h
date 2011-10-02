/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// current version information for read/write
//
#define U32_ID4_XOX0  U32_ID4('X','O','X','0')
#define XOX0_VERSION 19


#define XOX0_RELOCATE(target,type,base)	if(target) { target=(type*) ( ((u32)(base)) + ((u32)(target)) ); }


// the twiddle function converts into little endian if this code is run on a big endian system


struct XOX0_header
{
	u32 id;
	u32 version;
	u32 filesize; // not including this header

	void twiddle(void)
	{
	}
	
	struct XOX0_info *data(void)
	{
		return (struct XOX0_info *) (((u8*)(this))+12);
	}
};

struct XOX0_morph_point
{
	f32 x,y,z;
	f32 nx,ny,nz;

	void twiddle(void)
	{
	}
};

struct XOX0_point
{
	f32 x,y,z;
	f32 nx,ny,nz;
	f32 u,v;
	u32 argb;			// this is blended with surface color so should be set to 0xffffffff by default

	void twiddle(void)
	{
	}
};


struct XOX0_surface
{
	char name[32];

	u32 argb;
	u32 spec;
	f32 gloss; // 0-1

	u32 flags;

	s32 numof_strips;
	s32 numof_polys;

	u16 *strips;

	s32 index_base;

	s32 min_point;
	s32 max_point;

	void twiddle(void)
	{
	}
};

struct XOX0_morph
{
	char name[32];

	s32 min_point;
	s32 numof_points;

	XOX0_morph_point *points;

	void twiddle(void)
	{
	}
};


struct XOX0_info
{
	s32 numof_surfaces;
	s32 numof_morphs;
	s32 numof_points;
	s32 numof_polys;
	s32 numof_polyindexs;
	s32 numof_polystrips;
	s32 numof_morphpoints;

	XOX0_surface		*surfaces;
	XOX0_morph			*morphs;
	XOX0_point			*points;
	u16					*indexs;
	u16					*strips;
	XOX0_morph_point	*morphpoints;


	f32		maxrad; // bounding radius from origin
	v3		min[1];	// minimum values of local axis aligned bounding box
	v3		max[1];	// maximum values of local axis aligned bounding box

	void twiddle(void)
	{
	}

//
// Relocate the file offsets into pointers
//
	void relocate(void)
	{
	XOX0_surface *surface;
	XOX0_morph *morph;

		XOX0_RELOCATE(	surfaces		,	XOX0_surface		,	this	)
		XOX0_RELOCATE(	morphs			,	XOX0_morph			,	this	)
		XOX0_RELOCATE(	points			,	XOX0_point			,	this	)
		XOX0_RELOCATE(	indexs			,	u16					,	this	)
		XOX0_RELOCATE(	strips			,	u16					,	this	)
		XOX0_RELOCATE(	morphpoints		,	XOX0_morph_point	,	this	)

		for( surface=surfaces ; surface<surfaces+numof_surfaces ; surface++ )
		{
			XOX0_RELOCATE(	surface->strips	,	u16					,	this	)
		}

		for( morph=morphs ; morph<morphs+numof_morphs ; morph++ )
		{
			XOX0_RELOCATE(	morph->points	,	XOX0_morph_point	,	this	)
		}
	}
};
