--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.grd

	local wgrd=require("wetgenes.grd")

We use wgrd as the local name of this library.

Handle bitmap creation, loading, saving and blitting. The bitmap and 
the colormap for indexed bitmaps are represented by the same data 
structure which describes a continuous chunk of memory with optional 
ability to select an area of a larger chunk using simple byte spans.

Swanky Paint uses this to manage its bitmaps and its also used to 
convert art into data at build time for use in the GameCake engine. The 
PageCake engine uses this for image management, creating live thumbnails 
and so on.

We load and save jpeg, png and gif. The png lib contains extensions for 
apng which allows animation chunks. Animations are contained in the Z 
(depth) dimension of the grd.

The following are possible format options that we support. Most of them 
are OpenGL friendly.

	wgrd.FMT_U8_RGBA
	
32 bits per pixel with a byte order of red, green, blue, alpha and a 
little endian U32 of ABGR. We prefer this byte order because OpenGL.

	wgrd.FMT_U8_ARGB

32 bits per pixel with a byte order of alpha, red, green, blue and a 
little endian U32 of BGRA.

	wgrd.FMT_U8_RGB

24 bits per pixel with a byte order of red, green, blue.

	wgrd.FMT_U8_INDEXED
	
8 bits per pixel which indexes a wgrd.FMT_U8_RGBA palette.

	wgrd.FMT_U8_LUMINANCE

8 bits per pixel, grey scale only.

	wgrd.FMT_U8_ALPHA

8 bits per pixel, alpha only.

	wgrd.FMT_U16_RGBA_5551

16 bits per pixel with 5 bits each of red,green,blue and 1 bit of alpha 
packed into a single U16.

	wgrd.FMT_U16_RGBA_4444

16 bits per pixel with 4 bits each of red,green,blue,alpha packed into 
a single U16.

	wgrd.FMT_U16_RGBA_5650

16 bits per pixel with 4 bits of red, 5 bits of green and 4 bits of 
blue packed into a single U16.


	wgrd.FMT_U8_RGBA_PREMULT
	wgrd.FMT_U8_ARGB_PREMULT
	wgrd.FMT_U8_INDEXED_PREMULT
	wgrd.FMT_U16_RGBA_5551_PREMULT
	wgrd.FMT_U16_RGBA_4444_PREMULT
	wgrd.FMT_U16_RGBA_5650_PREMULT

Are all just pre-multiplied alpha versions of the base format described 
above.

Check this link out for why pre-multiplied alpha is a good idea for any 
image that will be used as a texture 
http://blogs.msdn.com/b/shawnhar/archive/2009/11/06/premultiplied-alpha.aspx


	wgrd.GRD_FMT_HINT_PNG
	wgrd.GRD_FMT_HINT_JPG
	wgrd.GRD_FMT_HINT_GIF

These are used to choose a specific file format when loading or saving.

]]

-- the core module previously lived in "grd" now it is in "wetgenes.grd.core" with this wrapper code

local grd={}

local wjson=require("wetgenes.json")
local wstr=require("wetgenes.string")

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

// u8[2]  luminance and alpha
#define	GRD_FMT_U8_LUMINANCE_ALPHA					0x0034
#define	GRD_FMT_U8_LUMINANCE_ALPHA_PREMULT			0x0134



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
// Paint using ALPHA from palette (skip if < 0x80)
#define	GRD_PAINT_MODE_ALPHA						0x0805

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

--[[#lua.wetgenes.grd.create

	ga=wgrd.create(gb)

Duplicate the grd.
	
	ga=wgrd.create(format,width,height,depth)

Create a grd in the given format with the given width height and depth. 

	ga=wgrd.create(filename,opts)

Load an image from the file system.

	ga=wgrd.create()
	
Create a blank grd of 0 dimensions.

This is usually the only wgrd function you would need to call as once you 
have a grd you can use the : calling convention to modify it. The other 
functions will be shown as examples using the : calling convention.

	wgrd.create():load(opts)

For instance could be used if you wish to perform a more esoteric load 
than from the file system.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns a grd object.

]]
-- many options
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
		local w,h,d=args[2] or 1,args[3] or 1,args[4] or 1
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
base.create=grd.create

