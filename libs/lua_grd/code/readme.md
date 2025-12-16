

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## code.swankyquant.algorithm


SwankyQuant
===========

To use this code remember to include it only once with SWANKYQUANT_C 
defined or no code will be generated. For example.


	#define SWANKYQUANT_C
	#include "swankyquant.h"
	
	...
	
	swanky_quant( input, length, 256, output, palette, 6 );
	
	swanky_quant_remap( input, length, 256, output, palette, width, 6 );
	

The remap step is unnecessary unless you want your output image 
dithered.


SwankyQuant is a palette selection algorithm made of a combination of 
simple actions, it requires a weighted compare function between two 
colours and everything else is brute force selection based on this 
compare. Since this compare is only used for yes/no logic then a 4d 
distance squared between two 32bit colours is the default. Unlike the 
alternatives we fully support alpha in out palettes.

The trick that makes this possible is using a byte per pixel thinking 
buffer. Since you are presumably intending to remap the original image 
into a buffer exactly like this I do not consider this an excessive 
extra resource, especially since this buffer is *also* the output 
indexed image when we are finished.

This buffer allows us to keep track of which bucket we have assigned to 
each pixel, this combined with multiple image passes allows pixels to 
jump from bucket to bucket until the allocation of pixels to available 
buckets is reasonably optimal. We don't actually pick a palette we just 
sort pixels into buckets with other pixels of a similar colour. When 
finished the average colour of each bucket is the resulting palette. 

Essentially we skip the generate a palette step and instead remap the 
image repeatedly with the output palette automatically effected by 
every pixels decision. Giving nice simple feedback until we hit a 
stable optimum solution.

In order to reduce the number of image passes (each one is expensive) 
we use an adjustable weight on the decision to jump so that early 
passes have more of an effect and later passes cleanup any mistakes, 
this combined with a reasonable starting state (map by luminescence) 
helps to reduce the cost of multiple passes and brute force searches to 
an acceptable level.

Due to the brute force compares, generating a smaller palette is faster 
than a larger palette. In Swanky Paint we are much more interested in 
16/32 colour palettes than 256 colour palettes so this is an additional 
advantage.

If the input contains only a handful of unique colours then we are very 
likely to pick these exact colours provided there is enough room for 
them all in the output palette.


REFERENCE LINKS TO OTHER ALGORITHMS
===================================

1.	http://www.imagemagick.org/script/quantize.php
2.	https://www.researchgate.net/publication/232079905_Kohonen_neural_networks_for_optimal_colour_quantization
3.	https://scientificgems.wordpress.com/stuff/neuquant-fast-high-quality-image-quantization/


In comparison to the above I would list the following pros and cons.


	SWANKYQUANT PROS
	================
	
		. Is simple to understand and tweak.

		. Works with 32bit alpha.

		. Works well with a small output palette.

		. Will pick every distinct input if palette size allows.
	

	SWANKYQUANT CONS
	================
	
		. Uses output memory as a thinking buffer.

		. Is slower for large images and 256 colour palettes.


For large images I would recommend scaling the image down in size then 
using the smaller image to generate the palette. Less precise but a 
huge performance gain. This is the logic I use in Swanky Paint and the 
grd library when loading larger images.




## code.swankyquant.color_distance


Compare two colors and return a distance value. This is non linear, as 
we do not need to bother with the sqrt since the numbers are only used 
for comparison.



## code.swankyquant.color_distance_weight


Compare how close a match it would be if we added our color(weighted) 
to a bucket.

Each bucket is 5 doubles and contains an rgba accumulator in 
[0][1][2][3]  and a number of samples in [5] Doubles must be used 
rather than floats or we will encounter number overflow for images that 
are larger than 256x256 pixels. 

(A float or int version for smaller images might be worthwhile.)

The weight should be a natural 1.0 but since we want to encourage early 
bucket jumping we fudge it high at the start and then lower it on later 
passes to bring stability to the feedback system. This is simply to 
reduce the number of passes over the data required to get good results.

This function also accounts for most of the thinking time so 
could/should be optimised more.



## code.swankyquant.swanky_quant


Reduce input rgba data to the requested number of index colors(2-256)

reads data from input[4*length]

writes data to output[length] and palette[4*colors]

Make sure this memory is available.

Quality is the number of passes and 6 or more is recommended for good 
results.



## code.swankyquant.swanky_quant_remap


Perform a final remap on a swanky_quant output image with an optional 
amount of ordered dithering.

	dither = 0  ==   1 bit pattern  ( no dithering )
	dither = 1  ==   3 bit patterns
	dither = 2  ==   5 bit patterns
	dither = 3  ==   9 bit patterns
	dither = 4  ==  17 bit patterns ( recommended dithering )
	dither = 5  ==  33 bit patterns
	dither = 6  ==  65 bit patterns ( maximum dithering )
	
width is the width of the image, we need to know this so we can dither 
nicely.

All other values are the same as used in the swanky_quant function 
call.



## lua.wetgenes.grd


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



## lua.wetgenes.grd.adjust_contrast


	ga:adjust_contrast(sub,con)

sub is the middle grey value, probably 127, and con is the amount of 
contrast.

A con of 0 should have no effect, a con of -1 will be a flat grey and a 
con of 1 will give a huge contrast increase.



## lua.wetgenes.grd.adjust_hsv


	ga:adjust_hsv(hue,saturation,value)

Add hue and adjust -1 to +1 in for saturation and value.



## lua.wetgenes.grd.adjust_rgb


	ga:adjust_rgb(red,green,blue)

