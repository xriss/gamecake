/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//#define ARGBU32(a,r,g,b) (((((u32)a))<<24)+((((u32)r))<<16)+((((u32)g))<<8)+(((u32)b)))

#define T3D_MAX_BONES_PER_VERT 2


struct t3d_bone
{
	LLATOM_DEF(t3d_bone);

	bool setup(void);
	bool reset(void);
	void clean(void);

	s32 id;

	char name[T3D_MAX_BONES_PER_VERT][256];		// used to link to an object in a scene
	f32	weight[T3D_MAX_BONES_PER_VERT];
};
struct t3d_bone_dhead
{
	DHEAD_DEF(t3d_bone)
};


struct t3d_point_dhead
{
	DHEAD_DEF(t3d_point)
};
struct t3d_point
{
	LLATOM_DEF(t3d_point);

	bool setup(void);
	bool reset(void);
	void clean(void);

	s32 id; // point unique ID or morph group ID


	f32 x,y,z;
	f32 nx,ny,nz;

	t3d_point_dhead morphs[1];

	t3d_bone *bone;

	s32 output_index;

	t3d_point * find_morph_point(struct t3d_morph *morph);
};

struct t3d_surface
{
	LLATOM_DEF(t3d_surface);

	bool setup(void);
	bool reset(void);
	void clean(void);

	char name[32];

	f32	 a,r,g,b;		// diffuse color
	f32	 sa,sr,sg,sb;	// specular color (default to white, alpha is ignored)
	f32  gloss;

	s32		min_point; // point reference range of all polys in this surface
	s32		max_point;

	u16  *polys_xox_indexs; 
	u16  *polys_xox_strips; 
	s32   polys_xox_numof_strips;

	s32 id;
};
struct t3d_surface_dhead
{
	DHEAD_DEF(t3d_surface)
};


struct t3d_poly
{
	LLATOM_DEF(t3d_poly);

	bool setup(void);
	bool reset(void);
	void clean(void);

	s32 id;
	
	t3d_surface *surface;

	t3d_point *points[3];
	t3d_point *maps[3];			// reuse point structs as uv texture cord values

	f32 nx,ny,nz;

	s32 strip; // strip id we belong to
	s32 strip_link;
	s32 strip_length;

	s32 bone_id; // the lowest bone id of all points in this poly, used for sorting, -1 for no bones attached

	s32 output_index;
};
struct t3d_poly_dhead
{
	DHEAD_DEF(t3d_poly)
};



struct t3d_morph
{
	LLATOM_DEF(t3d_morph);

	bool setup(void);
	bool reset(void);
	void clean(void);

	char name[32];

	float *morphs_xox;
	s32    morphs_xox_sizeof;

	s32 id;

	s32 min_pointid;
	s32 max_pointid;
	s32 numof_points;

};
struct t3d_morph_dhead
{
	DHEAD_DEF(t3d_morph)
};



struct t3d_object
{
	LLATOM_DEF(t3d_object);

	bool setup(struct thunk3d *_thunk3d);
	bool reset(void);
	void clean(void);

	struct thunk3d *master;

	lwObject *lwobj;


	s32 numof_points;
	s32 numof_maps;
	s32 numof_polys;
	s32 numof_polyindexs;
	s32 numof_polystrips;
	s32 numof_surfaces;
	s32 numof_morphs;
	s32 numof_bones;

	t3d_point_dhead		points[1];
	t3d_point_dhead		maps[1];
	t3d_poly_dhead		polys[1];
	t3d_surface_dhead	surfaces[1];
	t3d_morph_dhead		morphs[1];
	t3d_bone_dhead		bones[1];

	float *points_xox;
	s32    points_xox_sizeof;

	u16   *polys_xox;
	s32    polys_xox_sizeof;


	f32		maxrad; // bounding radius from origin
	v3		min[1];	// minimum values of local axis alligned bounding box
	v3		max[1];	// maximum values of local axis alligned bounding box


	bool FillObject(lwObject *lwo, s32 layer);


	bool SortPolys();
	bool SortPoints();

	bool BuildBounds();
	bool BuildNormals();
	bool BuildBones();

	bool BuildXOX();
	bool SaveXOX(const char *filename);


	t3d_point * findpoint(s32 index);
	t3d_point * findmap(s32 index);
	t3d_poly * findpoly(s32 index);
	t3d_surface * findsurface(s32 index);
	t3d_morph * findmorph(s32 index);
	t3d_bone * findbone(s32 index);

