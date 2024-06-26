/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


static unsigned char const font_bits[16*48]={

0x00,0x18,0x66,0x6c,0x18,0x00,0x70,0x18,0x0c,0x30,0x00,0x00,0x00,0x00,0x00,0x06,
0x00,0x18,0x66,0x6c,0x3e,0x66,0xd8,0x18,0x18,0x18,0xcc,0x30,0x00,0x00,0x00,0x0c,
0x00,0x18,0x00,0xfe,0x60,0xac,0xd0,0x00,0x30,0x0c,0x78,0x30,0x00,0x00,0x00,0x18,
0x00,0x18,0x00,0x6c,0x3c,0xd8,0x76,0x00,0x30,0x0c,0xfc,0xfc,0x00,0x7e,0x00,0x30,
0x00,0x18,0x00,0xfe,0x06,0x36,0xdc,0x00,0x30,0x0c,0x78,0x30,0x00,0x00,0x00,0x60,
0x00,0x00,0x00,0x6c,0x7c,0x6a,0xdc,0x00,0x18,0x18,0xcc,0x30,0x18,0x00,0x18,0xc0,
0x00,0x18,0x00,0x6c,0x18,0xcc,0x76,0x00,0x0c,0x30,0x00,0x00,0x18,0x00,0x18,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,

0x78,0x18,0x3c,0x3c,0x1c,0x7e,0x1c,0x7e,0x3c,0x3c,0x00,0x00,0x00,0x00,0x00,0x3c,
0xcc,0x38,0x66,0x66,0x3c,0x60,0x30,0x06,0x66,0x66,0x18,0x18,0x06,0x00,0x60,0x66,
0xdc,0x78,0x06,0x06,0x6c,0x7c,0x60,0x06,0x66,0x66,0x18,0x18,0x18,0x7e,0x18,0x06,
0xfc,0x18,0x0c,0x1c,0xcc,0x06,0x7c,0x0c,0x3c,0x3e,0x00,0x00,0x60,0x00,0x06,0x0c,
0xec,0x18,0x18,0x06,0xfe,0x06,0x66,0x18,0x66,0x06,0x00,0x00,0x18,0x7e,0x18,0x18,
0xcc,0x18,0x30,0x66,0x0c,0x66,0x66,0x18,0x66,0x0c,0x18,0x18,0x06,0x00,0x60,0x00,
0x78,0x18,0x7e,0x3c,0x0c,0x3c,0x3c,0x18,0x3c,0x38,0x18,0x18,0x00,0x00,0x00,0x18,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,

0x7c,0x3c,0x7c,0x1e,0x78,0x7e,0x7e,0x3c,0x66,0x3c,0x06,0xc6,0x60,0xc6,0xc6,0x3c,
0xc6,0x66,0x66,0x30,0x6c,0x60,0x60,0x66,0x66,0x18,0x06,0xcc,0x60,0xee,0xe6,0x66,
0xde,0x66,0x66,0x60,0x66,0x60,0x60,0x60,0x66,0x18,0x06,0xd8,0x60,0xfe,0xf6,0x66,
0xd6,0x7e,0x7c,0x60,0x66,0x78,0x78,0x6e,0x7e,0x18,0x06,0xf0,0x60,0xd6,0xde,0x66,
0xde,0x66,0x66,0x60,0x66,0x60,0x60,0x66,0x66,0x18,0x06,0xd8,0x60,0xc6,0xce,0x66,
0xc0,0x66,0x66,0x30,0x6c,0x60,0x60,0x66,0x66,0x18,0x66,0xcc,0x60,0xc6,0xc6,0x66,
0x78,0x66,0x7c,0x1e,0x78,0x7e,0x60,0x3e,0x66,0x3c,0x3c,0xc6,0x7e,0xc6,0xc6,0x3c,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x7c,0x3c,0x7c,0x3c,0x7e,0x66,0x66,0xc6,0xc6,0xc6,0x7e,0x3c,0x80,0x3c,0x00,0x00,
0x66,0x66,0x66,0x66,0x18,0x66,0x66,0xc6,0x6c,0x6c,0x0c,0x30,0xc0,0x0c,0x18,0x00,
0x66,0x66,0x66,0x70,0x18,0x66,0x66,0xc6,0x38,0x38,0x18,0x30,0x60,0x0c,0x66,0x00,
0x7c,0x66,0x7c,0x3c,0x18,0x66,0x66,0xd6,0x38,0x18,0x30,0x30,0x30,0x0c,0x00,0x00,
0x60,0x66,0x6c,0x0e,0x18,0x66,0x3c,0xfe,0x38,0x18,0x60,0x30,0x18,0x0c,0x00,0x00,
0x60,0x6e,0x66,0x66,0x18,0x66,0x3c,0xee,0x6c,0x18,0xc0,0x30,0x0c,0x0c,0x00,0x00,
0x60,0x3f,0x66,0x3c,0x18,0x3c,0x18,0xc6,0xc6,0x18,0xfe,0x3c,0x06,0x3c,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,

0x18,0x00,0x60,0x00,0x06,0x00,0x1c,0x00,0x60,0x18,0x0c,0x60,0x18,0x00,0x00,0x00,
0x18,0x00,0x60,0x00,0x06,0x00,0x30,0x00,0x60,0x00,0x00,0x60,0x18,0x00,0x00,0x00,
0x0c,0x3c,0x7c,0x3c,0x3e,0x3c,0x7c,0x3e,0x7c,0x18,0x0c,0x66,0x18,0xec,0x7c,0x3c,
0x00,0x06,0x66,0x60,0x66,0x66,0x30,0x66,0x66,0x18,0x0c,0x6c,0x18,0xfe,0x66,0x66,
0x00,0x3e,0x66,0x60,0x66,0x7e,0x30,0x66,0x66,0x18,0x0c,0x78,0x18,0xd6,0x66,0x66,
0x00,0x66,0x66,0x60,0x66,0x60,0x30,0x3e,0x66,0x18,0x0c,0x6c,0x18,0xc6,0x66,0x66,
0x00,0x3e,0x7c,0x3c,0x3e,0x3c,0x30,0x06,0x66,0x18,0x0c,0x66,0x0c,0xc6,0x66,0x3c,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x18,0x30,0xfe,0xff,
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x18,0x38,0x00,0xff,
0x7c,0x3e,0x7c,0x3c,0x7c,0x66,0x66,0xc6,0xc6,0x66,0x7e,0x18,0x18,0x18,0x00,0xff,
0x66,0x66,0x66,0x60,0x30,0x66,0x66,0xc6,0x6c,0x66,0x0c,0x38,0x18,0x1c,0x00,0xff,
0x66,0x66,0x60,0x3c,0x30,0x66,0x66,0xd6,0x38,0x66,0x18,0x38,0x18,0x1c,0x00,0xff,
0x7c,0x3e,0x60,0x06,0x30,0x66,0x3c,0xfe,0x6c,0x3c,0x30,0x18,0x18,0x18,0x00,0xff,
0x60,0x06,0x60,0x7c,0x1c,0x3e,0x18,0x6c,0xc6,0x18,0x7e,0x1c,0x18,0x38,0x00,0xff,
0x60,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x0c,0x18,0x30,0x00,0xff

};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool fenestra_ogl::font_setup(fogl_font *font,const char *fontfilename)
{
if(font->chars[0]){return true;} // already loaded
if(fontfilename)
{
		
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */

	int error;

		error = FT_Init_FreeType( &library );
		error = FT_New_Face( library,fontfilename,0, &face );
		if(error){ // check file exists
			FT_Done_FreeType(library); // free everything
			return false;
		}
		error = FT_Set_Pixel_Sizes( face, 32, 32 );
		
	int glyph_index;

	int x,y,xx,yy;
	u8 b;
	u8 m;
	u32 *t;
	u32 c;
	bool last;
	u32 bmap[64*64];
	int i=0;

	unsigned char* bp;
	int bpx;
	int bpy;
	int gpl;
	int gpt;
	float gpa;
	fogl_glyph *pfg;

		// allocate texture names
		glGenTextures( 96, font->chars );

		for( i=0; i<96 ; i++ )
		{
			glyph_index = FT_Get_Char_Index( face, 32+i);
			error = FT_Load_Glyph( face, glyph_index , 0 );
			error = FT_Render_Glyph( face->glyph , FT_RENDER_MODE_NORMAL ); 

			bp=face->glyph->bitmap.buffer;
			bpx=face->glyph->bitmap.width+2;
			bpy=face->glyph->bitmap.rows+2;
			
			gpl=(face->glyph->bitmap_left)-1; // adjust by our 1pixel outline
			gpt=(-32+face->glyph->bitmap_top)+3; // move to base line (including our 1pixel outline)
			gpa=(face->glyph->advance.x/64.0f);
			
			if(bpx>64){ bpx=64; } // buffer over run sanity, should not happen
			if(bpy>64){ bpy=64; } // dont bother making this nice, it is just a hack

			pfg=font->infos+i; // cache some info in thsi for later drawing

	// we are rendering at 32x32 so scale down by that	
			pfg->top=gpt/32.0f;
			pfg->left=gpl/32.0f;
			pfg->advance=gpa/32.0f;
			pfg->width=bpx/32.0f;
			pfg->height=bpy/32.0f;

				
	// build a single char			
			for(y=0;y<bpy;y++)
			{
				for(x=0;x<bpx;x++)
				{
					if((x==0)||(y==0)||(x==bpx-1)||(y==bpy-1))
					{
						c=0x00ffffff;
					}
					else
					{
						c=0x00ffffff | (bp[x-1]<<24);
					}
					
					bmap[x+(y*bpx)]=c;
				}
				if(y!=0)
				{
					bp+=face->glyph->bitmap.pitch;
				}
			}
			
			// select our current texture
			glBindTexture( GL_TEXTURE_2D, font->chars[i] );

			// select modulate to mix texture with color for shading
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			// when texture area is small, bilinear filter the closest mipmap
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

			// when texture area is large, bilinear filter the first mipmap
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

			// build our texture mipmaps
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA , bpx, bpy, GL_RGBA, GL_UNSIGNED_BYTE, bmap );
			
		}

		FT_Done_FreeType(library); // free everything
	}
	else // use builtin font
	{
	int xi,yi;
	int x,y,xx,yy;
	u8 b;
	u8 m;
	cu8 *f=font_bits;
	u32 *t;
	u32 c;
	bool last;
	u32 bmap[8*8];
	int i=0;
	fogl_glyph *pfg;

		// allocate texture names
		glGenTextures( 96, font->chars );

		for(yy=0;yy<48;yy+=8) // bitmap data is 6 chars high
		{
			last=false;
			for(xx=0;xx<128;xx+=8) // bitmap data is 16 chars wide
			{
				
	// build a single char			
				for(y=0;y<8;y++)
				{
					f=font_bits+((yy)*16)+(xx/8);
					for(x=0;x<8;x++)
					{
						b=*(f+(y*16));
						m=0x80>>x;
						if(b&m)
						{
							c=0xffffffff; // solid
						}
						else
						{
							c=0x00000000; // transparent
						}
						bmap[x+(y*8)]=c;
					}
				}
				
				pfg=font->infos+i; // cache some info in thsi for later drawing
				
				pfg->top=0;
				pfg->left=0;
				pfg->advance=1;
				pfg->width=1;
				pfg->height=1;


				// select our current texture
				glBindTexture( GL_TEXTURE_2D, font->chars[i] );

				// select modulate to mix texture with color for shading
				glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

				// when texture area is small, bilinear filter the closest mipmap
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

				// when texture area is large, bilinear filter the first mipmap
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

				// build our texture mipmaps
				gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA , 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, bmap );
				
				i++;
			}
		}
	}
	
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::font_position(fogl_font *font, f32 x, f32 y, f32 size, u32 color)
{
	font->x=x;
	font->y=y;
	font->size=size;
	font->color=color;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Draw a single char
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::font_draw(fogl_font *font, char c)
{
	fogl_glyph *pfg;

	GLfloat x,y,siz;
	GLfloat cx,cy;

	x=(GLfloat)font->x;
	y=(GLfloat)font->y;
	siz=(GLfloat)font->size;

	c=c-32;
	if(c<0)   { c=95; } 
	if(c>95)  { c=95; }
	pfg=font->infos+c;

    glBindTexture( GL_TEXTURE_2D, font->chars[c] );
    
	glColor4ub( (font->color>>16)&0xff , (font->color>>8)&0xff , (font->color)&0xff , (font->color>>24)&0xff );
	
	glBegin(GL_QUADS);
			glTexCoord2d(0,1); glVertex2f(x+((pfg->left)*siz),            y-((-pfg->top+pfg->height)*siz) );
			glTexCoord2d(0,0); glVertex2f(x+((pfg->left)*siz),            y-((-pfg->top)*siz)             );
			glTexCoord2d(1,0); glVertex2f(x+((pfg->left+pfg->width)*siz), y-((-pfg->top)*siz)             );
			glTexCoord2d(1,1); glVertex2f(x+((pfg->left+pfg->width)*siz), y-((-pfg->top+pfg->height)*siz) );
	glEnd();

	font->x+=font->size;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// draw a string
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::font_draw_string(fogl_font *font, const char *string)
{
const char *s;

glDisable(GL_LIGHTING);
glEnable( GL_TEXTURE_2D );
	
glColor4ub( (font->color>>16)&0xff , (font->color>>8)&0xff , (font->color)&0xff , (font->color>>24)&0xff );
		
	for(s=string;*s;s++)
	{
		char c=*s;
		fogl_glyph *pfg;
		
		GLfloat x,y,siz;

		x=(GLfloat)font->x;
		y=(GLfloat)font->y;
		siz=(GLfloat)font->size;
		
		c=c-32;
		if(c<0)   { c=95; } 
		if(c>95)  { c=95; }
		pfg=font->infos+c;
		glBindTexture( GL_TEXTURE_2D, font->chars[c] );
		
		glBegin(GL_QUADS);
			glTexCoord2d(0,1); glVertex2f(x+((pfg->left)*siz),            y-((-pfg->top+pfg->height)*siz) );
			glTexCoord2d(0,0); glVertex2f(x+((pfg->left)*siz),            y-((-pfg->top)*siz)             );
			glTexCoord2d(1,0); glVertex2f(x+((pfg->left+pfg->width)*siz), y-((-pfg->top)*siz)             );
			glTexCoord2d(1,1); glVertex2f(x+((pfg->left+pfg->width)*siz), y-((-pfg->top+pfg->height)*siz) );
		glEnd();

		font->x+=(pfg->advance*siz);
		
//		if(font->x>width) break;
	}
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
glDisable( GL_TEXTURE_2D );
glEnable(GL_LIGHTING);

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how wide is this string?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
f32 fenestra_ogl::font_width_string(fogl_font *font, const char *string)
{
const char *s;
f32 w=0;
GLfloat siz=(GLfloat)font->size;
		
	for(s=string;*s;s++)
	{
		char c=*s;
		
		c=c-32;
		if(c<0)   { c=95; } 
		if(c>95)  { c=95; }
		
		w+=(font->infos[c].advance*siz);
	}
	
	return w;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how many characters from this string can we fit in this space without overflowing
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 fenestra_ogl::font_fit_string(fogl_font *font, const char *string,f32 width)
{
int count=0;
const char *s;
f32 w=0;
GLfloat siz=(GLfloat)font->size;
		
	for(s=string;*s;s++)
	{
		char c=*s;
		
		c=c-32;
		if(c<0)   { c=95; } 
		if(c>95)  { c=95; }
		
		w+=(font->infos[c].advance*siz);
		
		if(w>width)
		{
			break;
		}
		count++;
	}
	
	return count;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// which char is under a given x position, returns a string index (0 is first,etc)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 fenestra_ogl::font_which_char(fogl_font *font, const char *string,f32 x)
{
const char *s;
s32 i;
f32 w=0;
GLfloat siz=(GLfloat)font->size;

	if(x<0) { return -1; } // out of bounds

	for(s=string,i=0;*s;s++,i++)
	{
		char c=*s;
		
		c=c-32;
		if(c<0)   { c=95; } 
		if(c>95)  { c=95; }
		
		w+=(font->infos[c].advance*siz);
		
		if(w>=x) { return i; } // found it
	}
	
	return -1; // not in range
}
