/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2010 http://XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

#define glError() { \
	GLenum err = glGetError(); \
	while (err != GL_NO_ERROR) { \
		fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
		err = glGetError(); \
	} \
}


struct XOX_surface: public XOX0_surface
{
};

struct XOX_point: public XOX0_point
{
};

struct XOX_morph // some extra morph info
{
	f32 f;
	s32 link; // link to this stream id (0 for no link) and >=9 for valid link
};

struct XOX
{
	const struct XOX0_info *info;
	
	s32 numof_surfaces;
	s32 numof_points;
	s32 numof_morphs;
	
	XOX_surface		*surfaces;
	XOX_point		*points;
	XOX_morph		*morphs;
	
	bool			flipX; // set if need to X mirror this object
	
	bool			need_rebuild; //set if we need to rebuild morphs
};

struct XSX_stream
{
	const struct XSX0_stream		*stream;
	XSX0_key *key; // the last key we found, search from here for next one

	f32 value; // the sampled value for this stream

	u8 mid[4];	// morph link id for object 0-3
	
	void update(f32 frame);
};

struct XSX_item
{
	const struct XSX0_item		*item;
	
	struct XSX_item *parent;
	
	f32	morphs[4]; // some morphs modifier for this object.
	v3	siz[1]; // size modifier for this object.
	v3	pos[1]; // position modifier for this object.
	tfm mytfm[1]; // transform
	m44 mat[1]; // sampled transform (including all parents)

	struct XOX * xoxs[4]; // up to 4 objects to render...
	
	s32				numof_streams;
	XSX_stream		*streams;
	
	inline void buildmat(m44 *m)
	{
	m44 p[1];
	m44 t[1];

		if(parent)
		{
			parent->buildmat(p);
			t->set(mytfm);
			m44_mul_m44(m,p,t);
		}
		else
		{
			m->set(mytfm);
		}
	}

};

struct XSX
{
	const struct XSX0_info *info;
	
	void *memory; // a malloced memory chunk for the items and streams
	s32   memory_size;
	
	s32				numof_streams;
	struct XSX_stream *streams;
	
	s32				numof_items;
	struct XSX_item *items;
};


struct fogl_fbo
{
	s32 width;
	s32 height;
	s32 depth; // if non 0 then we want a depth buffer
	
	s32 flags;

	GLenum status;
	
	GLuint texture_buffer;
	GLuint depth_buffer;
	GLuint frame_buffer;
	
};

struct fogl_tex
{
	s32 width;
	s32 height;
	s32 depth; // if non 0 then we want a depth buffer
	
	s32 flags;

	GLenum status;
	
	GLuint texture_buffer;
};

struct fogl_glyph
{
	f32 top;
	f32 left;
	f32 width;
	f32 height;
	f32 advance;
};

// a main window class to hangs everything off of

struct fenestra_ogl
{
	struct fenestra *fenestra; // up pointer

	bool setup(struct fenestra * _fenestra);
	bool setup_viewport(int _width,int _height);
	void clean(void);

	void clip2d(float xp, float yp, float xh, float yh);
	void project23d(float aspect, float fov, float depth);

	void begin(s32 _width,s32 _height); // pass in size or 0,0 to use current window size
	void swap();

	int set_target(int w,int h);

	void draw_cube(float size);
	
	s32  xox_sizeof(const struct XOX0_info *xsx_info);
	void xox_setup(struct XOX *xox ,const struct XOX0_info *xox_info);
	void xox_update( struct XOX *xox);
	void xox_draw( struct XOX *xox);
	void xox_clean(struct XOX *xox);

	s32  xsx_sizeof(const struct XSX0_info *xsx_info);
	void xsx_setup(struct XSX *xsx ,const struct XSX0_info *xsx_info);
	void xsx_update( struct XSX *xsx,f32 f);
	void xsx_draw( struct XSX *xsx);
	void xsx_clean(struct XSX *xsx);


	bool debug_setup();

	void debug_font_start();
	void debug_font_position(f32 x, f32 y, f32 size , u32 color);
	void debug_font_draw(char c);
	void debug_font_draw_string(const char *string);
	void debug_font_draw_string_alt(const char *string);
	void debug_font_end();
	void debug_font_string(f32 x, f32 y, f32 size , u32 color , const char *string);
	void debug_rect(f32 x1, f32 y1, f32 x2, f32 y2, u32 argb);
	void debug_polygon_begin();
	void debug_polygon_vertex(f32 x,f32 y,u32 argb);
	void debug_polygon_end();

	bool fbo_setup(struct fogl_fbo *fbo);
	void fbo_clean(struct fogl_fbo *fbo);
	bool fbo_bind(struct fogl_fbo *fbo);
	bool fbo_texture(struct fogl_fbo *fbo);

	bool tex_setup(struct fogl_tex *tex);
	void tex_clean(struct fogl_tex *tex);
	bool tex_bind(struct fogl_tex *tex);

	bool flat_setup();
	void flat_begin();
	void flat_end();

	bool font_setup();
	void font_position(f32 x, f32 y, f32 size , u32 color);
	void font_draw(char c);
	void font_draw_string(const char *string);
	f32 font_width_string(const char *string);
	s32 font_fit_string( const char *string, f32 width );

#if defined(WIN32)
	HDC hDC;
	HGLRC hRC;
	
#elif defined(X11)

	GLXContext Xcontext;

#endif

	u32 force_diffuse; // replace diffuse from material if !0 (for blooming effect)
	u32 force_spec;    // same with spec
	f32 force_gloss;   // and gloss

	struct fogl_fbo target[1];

// the master window size

	f32 master_width;
	f32 master_height;

// the full window size
	f32 width;
	f32 height;

// the cliped window position/size	
	f32 clip_xp;
	f32 clip_yp;
	f32 clip_xh;
	f32 clip_yh;

//debug font
	GLuint debug_font_chars[96];
	f32 debug_font_x;
	f32 debug_font_y;
	f32 debug_font_size;
	u32 debug_font_color;

//current basic font
	fogl_glyph font_infos[96];
	GLuint font_chars[96];
	f32 font_x;
	f32 font_y;
	f32 font_size;
	u32 font_color;
};


#ifdef __cplusplus
extern "C" {
#endif

	LUALIB_API int luaopen_fenestra_core_ogl (lua_State *l);

#ifdef __cplusplus
};
#endif
