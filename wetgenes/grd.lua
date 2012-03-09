--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- the core module previously lived in "grd" now it is in "wetgenes.grd.core" with this wrapper code

local grd={}

local core=require("wetgenes.grd.core")


local base={}
local meta={}
meta.__index=base

setmetatable(grd,meta)

-- copypasta from GRD header
local import=[[

//
// Only have popular/usefull formats as basic types that can be used internaly
//
// data is always in ARGB order in memory but these are little endian, hence the BGRA (u32) default 
//
// these are all signed values and fit in 16bits
//

#define	GRD_FMT_NONE								0x0000

// basic formats, most internal manipulations will only work on GRD_FMT_U8_ARGB
// also you may need to convert to ARGB or RGB or INDEXED before saving and from after loading
// I'm trying to avoid diferent byte order to keep it simple, so ARGB **memory** order default

// u32[1] or u8[4] ARGB per pixel, so thats a U32-BGRA (in little endian)
#define	GRD_FMT_U8_ARGB								0x0001
	
// A is the same as in ARGB but ( RGB=RGB*A )
#define	GRD_FMT_U8_ARGB_PREMULT						0x0002

// bit swizzzzzzzzled for gles prefered order
#define	GRD_FMT_U8_RGBA								0x0003
#define	GRD_FMT_U8_RGBA_PREMULT						0x0004

// u16[1] per pixel, 1 bit alpha , 5 bits red , 5 bits green , 5 bits blue
#define	GRD_FMT_U16_ARGB_1555						0x0021	
// again premult makes more sense
#define	GRD_FMT_U16_ARGB_1555_PREMULT				0x0022

// u16[1] per pixel, 4 bit alpha , 4 bits red , 4 bits green , 4 bits blue
#define	GRD_FMT_U16_RGBA_4444						0x0023
// again premult makes more sense
#define	GRD_FMT_U16_RGBA_4444_PREMULT				0x0024

// u16[1] an output display 16bit format for gles
#define	GRD_FMT_U16_RGB_565							0x0025


// I think it makes sense to keep all floating point values as premultiplied alpha?
// a 1.0 in here is the same as a 255 in U8 format

// f16[4] per pixel
#define	GRD_FMT_F16_ARGB_PREMULT					0x0041
// f32[4] per pixel
#define	GRD_FMT_F32_ARGB_PREMULT					0x0062
// f64[4] per pixel
#define	GRD_FMT_F64_ARGB_PREMULT					0x0083

// u8[3]  per pixel, probably just normal palette information
#define	GRD_FMT_U8_RGB								0x00a1

// u8[1]  per pixel, forced U8 Indexed input
#define	GRD_FMT_U8_INDEXED							0x00c1

// u8[1]  per pixel, forced U8 gray scale (treat as indexed)
#define	GRD_FMT_U8_LUMINANCE						0x00e1


// more formats, not to be used when mucking about with data
// these are hints for textures rather than specific formats and don't guarantee any number of bits
// in fact the texture may even use a simple lossy compressed format if enabled
// basically it is none of your concern, if you intend to do anything with the dat convert it to one of the
// basic formats

// just RGB , probably u32 or u16(565)
#define	GRD_FMT_HINT_NO_ALPHA						0x0101

// and RGB  , probably u32 or u16(1555)
#define	GRD_FMT_HINT_ALPHA_1BIT						0x0102

// and RGB  , probably u32 or u16(4444)
#define	GRD_FMT_HINT_ALPHA							0x0103

// no RGB   , probably u8
#define	GRD_FMT_HINT_ONLY_ALPHA						0x0104

// we want to save or load as a png									
#define	GRD_FMT_HINT_PNG							0x0105

// we want to save or load as a jpg									
#define	GRD_FMT_HINT_JPG							0x0106
	
// maximum GRD_FMT value		
#define	GRD_FMT_MAX									0x0107

]]
-- parse the above string for constants, makes updates as easy as a cutnpaste from original source code

grd.defs={}

for l in import:gmatch("([^\n]*)") do
	local define,value
	local state="start"
	for w in l:gmatch("([^%s]+)") do
		if state=="start" then
			if w=="#define" then
				state="define"
			else
				break
			end
		elseif state=="define" then
			define=w
			state="value"
		elseif state=="value" then
			value=w
				if define:sub(1,4)=="GRD_" then -- sanity check
					define=define:sub(5)
					
					if value:sub(1,4)=="GRD_" then -- allow lookback
						value=grd.defs[value:sub(5)]
					end
					
					grd.defs[define]=tonumber(value)

					if define:sub(1,4)=="FMT_" then -- allow even shorter names
						grd.defs[define:sub(5)]=tonumber(value)
					end
					
				end
			break
		else
			break
		end
	end
