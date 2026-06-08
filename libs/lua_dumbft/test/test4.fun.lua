
local usb = require("moonusb")
local dumbft=require("dumbft")

-- expect this many s16 audio samples per second
local sample_rate=48000
local buffer_size=math.floor(sample_rate/16)

-- roughly, 8hz is midinote 0, 16hz=12, 32hz=24, 64hz=36, 128hz=48, etc...

local midinote_to_freq=function(m)
	return (2^((m-69)/12))*440
end
local midinote_to_wavelen=function(m)
	return (sample_rate/midinote_to_freq(m))
end

local math_log_2=math.log(2)

local freq_to_midinote=function(f)
	return 69+((math.log(f/440)/math_log_2)*12)
end

local wavelen_to_midinote=function(w)
	return freq_to_midinote(sample_rate/w)
end

-- idx of max bucket ( bottom bucket is 0 not 1)
local max_bucket=256

-- we are zooming in here on the midinote 0-127 range in a non linear array
-- to get this quality of data from a standard FFT would need at least 16k of 
-- buckets instead of the 256 we have here so 64x the number of buckets
-- most of which we would ignore and that huge bucket size means a "laggy"
-- sample rate or running the "fast" code multiple times on the same samples.
-- So an FFT is only "faster" at generating data we do not want or need.
-- Also remember we are looking here at visualization so only need to be
-- concerned with a one way transform, no rebuilding the original data.


--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
local fats=require("wetgenes.fats")

oven.opts.fun="" -- back to menu on reset

sysopts={
	mode="fun64", -- select a characters+sprites on a 256x128 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly at 60hz to update+draw
	hx=256,hy=256, -- set custom size
}

hardware,main=system.configurator(sysopts)