	t3d_bone * ref_bone(const char *name0, f32 weight0 , const char *name1, f32 weight1 );

};



enum t3d_key_type
{
	T3D_KEY_TYPE_LINEAR=0,
	T3D_KEY_TYPE_STEP=3,
	T3D_KEY_TYPE_TCB=4,
	T3D_KEY_TYPE_BEZIER=7,
};

struct t3d_key
{
	LLATOM_DEF(t3d_key);

	bool setup(void);
	bool reset(void);
	void clean(void);

	s32 id;

	s32 type; // tween type for this key, obviously it doesnt really make any sense to mix bezier and tcb

	f32 time; // 1.0 == 1sec
	f32	value;
	
	f32 bezier_time[2];	// bezier control points, relative to time and value 
	f32 bezier_value[2];

	f32 t,c,b; // tcb control points

	s32 output_id;

};
struct t3d_key_dhead
{
	DHEAD_DEF(t3d_key)
};

struct t3d_stream
{
	LLATOM_DEF(t3d_stream);

	bool setup(struct thunk3d *_thunk3d);
	bool reset(void);
	void clean(void);

	struct thunk3d *master;

	char	name[32];
	s32		id;

	s32		behaviour[2];

	s32 numof_keys;

	t3d_key_dhead		keys[1];

	s32 output_id;

	t3d_key *find_key(s32 index);
	void sort_keys(void);
	void reID_keys(void);

	void delete_keys(void);

};
struct t3d_stream_dhead
{
	DHEAD_DEF(t3d_stream)
};

//
// draw object flipped along the x,y,z axis
//
#define T3D_ITEM_FLAG_FLIPX 0x00000001
#define T3D_ITEM_FLAG_FLIPY 0x00000002
#define T3D_ITEM_FLAG_FLIPZ 0x00000004

struct t3d_item
{
	LLATOM_DEF(t3d_item);

	bool setup(struct thunk3d *_thunk3d);
	bool reset(void);
	void clean(void);

	struct thunk3d *master;

	char name[256];

	char type[256];

	s32 id;

	s32 numof_streams;

	t3d_stream_dhead	streams[1];

	f32 rest_values[9]; // the first key values of the first 9 streams (defaults to 0 or 1 if no keys in stream)

	s32 parentid;
	struct t3d_item *parent;

	s32 output_id;

	s32 depth;

	s32 flags;

	t3d_stream *find_stream(s32 id);

	void fill_rest_values(void);

};
struct t3d_item_dhead
{
	DHEAD_DEF(t3d_item)
};


struct t3d_scene
{
	LLATOM_DEF(t3d_scene);

	bool setup(struct thunk3d *_thunk3d);
	bool reset(void);
	void clean(void);

	struct thunk3d *master;

	s32 numof_items;
	s32 numof_streams;
	s32 numof_keys;

	t3d_item_dhead		items[1];

	f32 first_frame;
	f32 last_frame;
	f32 frames_per_second;



	bool LoadLWS(const char *fname);

	t3d_item *find_item(s32 id);

	bool sort_items(void);


	t3d_item* master_item(void);
	t3d_item* child_item(t3d_item *parent , t3d_item *last_item);


	bool SaveXSX(const char *filename);
};




struct thunk3d
{
	bool setup(void);
	bool reset(void);
	void clean(void);


	llatoms scenes[1];
	t3d_scene *	AllocScene(void);
	void			 FreeScene(t3d_scene *item);

	llatoms items[1];
	t3d_item *	AllocItem(void);
	void			 FreeItem(t3d_item *item);

	llatoms streams[1];
	t3d_stream *	AllocStream(void);
	void			 FreeStream(t3d_stream *item);

	llatoms keys[1];
	t3d_key *		AllocKey(void);
	void			 FreeKey(t3d_key *item);

	llatoms objects[1];
	t3d_object *	AllocObject(void);
	void			 FreeObject(t3d_object *item);

	llatoms surfaces[1];
	t3d_surface *	AllocSurface(void);
	void			FreeSurface(t3d_surface *item);

	llatoms morphs[1];
	t3d_morph *	    AllocMorph(void);
	void			FreeMorph(t3d_morph *item);

	llatoms polys[1];
	t3d_poly *		AllocPoly(void);
	void			FreePoly(t3d_poly *item);

	llatoms points[1];
	t3d_point *		AllocPoint(void);
	void			FreePoint(t3d_point *item);

	llatoms bones[1];
	t3d_bone *		AllocBone(void);
	void			FreeBone(t3d_bone *item);

};