end
import=nil -- free it just because

grd.nums={}

for i,v in pairs(grd.defs) do -- copy vals into base for shorthand grd.FALSE style use
	base[i]=v
	grd.nums[v]=i -- and reverse lookup
end

function grd.numtostring(num)
	return grd.nums[num]
end
function grd.stringtonum(str)
	return grd.defs[str] 
end

-- many options
-- grd.create(g) -- duplicate
-- grd.create(fmt,w,h,d) -- a new blank image of given dimensions
-- grd.create(filename,opts) -- load image
-- grd.create() -- a blank image of 0 dimensions
grd.create=function(...)
	local args={...}
	local g={}
	setmetatable(g,meta)
	
	if type(args[1]) == "table" then -- duplicate
	
--		local g2=args[1]
--		g[0]=core.create(g2[0])
	
	elseif type(args[2]) == "number" then -- a new blank image of given dimensions
	
		local fmt=args[1]
		local w,h,d=args[2],args[3],args[4]
		if type(fmt) == "string" then
			fmt=grd.stringtonum(args[1])
		end
		g[0]=core.create(fmt,w,h,d)

	elseif type(args[1]) == "string" then -- load image
	
		local filename=args[1]
		local fmt=args[2]
		g[0]=core.create()
		g:load_file(filename,fmt)
	
	elseif type(args[1]) == "userdata" then -- just wrap the table

		g[0]=args[1]

	else -- a blank image of 0 dimensions

		g[0]=core.create()
	
	end
	
	
	core.info(g[0],g)
	return g
end

base.destroy=function(g)
	return core.destroy(g[0])
end


base.reset=function(g)
	local r=core.reset(g[0])
	core.info(g[0],g)
	return r and g
end

base.load_file=function(g,filename,fmt)
	return base.load(g,{filename=filename,fmt=fmt})
end

base.load_data=function(g,data,fmt)
	return base.load(g,{data=data,fmt=fmt})
end

base.load=function(g,opts)
	if opts.fmt=="jpg" then
		opts.fmt=grd.FMT_HINT_JPG
	end
	if opts.fmt=="png" then
		opts.fmt=grd.FMT_HINT_PNG
	end
	local r=core.load(g[0],opts)
	core.info(g[0],g)
	return r and g
end

base.save=function(g,filename,opts)
	local r=core.save(g[0],filename,opts)
	core.info(g[0],g)
	return r and g
end

base.duplicate=function(g)
	local r=core.duplicate(g[0])
	return grd.create(r)
end

base.duplicate_convert=function(g,fmt)
	if type(fmt) == "string" then
		fmt=grd.stringtonum(fmt)
	end
	local r=core.duplicate_convert(g[0],fmt)
	if r==g[0] then r=core.duplicate(g[0]) end -- force duplication
	return grd.create(r)
end

base.convert=function(g,fmt)
	if type(fmt) == "string" then
		fmt=grd.stringtonum(fmt)
	end
	local r=core.convert(g[0],fmt)
	core.info(g[0],g)
	return r and g
end

base.quant=function(g,num)
	local r=core.quant(g[0],num)
	core.info(g[0],g)
	return r and g
end

base.pixels=function(g,...)
	local r=core.pixels(g[0],...)
	core.info(g[0],g)
	return r
end

base.palette=function(g,...)
	local r=core.palette(g[0],...)
	core.info(g[0],g)
	return r
end

base.scale=function(g,...)
	local r=core.scale(g[0],...)
	core.info(g[0],g)
	return r and g
end

base.flipy=function(g,...)
	local r=core.flipy(g[0],...)
	core.info(g[0],g)
	return r and g
end

base.shrink=function(g,...)
	local r=core.shrink(g[0],...)
	core.info(g[0],g)
	return r and g
end

base.blit=function(ga,gb,x,y,cx,cy,cw,ch)

	if cx then -- autoclip
		if cx<0 then cw=cw+cx cx=0 end
		if cy<0 then ch=ch+cy cy=0 end
		if (cx+cw)>gb.width  then cw=gb.width -cx end
		if (cy+ch)>gb.height then ch=gb.height-cy end
	else -- auto build
		cx=0
		cy=0
		cw=gb.width
		ch=gb.height
	end
	
	if x<0 then cx=cx-x cw=cw+x x=0 end
	if y<0 then cy=cy-y ch=ch+y y=0 end	
	if (x+cw)>ga.width  then cw=ga.width -x end
	if (y+ch)>ga.height then ch=ga.height-y end

	if cw<=0 or ch<=0 then return true end -- nothing to draw

	local r=core.blit(ga[0],gb[0],x,y,cx,cy,cw,ch)
	
	return r
end

return grd
