/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::setup(struct fenestra * _fenestra)
{
	fenestra=_fenestra;
	
	force_diffuse=0;
	force_spec=0;
	force_gloss=-1;
	
#if defined(WIN32)

    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    hDC = GetDC( fenestra->hwnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24; // 16 is too small?
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat( hDC, &pfd );

    SetPixelFormat( hDC, iFormat, &pfd );

    // create and enable the render context (RC)
    hRC = wglCreateContext( hDC );

    wglMakeCurrent( hDC, hRC );

#elif defined(X11)

	int attrcount;
	int AttributeList[] = {
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			GLX_ALPHA_SIZE, 0,
			GLX_DEPTH_SIZE, 1,
			GLX_STENCIL_SIZE, 0,
			GLX_X_RENDERABLE,1,
//			GLX_RENDER_TYPE, GLX_RGBA_BIT, 
			GLX_DOUBLEBUFFER,1,
//			GLX_TRANSPARENT_TYPE,GLX_NONE,
//			GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
			None};
	GLXFBConfig *conf=glXChooseFBConfig(fenestra->dsp,fenestra->screen,AttributeList,&attrcount);
	

//	for( int i=0 ; conf[i] ; i++ )
	int i=0;
	{
		int v;

#define confdump(dd) glXGetFBConfigAttrib(fenestra->dsp, conf[i], dd, &v); printf( #dd ":0x%X , ",v);
		
		printf( "\nfound glx config %d = { ",i);
		confdump( GLX_FBCONFIG_ID )
		confdump( GLX_VISUAL_ID )
		confdump( GLX_BUFFER_SIZE )
		confdump( GLX_LEVEL )
		confdump( GLX_DOUBLEBUFFER )
		confdump( GLX_STEREO )
		confdump( GLX_AUX_BUFFERS )
		confdump( GLX_RED_SIZE )
		confdump( GLX_GREEN_SIZE )
		confdump( GLX_BLUE_SIZE )
		confdump( GLX_ALPHA_SIZE )
		confdump( GLX_DEPTH_SIZE )
		confdump( GLX_STENCIL_SIZE )
		confdump( GLX_ACCUM_RED_SIZE )
		confdump( GLX_ACCUM_GREEN_SIZE )
		confdump( GLX_ACCUM_BLUE_SIZE )
		confdump( GLX_ACCUM_ALPHA_SIZE )
		confdump( GLX_RENDER_TYPE )
		confdump( GLX_DRAWABLE_TYPE )
		confdump( GLX_X_RENDERABLE )
		confdump( GLX_X_VISUAL_TYPE )
		confdump( GLX_CONFIG_CAVEAT )
		confdump( GLX_TRANSPARENT_TYPE )
		confdump( GLX_TRANSPARENT_INDEX_VALUE )
		confdump( GLX_TRANSPARENT_RED_VALUE )
		confdump( GLX_TRANSPARENT_GREEN_VALUE )
		confdump( GLX_TRANSPARENT_BLUE_VALUE )
		confdump( GLX_TRANSPARENT_ALPHA_VALUE )
		confdump( GLX_MAX_PBUFFER_WIDTH )
		confdump( GLX_MAX_PBUFFER_HEIGHT )
		confdump( GLX_MAX_PBUFFER_PIXELS )
		printf( "}\n");

	}
        
	Xcontext=glXCreateNewContext( fenestra->dsp , conf[0] , GLX_RGBA_TYPE , NULL , true );
glError();
	glXMakeContextCurrent( fenestra->dsp , fenestra->win , fenestra->win,Xcontext );

// this does not work?	
//	glXSwapIntervalEXT(fenestra->dsp , fenestra->win , fenestra->ogl->swap_interval);

glError();

#endif

	int ret;
	
	ret=GLeeInit();
	ret=GLeeForceLink( "GL_VERSION_1_4" );

	debug_setup();
	font_setup(font_base,0);
	font_setup(font_sans,"../../mods/data/fonts/DejaVuSans.ttf");
	font=font_base;


	return true;
}


bool fenestra_ogl::setup_viewport(int _width,int _height)
{
	if((_width>0)&&(_height>0))
	{
		width=(f32)_width;
		height=(f32)_height;
	}
	else // use the master sizes
	{
		width=master_width;
		height=master_height;
	}
	
	/* This is the default view clipping */

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	clip2d(0,0,width,height);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::clean()
{

#if defined(WIN32)
	wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( fenestra->hwnd, hDC );

#elif defined(X11)

	glXMakeCurrent(fenestra->dsp,None,NULL);
	glXDestroyContext(fenestra->dsp,Xcontext);

#endif

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
// clip output to a part of the window
// call before project23d and the projection will be contained within this rectangle
// call after and the rectangle will just clip the projection throwing away what is outside
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::clip2d(float xp, float yp, float xh, float yh)
{
	if( (xp==0.0f)&&(yp==0.0f)&&(xh==0.0f)&&(yh==0.0f) ) // call with all (0,0,0,0) to disable clipping
	{
		clip_xp=0.0f;
		clip_yp=0.0f;
		clip_xh=width;
		clip_yh=height;
		
		glScissor( (int)clip_xp , (int)clip_yp  , (int)clip_xh , (int)clip_yh ); 
		glDisable(GL_SCISSOR_TEST);
		
		return;
	}
	
	// a number less than 1 represents a fraction of height or width
	// note that that is less than 1, 1 still becomes 1
	if(xp<1.0f) { xp*=width; }
	if(xh<1.0f) { xh*=width; }
	if(yp<1.0f) { yp*=height; }
	if(yh<1.0f) { yh*=height; }
	
	if(xp<0.0f) { xp=0.0f; }
	if(yp<0.0f) { yp=0.0f; }
	if(xh<0.0f) { xh=0.0f; }
	if(yh<0.0f) { yh=0.0f; }
	
	if(xp>width)     { xp=width; }
	if(yp>height)    { yp=height; }
	if(xp+xh>width)  { xh=width-xp; }
	if(yp+yh>height) { yh=height-yp; }
	
	clip_xp=xp;
	clip_yp=yp;
	clip_xh=xh;
	clip_yh=yh;
	
	glScissor( (int)clip_xp , (int)clip_yp  , (int)clip_xh , (int)clip_yh ); 
	glEnable(GL_SCISSOR_TEST);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set a simple field of view projection matrix designed to work in 2d or 3d and keep the numbers
// simpler for 2d positioning.
//
// it adjusts itself based upon the width and height of the display window
// setting an FBO will change these values, so call this afterwards
//
// aspect is x/y of what you want to see, setting a z=1 and y=1 would get you the top of the screen
// as would a z=128 and y=128. This means we can treat the z as a simple divider with no adjustment
//
// so setting aspect to 640,480 would mean at a z depth of 240 (which is y/2) then your view area would be
// -320 to +320 in the x any -240 to +240 in the y.
//
// fov is just a viewscale, (the tan of ( 90deg - half the viewscale ) )
// so 1 would be 90deg, 2 would be 45deg and so on, whatever you set this to is also the inverse of what you 
// need to scale your x or y calculations or 3d sizes by when doing 2d positioning
// this angle is a vertical view angle
//
// so again in the above 640,480 example, a fov of 4 (a more natural looking fov) the extents at z=240
// -80 to +80 in the x and -60 to +60 in the y. Which is the original numbers * 0.25
// this fov of 4 gives a viewing angle of about 28 degrees
// 
// This view system is designed to make 2d easier without confusing the 3d too much.
//
// after calling this function you could adjust the projection matrix more using translate or scale
// opengl is left in a glMatrixMode(GL_PROJECTION) state
//
// the depth parameter is only used to limit the range of the zbuffer, 0-depth
//
// The following would be a reasonable default for a 640x480 screen.
//
// view(320/240,4,1024)
//
// then at z=(240/4)=60 , the view area would be -320 +320 , -240 +240 , -60 +(1024-60)
//
// and this is about as simple as it is going to get for 2d things
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::project23d(float aspect, float fov, float depth)
{
	float ooaspect=1.0f/aspect;

// diveide by zero sanity
	clip_xh=clip_xh ? clip_xh : 1;
	clip_yh=clip_yh ? clip_yh : 1;
	width  =width   ? width   : 1;
	height =height  ? height  : 1;
	
	glMatrixMode (GL_PROJECTION);	
	glLoadIdentity();	
	glTranslatef( (2*clip_xp/width)-((width-clip_xh)/width),(2*clip_yp/height)-((height-clip_yh)/height),0);
//	glTranslatef( -0.5,-0.5,0);
	
	float m[16];
	float f=depth+1.0f;
	float n=1.0f;
	
// fit height by default

		m[0] = clip_yh/clip_xh;
		m[5] = 1.0f;
		
		if(m[0] > (ooaspect) ) // we need to fit width instead to keep everything onscreen
		{
			m[0] = (ooaspect)*1.0f;
			m[5] = (ooaspect)*clip_xh/clip_yh;
		}
		
		m[1] = 0.0f;
		m[2] = 0.0f;
		m[3] = 0.0f;

		m[4] = 0.0f;
		m[6] = 0.0f;
		m[7] = 0.0f;
		
		m[8] = 0.0f;
		m[9] = 0.0f;
		m[10] = -(f+n)/(f-n);
		m[11] = -1.0f;

		m[12] = 0.0f;
		m[13] = 0.0f;
		m[14] = -2.0f*f*n/(f-n);
		m[15] = 0.0f;
		
// adjust field of view using scale
	
		m[0]*=fov * clip_xh/width ;
		m[5]*=fov * clip_yh/height ;
		
	glMultMatrixf(m);
//	glLoadMatrixf(m);
//	glScalef(clip_xh/width,clip_yh/height,1);
//	glTranslatef( -(width-(clip_xp+clip_xh)),-(height-(clip_yp+clip_yh)),0);

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::begin(s32 _width,s32 _height)
{
	if( (_width>0) && (_height>0) )
	{
		master_width=_width;
		master_height=_height;
	}
	else
	{
		master_width=fenestra->width;
		master_height=fenestra->height;
	}
	width=master_width;
	height=master_height;
	
// set fill  color to white

	glColor3f(1.0, 1.0, 1.0);
	
	setup_viewport(width,height);
	
// reset model matrix before setting up lights
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	/* clear window */

glError();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_TEXTURE_2D );

//we need these for premultiplied
//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
//glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	GLfloat lightA[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	GLfloat light5[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat light0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat light1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	GLfloat light2[] = { 0.0f, 1.0f, 1.0f, 0.0f };

// the scene is nice and over bright
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light5);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light1);
	
	glLightfv(GL_LIGHT0, GL_POSITION, light2);
	

	glEnable (GL_DEPTH_TEST);
	
	glDisable( GL_TEXTURE_2D );
	
	glEnable(GL_NORMALIZE); // anoying need to rescale normals for sensible lighting...

// force vsync?
#if defined(WIN32)
	if (GLEE_WGL_EXT_swap_control)
	{
		wglSwapIntervalEXT(1);
	}
#endif
	
// 0 alpha means do not draw anything to zbuffer

	glAlphaFunc(GL_GREATER,0.0f);
	glEnable(GL_ALPHA_TEST);


}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::swap()
{

#if defined(WIN32)

	SwapBuffers( hDC );
	
#elif defined(X11)

	glXSwapBuffers( fenestra->dsp, fenestra->win );

#endif

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a texture, and set it as the render buffer, if _w or _h is 0 then do the reverse
// and restore the original
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int fenestra_ogl::set_target(int w,int h)
{

	if((w>0) && (h>0))
	{

		target->width=w;
		target->height=h;
		target->flags=0;
		
		fbo_setup(target);
		fbo_bind(target);

// the new width/height
		width=w;
		height=h;
		
  		return target->status;
	}
	else
	{
		
		fbo_bind(0);
		fbo_clean(target);

// the old width/height		
		width=master_width;
		height=master_height;

		return 0;
  	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::draw_cube(float size)
{

glBegin(GL_QUADS);
  //Quad 1
    glNormal3f(1.0f, 0.0f, 0.0f);   //N1
    glTexCoord2f(0.0f, 1.0f); glVertex3f( size/2, size/2, size/2);   //V2
    glTexCoord2f(0.0f, 0.0f); glVertex3f( size/2,-size/2, size/2);   //V1
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size/2,-size/2,-size/2);   //V3
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size/2, size/2,-size/2);   //V4
  //Quad 2
    glNormal3f(0.0f, 0.0f, -1.0f);  //N2
    glTexCoord2f(0.0f, 1.0f); glVertex3f( size/2, size/2,-size/2);   //V4
    glTexCoord2f(0.0f, 0.0f); glVertex3f( size/2,-size/2,-size/2);   //V3
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size/2,-size/2,-size/2);   //V5
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size/2, size/2,-size/2);   //V6
  //Quad 3
    glNormal3f(-1.0f, 0.0f, 0.0f);  //N3
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size/2, size/2,-size/2);   //V6
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size/2,-size/2,-size/2);   //V5
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size/2,-size/2, size/2);   //V7
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size/2, size/2, size/2);   //V8
  //Quad 4
    glNormal3f(0.0f, 0.0f, 1.0f);   //N4
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size/2, size/2, size/2);   //V8
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size/2,-size/2, size/2);   //V7
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size/2,-size/2, size/2);   //V1
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size/2, size/2, size/2);   //V2
  //Quad 5
    glNormal3f(0.0f, 1.0f, 0.0f);   //N5
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size/2, size/2,-size/2);   //V6
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size/2, size/2, size/2);   //V8
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size/2, size/2, size/2);   //V2
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size/2, size/2,-size/2);   //V4
  //Quad 6
    glNormal3f(1.0f, -1.0f, 0.0f);  //N6
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size/2,-size/2, size/2);   //V7
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size/2,-size/2,-size/2);   //V5
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size/2,-size/2,-size/2);   //V3
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size/2,-size/2, size/2);   //V1
glEnd();

}
