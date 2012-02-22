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
// these are all signed values 
//

#define	GRD_FMT_NONE								0x0000

// basic formats, most internal manipulations will only work on GRD_FMT_U8_ARGB
// also you may need to convert to ARGB or RGB or INDEXED before saving and from after loading
// I'm trying to avoid diferent byte order to keep it simple, so ARGB **memory** order only

// u8[4] ARGB per pixel, so thats a U32-BGRA (thinking little endian)
#define	GRD_FMT_U8_ARGB								0x0001
	
// A is the same as in ARGB but ( RGB=RGB*A )
#define	GRD_FMT_U8_ARGB_PREMULT						0x0002

// u16[1] per pixel, 1 bit alpha , 5 bits red , 5 bits green , 5 bits blue
#define	GRD_FMT_U16_ARGB_1555						0x0011
	
// again premult makes more sense
#define	GRD_FMT_U16_ARGB_1555_PREMULT				0x0012

// I think it makes sense to keep all floating point values as premultiplied alpha?
// a 1.0 in here is the same as a 255 in U8 format

// f16[4] per pixel
#define	GRD_FMT_F16_ARGB_PREMULT					0x0021
// f32[4] per pixel
#define	GRD_FMT_F32_ARGB_PREMULT					0x0022
// f64[4] per pixel
#define	GRD_FMT_F64_ARGB_PREMULT					0x0023

// u8[3]  per pixel, probably just normal palette information
#define	GRD_FMT_U8_RGB								0x0031

// u8[1]  per pixel, forced U8 Indexed input
#define	GRD_FMT_U8_INDEXED							0x0041

// u8[1]  per pixel, forced U8 gray scale (treat as indexed)
#define	GRD_FMT_U8_LUMINANCE						0x0051


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
				end
			break
		else
			break
		end
	end
end
import=nil -- free it just because

for i,v in pairs(grd.defs) do -- copy vals into base for shorthand grd.FALSE use
	base[i]=v
end

function grd.numtostring(num)
	return grd.defs[num]
end
function grd.stringtonum(str)
	return grd.defs[str] or grd.defs["FMT_"..str]
end

-- many options
-- grd.create(g) -- duplicate
-- grd.create(fmt,w,h,d) -- a new blank image of given dimensions
-- grd.create(filename,opts) -- load image
-- grd.create() -- a blank image of 0 dimensions
grd.create=function(...)
	local args={...}
	local g
	
	if type(args[1]) == "table" then -- duplicate
	
		local g2=args[1]
		g=core.create(g2)
	
	elseif type(args[2]) == "number" then -- a new blank image of given dimensions
	
		local fmt=args[1]
		local w,h,d=args[2],args[3],args[4]
		if type(fmt) == "string" then
			fmt=grd.stringtonum(args[1])
		end
		g=core.create(fmt,w,h,d)

	elseif type(args[1]) == "string" then -- load image
	
		local filename=args[1]
		local opts=args[2]
		g=core.create(filename,opts)
	
	else -- a blank image of 0 dimensions

		g=core.create()
	
	end

	if g then
		setmetatable(g,meta)
	end
	
	return g
end

base.destroy=function(g)
	return core.destroy(g)
end


base.reset=function(g)
	return core.reset(g)
end

base.load=function(g,filename,opts)
	return core.load(g,filename,opts)
end

base.save=function(g,filename,opts)
	return core.save(g,filename,opts)
end

base.convert=function(g,fmt)
	if type(fmt) == "string" then
		fmt=grd.stringtonum(fmt)
	end
	return core.convert(g,fmt)
end

base.quant=function(g,num)
	return core.quant(g,num)
end

base.pixels=function(...)
	return core.pixels(...)
end

base.palette=function(...)
	return core.palette(...)
end

base.scale=function(...)
	return core.scale(...)
end

base.flipy=function(...)
	return core.flipy(...)
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

	return core.blit(ga,gb,x,y,cx,cy,cw,ch)
end

return grd