Adjust -1 to +1 in for each RGB component.



## lua.wetgenes.grd.attr_redux


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



## lua.wetgenes.grd.blit


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



## lua.wetgenes.grd.clear


	g:clear(color)

Fill this grd with a single color, the color passed in depends on the 
format of the grd, it could be an index value for 8bit images or a 
32bit value for rgba images.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.clip


	gr=g:clip(x,y,z,w,h,d)

create a clipped window into this grd

the actual data is still stored in the original, so any changes there will effect the newly returned grd

x,y,z are the staring pixel and w,h,d are the width height and depth in pixels.

If you intend to use this clipped area for an extended period of time then you should duplicate this grd once you do this.

This returns a new grd with gr.parent set to g (the original grd)

This is a very shallow dangerous copy and should only really be used for temporary actions.



## lua.wetgenes.grd.convert


	g:convert(fmt)

Convert this grd to a new format, eg start with an 8 bit indexed grd 
and convert it to 32 bit like by passing in wgrd.FMT_U8_RGBA as the fmt.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.copy_data


	g:copy_data(gb)

Copy all of the bitmap data from gb into g.



## lua.wetgenes.grd.copy_data_layer


	g:copy_data_layer(gb,z,zb)

Copy one layer (frame) of the bitmap data from gb into g. z is the 
depth of the layer to copy into zb is the depth of the layer to copy 
from.



## lua.wetgenes.grd.create


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



## lua.wetgenes.grd.create_convert


	g:create_convert(fmt)

Like convert but returns a new grd rather than converting in place.



## lua.wetgenes.grd.create_normal


	gr=g:create_normal()

convert a greyscale height map  into an rgb normal map using the sobel filter.



## lua.wetgenes.grd.destroy


	g:destroy()

Free the grd and associated memory. This will of course be done 
automatically by garbage collection but you can force it explicitly 
using this function.
	


## lua.wetgenes.grd.duplicate


	ga = g:duplicate()

Create a duplicate of this grd and return it.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.fillmask


	ga:fillmask(gb,seedx,seedy)

Fill gb with a fillmask version of ga that starts the floodfill at 
seedx,seedy



## lua.wetgenes.grd.flipx


	g:flipx()
	
This function flips the image reversing the x axis.
	


## lua.wetgenes.grd.flipy


	g:flipy()
	
This function flips the image reversing the y axis.

Some image formats and rendering engines like to use upside down images 
so this is rather useful.



## lua.wetgenes.grd.info


	g:info()

This function looks at the userdata stored in g[0] and fills in the g 
table with its values. So it refreshes the width height etc values to 
reflect any changes. This should not need to be called explicitly as it 
is called whenever we change anything.



## lua.wetgenes.grd.load


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



## lua.wetgenes.grd.load_data


	g:load_data(datastring,format)

Load an image from memory.
 
Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.load_file


	g:load_file(filename,format)

Load an image from the file system.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.
 


## lua.wetgenes.grd.paint


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



## lua.wetgenes.grd.palette


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



## lua.wetgenes.grd.pixels


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



## lua.wetgenes.grd.quant


	g:quant(num)

Convert to an 8bit indexed image containing a palette of the requested size.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.remap


	ga:remap(gb)

Fill gb with a remaped version of ga, each pixel is mapped to the closest palette entry.



## lua.wetgenes.grd.reset


	g:reset()

Reset the grd which will now be a blank image of 0 dimensions.

Returns nil,error if something goes wrong so can be used with assert 
otherwise returns g so that we can chain the result.



## lua.wetgenes.grd.resize


	g:resize(w,h,d)
	
Resize the image to the given dimensions, this does not scale the image 
data so instead each pixel will be in the same place after the resize. 
This gives a crop effect when shrinking and extra blank area at the 
bottom right when growing. Useful if for instance you want to upload a 
texture to OpenGL and need to change the size to be a power of two in 
width and height so you can mipmap it.
	


## lua.wetgenes.grd.save


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



## lua.wetgenes.grd.scale


	g:scale(w,h,d)
	
Scale the image to the given dimensions, this is currently using a 
terrible scale filter that is only any good at halving or doubling the 
size.

This should only be used to create mipmaps until it is replaced with a 
better scale filter.
	


## lua.wetgenes.grd.shrink


	g:shrink(area)

area is an {x=0,y=0,z=0,w=100,h=100,d=100} style table specifying a 3D
area, set {z=0,d=1} for a 2D area.

You should set this to the full size of the image.

This function looks at the pixels in that area and shrinks each edge 
inwards if it is fully transparent then return this new area in the 
same table that was passed in.

You can then use this information to crop this image resulting in a 
smaller sized grd containing all the solid pixels.



## lua.wetgenes.grd.slide


	g:slide(dx,dy,dz)

Slide the image along the x,y,z axis by the given amounts. The image wraps around the edges 
so no pixels are lost just moved around.



## lua.wetgenes.grd.sort_cmap


	ga:sort_cmap()

Sort cmap into a "good" order and remap the image.



## lua.wetgenes.grd.stream


	stream=g:stream(filename)
	stream=g:stream({filename=filename,...})

Open a GIF stream, returns a table with the following functions,

	stream:write(ga)
	
Add a frame to the gif, each frame should be the same size and color map.

	stream:close()

Close the stream and finalise the GIF.



## lua.wetgenes.grd.xor


	g:xor(ga)

Set our image data to the XOR of the image/palette data from ga and g.

This is intended to be combined with g:shrink to work out the area of 
change between the two images.

Both grds must be the same size and format.
