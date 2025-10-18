

#shader "widget_draw_texteditor_tilemap"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
//uniform vec2 projection_zxy;
//uniform vec3 modelview_add;

//uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;
uniform sampler2D tex_map;
uniform sampler2D tex_cmap;

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

void main(void)
{
	vec4 bg,fg; // colors
	vec4 c;
	vec4 d;
	vec2 uv=v_texcoord.xy+map_info.xy;		// base uv
	vec2 tc=fract(uv);						// tile uv
	vec2 tm=(floor(mod(uv,map_info.zw))+vec2(0.5,0.5))/map_info.zw;			// map uv
	
	d=texture2D(tex_map, tm).rgba;	
	vec2 xy=((floor(vec2( 0.5, 0.5)+(d.rg*vec2(255.0,255.0)))+tc)*tile_info.xy);

// this will blur slightly
//	c=(   texture2D(tex_tile, (xy+vec2(-0.5,-0.5))/tile_info.zw ).rgba
//		+ texture2D(tex_tile, (xy+vec2( 0.5,-0.5))/tile_info.zw ).rgba
//		+ texture2D(tex_tile, (xy+vec2(-0.5, 0.5))/tile_info.zw ).rgba
//		+ texture2D(tex_tile, (xy+vec2( 0.5, 0.5))/tile_info.zw ).rgba )/4.0;

// better to do this and then render to an fbo and scale that
	c=(   texture2D(tex_tile, (xy                )/tile_info.zw ).rgba );
	
	fg=texture2D(tex_cmap, vec2( d.b,0.5) ).rgba;
	bg=texture2D(tex_cmap, vec2( d.a,0.5) ).rgba;

	c*=fg; // forground tint, can adjust its alpha	
	c=((bg*(1.0-c.a))+c); // background color mixed with pre-multiplied foreground
 
	gl_FragColor=c;

}

#endif

