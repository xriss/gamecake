/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup an fbo
// clear it to 0 and set fbo->width and height before calling this function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::fbo_setup(struct fogl_fbo *fbo)
{

//	fbo=(struct fogl_fbo *)calloc(1,sizeof(struct fogl_fbo));
//  fbo->width=320;
//  fbo->height=240;

	if(fbo)
	{
		glGenFramebuffersEXT(1, &fbo->frame_buffer);
		
		if(fbo->depth)
		{
			glGenRenderbuffersEXT(1, &fbo->depth_buffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo->depth_buffer);		
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depth_buffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, fbo->width, fbo->height );
		}
		
		glGenTextures(1, &fbo->texture_buffer);
		glBindTexture(GL_TEXTURE_2D, fbo->texture_buffer);

		
//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo->width, fbo->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenerateMipmapEXT(GL_TEXTURE_2D); // this seems to be very important...
		
// attach to frame buffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo->frame_buffer);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo->texture_buffer, 0);
		if(fbo->depth)
		{
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depth_buffer);
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


// should check for errors here
		fbo->status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		
	}
	
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean an fbo, be sure to unbind it first
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::fbo_clean(struct fogl_fbo *fbo)
{
	if(fbo)
	{
//Delete resources
		glDeleteTextures(1, &fbo->texture_buffer);
		glDeleteRenderbuffersEXT(1, &fbo->depth_buffer);
		glDeleteFramebuffersEXT(1, &fbo->frame_buffer);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// bind the fbo for rendering to, call with 0 ptr to unbind
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::fbo_bind(struct fogl_fbo *fbo)
{
	if(fbo)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo->frame_buffer);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo->texture_buffer, 0);
		if(fbo->depth)
		{
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->depth_buffer);
		}

// should check for errors here
		fbo->status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		
		setup_viewport(fbo->width,fbo->height);
	}
	else
	{
//Bind 0, which means render to back buffer, as a result, fb is unbound
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		setup_viewport(0,0);
	}
	
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// bind the fbo texture ready for rendering from
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::fbo_texture(struct fogl_fbo *fbo)
{
	if(fbo)
	{
		glBindTexture(GL_TEXTURE_2D, fbo->texture_buffer);
/*
//	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_COLOR_MATERIAL );

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

//	glDisable(GL_LIGHTING);
//	glDisable(GL_DEPTH_TEST);
	
	glEnable( GL_TEXTURE_2D );
	
//	glDisable( GL_CULL_FACE );
	
GLfloat color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color );		
		glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, color );
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 1 );
*/		
	}
	
	return true;
}