sendprint=function(s)
	if not printhandle then return end
	
	local mem = usb.malloc(nil, #s)
	mem:write(0,nil,s)
	local bytes_sent = printhandle:bulk_transfer(0x01, mem:ptr(), #s, 0)
	mem:free()
end

setup=function()
	-- setup audio capture
	oven.cake.sounds.start_capture()
	-- setup custom glsl shader
	system.components.copper.shader_name="fun_DFT"
	
	local buckets={}
	for idx=1,max_bucket do
		local midinote=36+((idx-1)*64/256) -- so 36 to 100
		local wavnum=math.ceil(128/midinote_to_wavelen(midinote)) -- must be at least 1
		local problen=math.floor(midinote_to_wavelen(midinote)*wavnum) -- multiple waves as probes
		buckets[idx*2-1]=problen -- number of samples in probe
		buckets[idx*2]=wavnum  -- number of waves in probe

--print(idx,midinote,problen,wavnum,problen/wavnum)

	end
	dft=dumbft.create(buckets)

--	usb.trace_objects(true) -- trace creation/deletion of objects
	local ctx = usb.init()
	local vendor_id, product_id = 0x0483,  0x2023
	local device, devhandle = ctx:open_device(vendor_id, product_id)
	printhandle=devhandle
	print("PRINTER",printhandle)

end

local wgrd=require("wetgenes.grd")
local grd_dft=nil
local grd_idx=0

local aline={}
	for i=1,256 do aline[i]=0 end
local bline={}
	for i=1,256 do bline[i]=0 end
local mline={}
	for i=1,256 do mline[i]=0 end
local oline={}
	for i=1,256 do oline[i]=0 end
local amerge=0
local amax=60
local add_line=function(line)

	local t={} -- blur array left/right
	t[1]=(line[1]+line[2])/3
	t[256]=(line[255]+line[256])/3
	for i=2,255 do
		t[i]=(line[i-1]+line[i]+line[i+1])/3
	end
	bline=t

	local done=false
	amerge=amerge+1
	for i=1,256 do
		aline[i]=aline[i]+bline[i]
	end
	if amerge>=amax then
		for i=1,256 do
			mline[i]=aline[i]/amerge
		end
		done=true
	end

	local copper=system.components.copper

	if not grd_dft then
		grd_dft=wgrd.create(wgrd.FMT_U8_LUMINANCE,256,256,1)
	end

	if done then
		grd_dft:pixels(0,grd_idx,256,1,mline) -- slow data

		for i=0,255 do aline[i]=0 end
		amerge=0

		grd_idx=(grd_idx+1)%256

		if grd_idx==0 or grd_idx==128 then
			local copper=system.components.copper
			local screen=system.components.screen
			local layer=screen.layers[copper.layer]
			local fbo=layer.fbo
			local grd=screen.fbo:download()
--			print("save",grd.width,grd.height)
--			local png=grd:save({})
--			local fp=assert( io.open( "/home/kriss/wav"..os.time()..".png","wb") )
--			fp:write(png)
--			fp:close()
			
phead=[[

SIZE 48 mm,24 mm
CLS
GAP 0,0
BLINE 0,0
OFFSET 0
REFERENCE 0,0
DIRECTION 0,0
SPEED 1
BITMAP 0,0,48,192,0,]]

pfoot=[[

PRINT 1
BACKUP 72

]]

local ga=grd:clip(0,0,0,256,192,1)
local gb=wgrd.create(wgrd.FMT_U8_INDEXED,384,192,1)
gb:palette(0,2,{0,0,0,255,1,1,1,255})
ga:scale(384,192,1)
ga:remap(gb,2,0)

--gb:save("test.png")

print(ga.width,ga.height)
local d=gb:pixels(0,0,384,192,"")

local b=0
local a=0
local t={}
for i=1,#d do
	local c=d:sub(i,i)=="\0" and 1 or 0
	a=a*2+c
	b=b+1
	if b==8 then
		b=0
		t[#t+1]=string.char(a)
		a=0
	end
end
local d=table.concat(t)

sendprint(phead..d..pfoot)

--			sendprint("SELFTEST\n")
		end
	end

	copper.shader_uniforms.tex_info={ grd_idx , amerge/amax , 0 , 0 } -- live line

	for i=1,256 do -- decay 67% old 33% new
		oline[i]=(oline[i]+oline[i]+bline[i])/3
	end
	grd_dft:pixels(0,grd_idx,256,1,oline) -- live data

	-- uploaded grd will autobind to "copper_tex" in shader
	copper.upload_grd(grd_dft)
end

local buff=""
update=function()

    if setup then setup() ; setup=nil end
    
	local nbuff,len=oven.cake.sounds.get_capture()
	if nbuff then
		buff=buff..nbuff
	end

	local chunksize=400*2
	while #buff>=chunksize do -- chunk process
		dft:push(buff:sub(1,chunksize))
		buff=buff:sub(chunksize+1,-1)

		local bs=fats.doubles_to_table( dft:pull() )
		local line={}
		for idx=1,max_bucket do
	--		local m=wavelen_to_midinote(dft.probelens[idx*2-1]/dft.probelens[idx*2])
			local n=bs[idx]
			-- scale to 0-255 range and tweak *4 since mic is usually low...
			n=math.ceil(n*2*256)
			line[idx]=n
		end
		add_line(line)
	end
end


--[=[
#shader "fun_DFT"

#version 100
#version 120
#ifdef VERSION_ES
precision mediump float;
#endif

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * vec4(a_vertex, 1.0);
	v_texcoord=a_texcoord;
	v_color=color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

// 256x256 8bit texture, each line is the 256 buckets we calculated
// tex_info[0] tells us the current line 0-255 
// tex_info[1] is 0/16 to 15/16 for fractional animation to the next line
// texture is scan filled, so we write a new smoothed line every 16 frames
// but always update the current line each frame with latest data
uniform vec4 tex_info;
uniform sampler2D copper_tex;

varying vec2  v_texcoord;
varying vec4  v_color;


void draw_wave(inout vec4 c , float y , float vy , float fade)
{
	y=(256.0-y)/128.0;
	if( ( y<0.0 ) || ( y>1.0 ) ) { return; } // out of range

	float n1=texture2D(copper_tex, vec2( (v_texcoord.x-2.0)/256.0 , vy ) ).r;
	float n2=texture2D(copper_tex, vec2( (v_texcoord.x-1.0)/256.0 , vy ) ).r;
	float n3=texture2D(copper_tex, vec2( (v_texcoord.x    )/256.0 , vy ) ).r;
	float n4=texture2D(copper_tex, vec2( (v_texcoord.x+1.0)/256.0 , vy ) ).r;
	float n5=texture2D(copper_tex, vec2( (v_texcoord.x+2.0)/256.0 , vy ) ).r;
	
	float nn1=max(max(n1,n2),n3)*fade;
	float nn2=max(max(n2,n3),n4)*fade;
	float nn3=max(max(n3,n4),n5)*fade;

	// white edge
	if( y-(2.0/256.0) <= nn2 )
	{
		c=vec4( 1.0 , 1.0 , 1.0 , 0.0 );
	}

	// black fill
	if( ( y <= nn1 ) && ( y <= nn2 ) && ( y <= nn3 ) )
	{
		c=vec4( 0.0 , 0.0 , 0.0 , 0.0 );
	}
}

void main(void)
{
	vec4 c=vec4(0.0);
	float fade=min( ( 128.0-abs(v_texcoord.x-128.0) ) , 128.0 )/128.0;


	float v=texture2D(copper_tex, vec2( v_texcoord.x/256.0 , fract( ( tex_info[0]+v_texcoord.y) /256.0 ) ) ).r;

//	c=vec4( v , v , v , 0.0 );

	float f;

#define STEP 8.0
#define DRAW_WAVE(DW) \
		draw_wave( c , v_texcoord.y+(256.0-(DW*STEP))+mod(tex_info[0],STEP) ,\
			fract( mod( tex_info[0]+256.0-(256.0-(DW*STEP))-mod(tex_info[0],STEP) , 256.0 ) /256.0 ) ,\
				pow(fade,0.5) );

DRAW_WAVE( 0.0)
DRAW_WAVE( 1.0)
DRAW_WAVE( 2.0)
DRAW_WAVE( 3.0)
DRAW_WAVE( 4.0)
DRAW_WAVE( 5.0)
DRAW_WAVE( 6.0)
DRAW_WAVE( 7.0)
DRAW_WAVE( 8.0)
DRAW_WAVE( 9.0)
DRAW_WAVE(10.0)
DRAW_WAVE(11.0)
DRAW_WAVE(12.0)
DRAW_WAVE(13.0)
DRAW_WAVE(14.0)
DRAW_WAVE(15.0)
DRAW_WAVE(16.0)
DRAW_WAVE(17.0)
DRAW_WAVE(18.0)
DRAW_WAVE(19.0)
DRAW_WAVE(20.0)
DRAW_WAVE(21.0)
DRAW_WAVE(22.0)
DRAW_WAVE(23.0)
DRAW_WAVE(24.0)
DRAW_WAVE(25.0)
DRAW_WAVE(26.0)
DRAW_WAVE(27.0)
DRAW_WAVE(28.0)
DRAW_WAVE(29.0)
DRAW_WAVE(30.0)
DRAW_WAVE(31.0)

	// last wave we want to slowly ease in as it scrolls up from the bottom
	f=0.0;
	draw_wave( c , v_texcoord.y+f+mod(tex_info[0],STEP)  ,
		fract( mod( tex_info[0]+256.0-f-mod(tex_info[0],STEP) , 256.0 ) /256.0 ) ,
			pow(fade,0.5)*((mod(tex_info[0],STEP)/STEP)+(tex_info[1]/STEP)) );

	// live data bottom wave
	draw_wave( c , mod( v_texcoord.y , 256.0 ) , fract( (tex_info[0])/256.0 ) , pow(fade,0.5) );

	
	gl_FragColor=pow( c*pow(fade,2.0) , vec4(1.0/2.2) );
}

#endif


//]=]
