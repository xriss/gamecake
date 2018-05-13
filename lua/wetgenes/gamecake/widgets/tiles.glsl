

#shader "widget_draw_texteditor_tilemap"

uniform mat4 modelview;
uniform mat4 projection;
//uniform vec2 projection_zxy;
//uniform vec3 modelview_add;
//uniform vec4 colors[16];
uniform vec4 colors_0;
uniform vec4 colors_1;
uniform vec4 colors_2;
uniform vec4 colors_3;
uniform vec4 colors_4;
uniform vec4 colors_5;
uniform vec4 colors_6;
uniform vec4 colors_7;

//uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;
uniform sampler2D tex_map;

uniform vec4  tile_info; /* 0,1 tile size eg 8x8 and 2,3 the font texture size*/
uniform vec4  map_info; /* 0,1 just add this to texcoord and 2,3 the map texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
    gl_Position=projection*vec4(a_vertex,1.0);
	v_texcoord=a_texcoord;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;

// we can not use variable array indexs and uniform arrays seem to trigger GLES2 driver bugs
// so best to work around the limitations and do it like this.
vec4 get_color(float f)
{
	switch( int(f) )
	{
		case 0: return colors_0;
		case 1: return colors_1;
		case 2: return colors_2;
		case 3: return colors_3;
		case 4: return colors_4;
		case 5: return colors_5;
		case 6: return colors_6;
		case 7: return colors_7;
	}
	return colors_0;
}

void main(void)
{
	vec4 bg,fg; // colors
	vec4 c;
	vec3 d;
	vec2 uv=v_texcoord.xy+map_info.xy;		// base uv
	vec2 tc=fract(uv);						// tile uv
	vec2 tm=(floor(mod(uv,map_info.zw))+vec2(0.5,0.5))/map_info.zw;			// map uv
	
	d=texture2D(tex_map, tm).rgb;	
	c=texture2D(tex_tile, (((d.rg*vec2(255.0,255.0))+tc)*tile_info.xy)/tile_info.zw ).rgba;
	
	bg=get_color(       d.b*255.0  / 16.0   ); // high 4 bits are background color (usually 0)
	fg=get_color( mod( (d.b*255.0) , 16.0 ) ); //  low 4 bits are foreground color

	c*=fg; // forground tint, can adjust its alpha	
	c=((bg*(1.0-c.a))+c); // background color mixed with pre-multiplied foreground
 
	gl_FragColor=c;

}

#endif

