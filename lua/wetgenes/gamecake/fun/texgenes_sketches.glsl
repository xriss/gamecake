
#shader "texgenes_sketch_crosshair1"
#include "texgenes_shadertoy_head"

// (C) 2022 Kriss@XIXs.com and released under the MIT license.
// This is a texgenes shader intended to create a static texture which is then downloaded for later use.
// Options can be tweaked below or predefined in your shadertoy compatibility boilerplate


#ifndef TEXTURE_ASPECT
//SWED+{ "class":"number", "fmt":"%.2f", "max":16, "min":1, "step":1, "vec":2 }
#define TEXTURE_ASPECT 1.00 / 1.00
#endif
const float texture_aspect = float( TEXTURE_ASPECT );

#ifndef COLOR_BACK
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":4 }
#define COLOR_BACK 0.00 , 0.00 , 0.00 , 0.00
#endif
const vec4 color_back = vec4( COLOR_BACK );

#ifndef COLOR_FORE
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":4 }
#define COLOR_FORE 1.00 , 1.00 , 1.00 , 1.00
#endif
const vec4 color_fore = vec4( COLOR_FORE );


#ifndef CROSSHAIR_MODE
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define CROSSHAIR_MODE 0.00
#endif
const float crosshair_mode = float( CROSSHAIR_MODE );


#ifndef EDGE_BLEND
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define EDGE_BLEND 0.03
#endif
const float edge_blend = float( EDGE_BLEND );

#ifndef LINE_WIDTH
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define LINE_WIDTH 0.02
#endif
const float line_width = float( LINE_WIDTH );


#ifndef CROSSHAIR_RADIUS
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define CROSSHAIR_RADIUS 0.43
#endif
const float crosshair_radius = float( CROSSHAIR_RADIUS );

#ifndef CROSSHAIR_SNIP
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define CROSSHAIR_SNIP 0.30
#endif
const float crosshair_snip = float( CROSSHAIR_SNIP );

#ifndef CROSSHAIR_ROTATE
//SWED+{ "class":"number", "fmt":"%.2f", "max":1, "min":0, "step":0.01, "vec":1 }
#define CROSSHAIR_ROTATE 0.50
#endif
const float crosshair_rotate = float( CROSSHAIR_ROTATE );

#ifndef CROSSHAIR_COUNT
//SWED+{ "class":"number", "fmt":"%.2f", "max":16, "min":0, "step":1, "vec":1 }
#define CROSSHAIR_COUNT 4.00
#endif
const float crosshair_count = float( CROSSHAIR_COUNT );


const float PI = 3.14159265359;
const vec4  ZZ = vec4( 0.0 , 1.0 , -1.0 , 1.0/256.0 );
const float EP = 1.0/16384.0;


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	fragColor=color_back;
	
	// calculate centered uv in range of -0.5 to +0.5
	vec2 uv=0.5*(2.0*fragCoord-iResolution.xy)/(iResolution.x>iResolution.y*texture_aspect?iResolution.yy*texture_aspect:iResolution.xx);
	if( uv.x<-0.5 || uv.x>0.5 || uv.y<-0.5 || uv.y>0.5 ) { return; } 

if( crosshair_mode < 0.5 ) // circle
{
	float d=sqrt(dot(vec2(1.0),pow(uv,vec2(2.0))))-crosshair_radius;
	float a=atan(uv.x,uv.y)/PI;

	fragColor=mix( color_fore , color_back , smoothstep( line_width , line_width+(edge_blend) , abs(d) ) );
	fragColor=mix( color_back , fragColor , smoothstep( crosshair_snip-edge_blend*crosshair_count , crosshair_snip ,
		abs(fract(crosshair_rotate+a*crosshair_count*0.5)-0.5)*2.0 ) );
}
else // lines
{
	float d = length(uv);
	float a = fract( crosshair_count*0.5*atan(uv.y, uv.x)/PI )+crosshair_rotate;
	vec2 q=vec2( sin(a*PI) , cos(a*PI) )*d;

	fragColor=mix( color_fore , color_back , smoothstep( line_width , line_width+(edge_blend) , abs(q.x) ) );
	fragColor=mix( color_back , fragColor , smoothstep( crosshair_snip-edge_blend , crosshair_snip , abs(q.y) ) );
	fragColor=mix( fragColor , color_back , smoothstep( crosshair_radius , crosshair_radius+edge_blend , abs(q.y) ) );
}

}

#include "texgenes_shadertoy_foot"
//END OF "texgenes_sketch_crosshair1"

		