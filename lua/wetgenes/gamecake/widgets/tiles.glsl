

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

vec4 get_color(float f)
{
	if(f< 1.0) { return colors_0; } else
	if(f< 2.0) { return colors_1; } else
	if(f< 3.0) { return colors_2; } else
	if(f< 4.0) { return colors_3; } else
	if(f< 5.0) { return colors_4; } else
	if(f< 6.0) { return colors_5; } else
	if(f< 7.0) { return colors_6; } else
	if(f< 8.0) { return colors_7; } else
//	if(f< 8.0) { return colors[ 8]; } else
//	if(f<10.0) { return colors[ 9]; } else
//	if(f<10.0) { return colors[10]; } else
//	if(f<12.0) { return colors[11]; } else
//	if(f<12.0) { return colors[12]; } else
//	if(f<14.0) { return colors[13]; } else
//	if(f<14.0) { return colors[14]; }
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
	
	bg=get_color(       d.b*255.0  / 16.0   ); // high 4 bits are background color
	fg=get_color( mod( (d.b*255.0) , 16.0 ) ); //  low 4 bits are foreground color

	c*=fg; // forground tint, can adjust its alpha	
	c=((bg*(1.0-c.a))+c); // background color mixed with pre-multiplied foreground
 
	gl_FragColor=c;

}

#endif

