/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup a texture
// clear it to 0 and set tex->width and height before calling this function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::tex_setup(struct fogl_tex *tex, struct grd *g)
{

	if((tex)&&(g))
	{
		tex->width=g->bmap->w;
		tex->height=g->bmap->h;
		
		glGenTextures(1, &tex->texture_buffer);
		glBindTexture(GL_TEXTURE_2D, tex->texture_buffer);

//
// really we need to replace gluBuild2DMipmaps
//
		// build our texture mipmaps
		u32 *p;
		int d=g->bmap->w*g->bmap->h;
		
		for(p=(u32*)(g->bmap->data); p<((u32*)(g->bmap->data))+d; p++ ) // swap grd data
		{
			u32 t=*p;
			*(p)= (0xff000000&(t<<24)) | (0x000000ff&(t>>8)) | (0x0000ff00&(t>>8)) | (0x00ff0000&(t>>8)) ;
		}
		
//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA , g->bmap->w, g->bmap->h, GL_RGBA, GL_UNSIGNED_BYTE, g->bmap->data );

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA , g->bmap->w, g->bmap->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, g->bmap->data );
 	
		for(p=(u32*)(g->bmap->data); p<((u32*)(g->bmap->data))+d; p++ ) // swap grd data back
		{
			u32 t=*p;
			*(p)= (0x000000ff&(t>>24)) | (0x0000ff00&(t<<8)) | (0x00ff0000&(t<<8)) | (0xff000000&(t<<8)) ;
		}
//		
// really we need to replace gluBuild2DMipmaps
//
	
 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
	
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean a texture, be sure to unbind it first
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::tex_clean(struct fogl_tex *tex)
{
	if(tex)
	{
//Delete resources
		glDeleteTextures(1, &tex->texture_buffer);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// bind the fbo texture ready for rendering from
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::tex_bind(struct fogl_tex *tex)
{
	if(tex)
	{
		glBindTexture(GL_TEXTURE_2D, tex->texture_buffer);
	}
	
	return true;
}
