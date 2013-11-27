--
-- (C) 2013 Kriss@XIXs.com
--
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
// We shall stop fighting the force of open GL and accept RGBA as the damn color order
//
// Old code will break after this change (its all ARGB order) so keep stuff upto date mkay :)
// all palettes have been switched to this new order as well
//

#define	GRD_FMT_NONE								0x0000

// A is the same as in ARGB but ( RGB=RGB*A )
// Notice that this is more of a flag ontop of other formats
#define	GRD_FMT_PREMULT								0x0100

// basic formats, most internal manipulations will only work on GRD_FMT_U8_RGBA
// in fact many will convert to this as an intermediate step.
// also you may need to convert to RGBA or RGB or INDEXED before saving and from after loading
// I'm trying to avoid diferent byte order to keep it simple, so RGBA **memory** order default

// bit swizzzzzzzzled for gls prefered (and only suported) order
// u32[1] or u8[4] RGBA per pixel, so thats a U32-ABGR (in little endian)
#define	GRD_FMT_U8_RGBA								0x0001
#define	GRD_FMT_U8_RGBA_PREMULT						0x0101

// u32[1] or u8[4] ARGB per pixel, so thats a U32-BGRA (in little endian)
// This format is not used anywhere internally anymore.
#define	GRD_FMT_U8_ARGB								0x0002
#define	GRD_FMT_U8_ARGB_PREMULT						0x0102


// u8[3]  per pixel RGB, no alpha
#define	GRD_FMT_U8_RGB								0x0011


// u8[1]  per pixel, palette indexed, palette contains alpha (png suports this)
#define	GRD_FMT_U8_INDEXED							0x0021
#define	GRD_FMT_U8_INDEXED_PREMULT					0x0121

// u8[1]  per pixel, black to white, gray scale
#define	GRD_FMT_U8_LUMINANCE						0x0022

// u8[1]  per pixel, alpha chanel only, assume white for rgb values
#define	GRD_FMT_U8_ALPHA							0x0023


// u16[1] per pixel
#define	GRD_FMT_U16_RGBA_5551						0x0031	
#define	GRD_FMT_U16_RGBA_5551_PREMULT				0x0131

// u16[1] per pixel
#define	GRD_FMT_U16_RGBA_4444						0x0032
#define	GRD_FMT_U16_RGBA_4444_PREMULT				0x0132

// u16[1] an output display 16bit format for gles
#define	GRD_FMT_U16_RGBA_5650						0x0033
#define	GRD_FMT_U16_RGBA_5650_PREMULT				0x0133




// I think it makes sense to keep all floating point values as premultiplied alpha?
// a 1.0 in here is the same as a 255 in U8 format
// none of these are used, so lets comment them all out for now
// f16[4] per pixel
//#define	GRD_FMT_F16_RGBA_PREMULT					0x0141
// f32[4] per pixel
//#define	GRD_FMT_F32_RGBA_PREMULT					0x0151
// f64[4] per pixel
//#define	GRD_FMT_F64_RGBA_PREMULT					0x0161


// the following are hints for internal code

// just RGB , probably u32 or u16(565)
#define	GRD_FMT_HINT_NO_ALPHA						0x0201

// and RGB  , probably u32 or u16(1555)
#define	GRD_FMT_HINT_ALPHA_1BIT						0x0202

// and RGB  , probably u32 or u16(4444)
#define	GRD_FMT_HINT_ALPHA							0x0203

// no RGB   , probably u8
#define	GRD_FMT_HINT_ONLY_ALPHA						0x0204



// we want to save or load as a png									
#define	GRD_FMT_HINT_PNG							0x0401

// we want to save or load as a jpg									
#define	GRD_FMT_HINT_JPG							0x0402

// we want to save or load as a jpg									
#define	GRD_FMT_HINT_GIF							0x0403

// Paint a copy skiping transparent color
#define	GRD_PAINT_MODE_TRANS						0x0801
// Paint a single color
#define	GRD_PAINT_MODE_COLOR						0x0802
// Paint a copy
#define	GRD_PAINT_MODE_COPY							0x0803
// Paint using XOR
#define	GRD_PAINT_MODE_XOR							0x0804

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

		local p=args[1]
		g[0]=core.create(p.format,p.width,p.height,p.depth)
		core.copy_data(g[0],p[0])

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
	elseif opts.fmt=="png" then
		opts.fmt=grd.FMT_HINT_PNG
	elseif opts.fmt=="gif" then
		opts.fmt=grd.FMT_HINT_GIF
	end
	local r=core.load(g[0],opts)
	core.info(g[0],g)
	return r and g
end

base.save=function(g,opts)

	if type(opts)=="string" then
		opts={filename=opts}
	end

	if type(opts.fmt)=="string" then -- convert from string
		if opts.fmt=="jpg" then
			opts.fmt=grd.FMT_HINT_JPG
		elseif opts.fmt=="png" then
			opts.fmt=grd.FMT_HINT_PNG
		elseif opts.fmt=="gif" then
			opts.fmt=grd.FMT_HINT_GIF
		end
	end

	if not opts.fmt then -- guess
	
		if opts.filename:lower():find("%.gif$") then
			opts.fmt=grd.FMT_HINT_GIF
		elseif opts.filename:lower():find("%.jpg$") then
			opts.fmt=grd.FMT_HINT_JPG
		elseif opts.filename:lower():find("%.jpeg$") then
			opts.fmt=grd.FMT_HINT_JPG
		else
			opts.fmt=grd.FMT_HINT_PNG
		end
	
	end

	local r=core.save(g[0],opts.filename,opts.fmt)
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
	local r,e=core.convert(g[0],fmt)
	core.info(g[0],g)
	return (r and g),e
end

base.clear=function(g,color)
	local r=core.clear(g[0],color)
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
	if not g.cmap then return nil end
	local r=core.palette(g[0],...)
	core.info(g[0],g)
	return r
end

base.resize=function(g,...)
	local r=core.resize(g[0],...)
	core.info(g[0],g)
	return r and g
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

base.flipx=function(g,...)
	local r=core.flipx(g[0],...)
	core.info(g[0],g)
	return r and g
end

base.shrink=function(g,...)
	local r=core.shrink(g[0],...)
	core.info(g[0],g)
	return r and g
end

base.info=function(g,...)
	core.info(g[0],g)
	return g
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

	return core.blit(ga[0],gb[0],x,y,cx,cy,cw,ch)
end

base.paint=function(ga,gb,x,y,cx,cy,cw,ch,mode,trans,color)

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

	return core.paint(ga[0],gb[0],x,y,cx,cy,cw,ch,mode,trans,color)
end

base.copy_data=function(ga,gb)
	return core.copy_data(ga[0],gb[0])
end
base.copy_data_layer=function(ga,gb,za,zb)
	return core.copy_data_layer(ga[0],gb[0],za,zb)
end

return grd