--[[#lua.wetgenes.grd.destroy

	g:destroy()

Free the grd and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.
	
]]
base.destroy=function(g)
	return core.destroy(g[0])
end


--[[#lua.wetgenes.grd.reset

	g:reset()

Reset the grd which will now be a blank image of 0 dimensions.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.reset=function(g)
	local r=core.reset(g[0])
	core.info(g[0],g)
	return (r and g),g.err
end

--[[#lua.wetgenes.grd.load_file

	g:load_file(filename,format)

Load an image from the file system.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.
 
]]
base.load_file=function(g,filename,fmt)
	return base.load(g,{filename=filename,fmt=fmt})
end

--[[#lua.wetgenes.grd.load_data

	g:load_data(datastring,format)

Load an image from memory.
 
Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.load_data=function(g,data,fmt)
	return base.load(g,{data=data,fmt=fmt})
end

--[[#lua.wetgenes.grd.load

	g:load(opts)

Load an image from memory or file system depending on settings in opts.

	opts.fmt

Lets you choose an image format, the strings "jpg","png" or "gif" will 
be converted to the appropriate wgrd.FMT_HINT_* value.

	opts.data

Flags this as a load from memory and provides the data string to load 
from.

	opts.filename

Flags this as a load the file system and provides the file name to 
open.
 
Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.load=function(g,opts)
	if type(opts)=="string" then
		opts={filename=opts}
	end
	if opts.fmt=="jpg" then
		opts.fmt=grd.FMT_HINT_JPG
	elseif opts.fmt=="png" then
		opts.fmt=grd.FMT_HINT_PNG
	elseif opts.fmt=="gif" then
		opts.fmt=grd.FMT_HINT_GIF
	end
	local r,j=core.load(g[0],opts)
	core.info(g[0],g)
	if not r then g.err=j end -- special error return
	if r and j then
--		print(wstr.dump(j))
		g.json=j.JSON and wjson.decode(j.JSON) -- may have some json as well
		g.undo=j.UNDO -- maybe some undo state
	end
	return (r and g),g.err
end

--[[#lua.wetgenes.grd.save

	g:save(opts)

Save an image to memory or filesytem depending on settings in opts.

	opts.fmt

Lets you choose an image format, the strings "jpg","png" or "gif" will 
be converted to the appropriate wgrd.FMT_HINT_* value.

We will guess opts.fmt from the file name extension if it is not 
provided and a file name is.

	opts.filename

Flags this as a load the file system and provides the file name to 
write to. If no filename is given then we will be saving into memory 
and be returning that data string as the first return value.
 
Returns nil,error if something goes wrong so can be used with assert.

If no file name is given then we *return* the data string that we saved.

]]
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
	
	if type(opts.json)=="table" then -- turn json opts into string
		opts.json=wjson.encode(opts.json)
	end

	local r,m=core.save(g[0],opts)
	core.info(g[0],g)
	if not r and g.err then return nil,g.err end
	return r,m -- first value may be a datastring if no filename was given or g[0] if it was,
end

--[[#lua.wetgenes.grd.duplicate

	ga = g:duplicate()

Create a duplicate of this grd and return it.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.duplicate=function(g)
	return grd.create(g)
end

--[[#lua.wetgenes.grd.convert

	g:convert(fmt)

Convert this grd to a new format, eg start with an 8 bit indexed grd 
and convert it to 32 bit like by passing in wgrd.FMT_U8_RGBA as the fmt.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.convert=function(g,fmt)
	if type(fmt) == "string" then
		fmt=grd.stringtonum(fmt)
	end
	local r,e=core.convert(g[0],fmt)
	core.info(g[0],g)
	return (r and g),e
end

--[[#lua.wetgenes.grd.create_convert

	g:create_convert(fmt)

Like convert but returns a new grd rather than converting in place.

]]
base.create_convert=function(g,fmt)
	if type(fmt) == "string" then
		fmt=grd.stringtonum(fmt)
	end
	local r,e=core.create_convert(g[0],fmt)
	return (r and grd.create(r)),e
end

--[[#lua.wetgenes.grd.clear

	g:clear(color)

Fill this grd with a single color, the color passed in depends on the 
format of the grd, it could be an index value for 8bit images or a 
32bit value for rgba images.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.clear=function(g,color)
	local r=core.clear(g[0],color)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.quant

	g:quant(num)

Convert to an 8bit indexed image containing a palette of the requested size.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.quant=function(g,num,dither)
	local r=core.quant(g[0],num,dither)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.attr_redux

	g:attr_redux(cw,ch,num,sub,bak)

Perform attribute clash simulation on an indexed image.

cw,ch are the width and height of each character we are simulating, 8x8 
is the right size for spectrum attrs but could be 4x8 for c64 multicolor 
mode.

num is the number of colors allowed within this area, so 2 for spectrum mode.

sub is the size of sub pallete groups, eg 16 in nes mode or 8 in 
spectrum mode, EG bright simulation in spectrum mode requires all 
colors in a attr block to be from the bright palette or the dark 
palette no mixing so this forces that grouping. Set to 0 or 256 and 
this restriction will be disabled.

bak is the index of the background color that is shared across all 
characters, set to -1 if there is no shared background color.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.

]]
base.attr_redux=function(g,cw,ch,num,sub,bak)
	local r=core.attr_redux(g[0],cw,ch,num,sub,bak)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.pixels

	g:pixels(x,y,w,h)
	g:pixels(x,y,z,w,h,d)

Read the area of pixels as a table of numerical values, the amount of 
numbers you get per pixel *depends* on the format of the grd.

	g:pixels(x,y,w,h,"")
	g:pixels(x,y,z,w,h,d,"")

Read the area of pixels as a string of byte values, the amount of bytes 
you get per pixel *depends* on the format of the grd. Note the passing 
in of an empty string to indicate that you with to receive a string.

	g:pixels(x,y,w,h,tab)
	g:pixels(x,y,z,w,h,d,tab)

Write the area of pixels from a table of numerical values which is 
provided in tab, the amount of numbers you need to provide per pixel 
*depends* on the format of the grd.

	g:pixels(x,y,w,h,str)
	g:pixels(x,y,z,w,h,d,str)

Write the area of pixels from a string of bytes which is provided in 
str, the amount of bytes you need to provide per pixel *depends* on the 
format of the grd.

	g:pixels(x,y,w,h,grd)
	g:pixels(x,y,z,w,h,d,grd)

Write the area of pixels from a grd which is provided in grd. use 
clip/layer functions to select a portion of a larger grd.

As you can see depending on the arguments given this does one of two 
things, read some pixels or write some pixels. The area that is to be 
used is provided first, as a 2d(x,y,w,h) or 3d(x,y,z,w,h,d) area. To 
read or write the entire 2d image or the first frame of an animation 
use (0,0,g.width,g.height)

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns the requested data.

]]
base.pixels=function(g,...)
	local a={...}
	local la=#a
	if type(a[la])=="table" and type(a[la][0])=="userdata" then a[la]=a[la][0] end -- get the grd userdata
	local r=core.pixels(g[0],unpack(a))
	core.info(g[0],g)
	return r
end

--[[#lua.wetgenes.grd.palette

	g:palette(x,w)
	g:palette(x,w,"")
	g:palette(x,w,tab)
	g:palette(x,w,str)
	g:palette(x,w,grd)

These are the same as g:pixels() but refer to the palette information 
which is stored as a 1 pixel high 256 pixel wide rgba image. The use of 
"" to read a string of bytes and passing in either a table of numerical 
values or string of bytes to write into the palette is the same system 
as used with pixels.

]]
base.palette=function(g,...)
	local a={...}
	local la=#a
	if type(a[la])=="table" and type(a[la][0])=="userdata" then a[la]=a[la][0] end -- get the grd userdata
	if not g.cmap then return nil end
	local r=core.palette(g[0],unpack(a))
	core.info(g[0],g)
	return r
end

--[[#lua.wetgenes.grd.resize

	g:resize(w,h,d)
	
Resize the image to the given dimensions, this does not scale the image 
data so instead each pixel will be in the same place after the resize. 
This gives a crop effect when shrinking and extra blank area at the 
bottom right when growing. Useful if for instance you want to upload a 
texture to OpenGL and need to change the size to be a power of two in 
width and height so you can mipmap it.
	
]]
base.resize=function(g,...)
	local r=core.resize(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.scale

	g:scale(w,h,d)
	
Scale the image to the given dimensions, this is currently using a 
terrible scale filter that is only any good at halving or doubling the 
size.

This should only be used to create mipmaps until it is replaced with a 
better scale filter.
	
]]
base.scale=function(g,...)
	local r=core.scale(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.slide

	g:slide(dx,dy,dz)

Slide the image along the x,y,z axis by the given amounts. The image wraps around the edges 
so no pixels are lost just moved around.

]]
base.slide=function(g,...)
	local r=core.slide(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.flipy

	g:flipy()
	
This function flips the image reversing the y axis.

Some image formats and rendering engines like to use upside down images 
so this is rather useful.

]]
base.flipy=function(g,...)
	local r=core.flipy(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.flipx

	g:flipx()
	
This function flips the image reversing the x axis.
	
]]
base.flipx=function(g,...)
	local r=core.flipx(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.xor

	g:xor(ga)

Set our image data to the XOR of the image/palette data from ga and g.

This is intended to be combined with g:shrink to work out the area of 
change between the two images.

Both grds must be the same size and format.

]]
base.xor=function(g,ga)
	local r=core.xor(g[0],ga[0])
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.shrink

	g:shrink(area)

area is an {x=0,y=0,z=0,w=100,h=100,d=100} style table specifying a 3D
area, set {z=0,d=1} for a 2D area.

You should set this to the full size of the image.

This function looks at the pixels in that area and shrinks each edge 
inwards if it is fully transparent then return this new area in the 
same table that was passed in.

You can then use this information to crop this image resulting in a 
smaller sized grd containing all the solid pixels.

]]
base.shrink=function(g,...)
	local r=core.shrink(g[0],...)
	core.info(g[0],g)
	return r and g
end

--[[#lua.wetgenes.grd.info

	g:info()

This function looks at the userdata stored in g[0] and fills in the g 
table with its values. So it refreshes the width height etc values to 
reflect any changes. This should not need to be called explicitly as it 
is called whenever we change anything.

]]
base.info=function(g,...)
	core.info(g[0],g)
	return g
end

--[[#lua.wetgenes.grd.blit

	g:blit(gb,x,y,cx,cy,cw,ch)

Blit a 2D area from one grd into another.

gb is the grd to blit from.

x,y is the location to blit too.

cx,cy,cw,ch is a clip area that should be applied to gb before it is 
blitted. EG to specify an area within gb. If not provided it will 
default to the entirety of gb,

g (destination) must be FMT_U8_RGBA and gb (source) must be 
FMT_U8_RGBA_PREMULT this function will blend the images using normal 
alpha blending.

This is not overly optimised but should be reasonably fast.

]]
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

--[[#lua.wetgenes.grd.paint

	g:paint(gb,x,y,cx,cy,cw,ch,mode,trans,color)

Blit a 2D area from one grd into another using dpaint style paint modes.

Both grids must be indexed - FMT_U8_INDEXED

gb is the grd to blit from.

x,y is the location to blit too.

cx,cy,cw,ch is a clip area that should be applied to gb before it is 
blitted. EG to specify an area within gb. If not provided it will 
default to the entirety of gb,

mode is one of the following

	PAINT_MODE_TRANS
	
Skip the transparent color.

	GRD_PAINT_MODE_COLOR
	
Skip the transparent color and make every solid pixel the same color.

	GRD_PAINT_MODE_COPY

Copy the entire area.

	GRD_PAINT_MODE_XOR

XOR the values. (Can be used to find differences in an image)

	GRD_PAINT_MODE_ALPHA

Skip the transparent colors as defined in the palette.


trans is the index of the transparent color, bground color, for use in 
the appropriate modes.

color is the index of the drawing color, foreground color, for use in 
the appropriate modes. 


This is not overly optimised but should be reasonably fast.

]]
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

--[[#lua.wetgenes.grd.copy_data

	g:copy_data(gb)

Copy all of the bitmap data from gb into g.

]]
base.copy_data=function(ga,gb)
	return core.copy_data(ga[0],gb[0])
end
--[[#lua.wetgenes.grd.copy_data_layer

	g:copy_data_layer(gb,z,zb)

Copy one layer (frame) of the bitmap data from gb into g. z is the 
depth of the layer to copy into zb is the depth of the layer to copy 
from.

]]
base.copy_data_layer=function(ga,gb,za,zb)
	return core.copy_data_layer(ga[0],gb[0],za,zb)
end

--[[#lua.wetgenes.grd.clip

	gr=g:clip(x,y,z,w,h,d)

create a clipped window into this grd

the actual data is still stored in the original, so any changes there will effect the newly returned grd

x,y,z are the staring pixel and w,h,d are the width height and depth in pixels.

If you intend to use this clipped area for an extended period of time then you should duplicate this grd once you do this.

This returns a new grd with gr.parent set to g (the original grd)

This is a very shallow dangerous copy and should only really be used for temporary actions.

]]
base.clip=function(ga,x,y,z,w,h,d)
	local g={}
	g.parent=ga -- help make sure this master grd stays alive
	setmetatable(g,meta)

	if x<0 then w=w+x x=0 end
	if y<0 then y=h-y y=0 end	
	if (x+w)>ga.width  then w=ga.width -x end
	if (y+h)>ga.height then h=ga.height-y end

	g[0]=assert(core.clip(ga[0],x,y,z,w,h,d))
	core.info(g[0],g)
	return g
end


--[[#lua.wetgenes.grd.create_normal

	gr=g:create_normal()

convert a greyscale height map  into an rgb normal map using the sobel filter.

]]
base.create_normal=function(ga)
	local gd,e=core.create_normal(ga[0])
	return gd and grd.create(gd) , e
end

	
--[[#lua.wetgenes.grd.stream

	stream=g:stream(filename)
	stream=g:stream({filename=filename,...})

Open a GIF stream, returns a table with the following functions,

	stream:write(ga)
	
Add a frame to the gif, each frame should be the same size and color map.

	stream:close()

Close the stream and finalise the GIF.

]]
base.stream=function(ga,opts)

	if type(opts)=="string" then
		opts={filename=opts}
	end

	local stream=core.stream_open(ga[0],opts)
	local it={[0]=stream}

	it.write=function(it,ga)
		core.stream_write(stream,ga[0])
		return it
	end

	it.close=function(it)
		core.stream_close(stream)
		return it
	end

	return it

end

--[[#lua.wetgenes.grd.remap

	ga:remap(gb)

Fill gb with a remaped version of ga, each pixel is mapped to the closest palette entry.

]]
base.remap=function(ga,gb,colors,dither)
	local r,m=core.remap(ga[0],gb[0],colors,dither)
	core.info(gb[0],gb)
	return r and gb,m
end


--[[#lua.wetgenes.grd.adjust_rgb

	ga:adjust_rgb(red,green,blue)

Adjust -1 to +1 in for each RGB component.

]]
base.adjust_rgb=function(g,ar,ag,ab)
	assert(core.adjust_rgb(g[0],ar,ag,ab))
	core.info(g[0],g)
	return g
end

--[[#lua.wetgenes.grd.adjust_hsv

	ga:adjust_hsv(hue,saturation,value)

Add hue and adjust -1 to +1 in for saturation and value.

]]
base.adjust_hsv=function(g,ah,as,av)
	assert(core.adjust_hsv(g[0],ah,as,av))
	core.info(g[0],g)
	return g
end

--[[#lua.wetgenes.grd.adjust_contrast

	ga:adjust_contrast(sub,con)

sub is the middle grey value, probably 127, and con is the amount of 
contrast.

A con of 0 should have no effect, a con of -1 will be a flat grey and a 
con of 1 will give a huge contrast increase.

]]
base.adjust_contrast=function(g,sub,con)
	assert(core.adjust_contrast(g[0],sub,con))
	core.info(g[0],g)
	return g
end
--[[#lua.wetgenes.grd.sort_cmap

	ga:sort_cmap()

Sort cmap into a "good" order and remap the image.

]]
base.sort_cmap=function(ga)
	assert(core.sort_cmap(ga[0]))
	core.info(ga[0],ga)
	return ga
end

--[[#lua.wetgenes.grd.fillmask

	ga:fillmask(gb,seedx,seedy)

Fill gb with a fillmask version of ga that starts the floodfill at 
seedx,seedy

]]
base.fillmask=function(ga,gb,seedx,seedy)
	local r,m=core.fillmask(ga[0],gb[0],seedx,seedy)
	core.info(gb[0],gb)
	return r and gb,m
end

return grd
