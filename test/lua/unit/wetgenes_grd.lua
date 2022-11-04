

module(...,package.seeall)

local grd=require("wetgenes.grd")
local wstr=require("wetgenes.string")


function test_png_clipdupe_t1()
	do_png_clipdupe("t1",16,16,0,16,16,1)
end
function test_png_clipdupe_t3()
	do_png_clipdupe("t3",32,32,0,32,32,1)
end

function test_png_slide_t1()
	do_png_slide("t1",16,16,0)
end
function test_png_slide_t3()
	do_png_slide("t3",-16,-16,0)
end

function test_do_png_bump()
	do_png_bump("bump")
end

function test_do_png_bump2()
	do_png_bump("bump2")
end

function test_do_png_json()
	do_png_json("t1")
end

function test_do_png_wmem()
	do_png_wmem("t4")
end

--function test_do_gif_wmem()
--	do_gif_wmem("t6")
--end

--function test_do_gif_stream()
--	do_gif_stream("t6")
--end

function test_do_jpg()
	do_jpg()
end

function test_gif_t6()
	do_gif("t6")
end


function test_jpg_mem_t1()
	do_jpg_mem("t1")
end

function test_jpg_8888_t1()
	do_jpg_8888("t1")
end
function test_jpg_5551_t1()
	do_jpg_5551("t1")
end
function test_jpg_8_t1()
	do_jpg_8("t1")	
end
function test_jpg_8x_t1()
	do_jpg_8x("t1")	
end

function test_jpg_8888_t2()
	do_jpg_8888("t2")
end
function test_jpg_5551_t2()
	do_jpg_5551("t2")
end
function test_jpg_8_t2()
	do_jpg_8("t2")	
end
function test_jpg_8x_t2()
	do_jpg_8x("t2")	
end

function test_png_mem_t4()
	do_png_mem("t4")
end
function test_png_mem_t5()
	do_png_mem("t5")
end

function test_png_8888_t4()
	do_png_8888("t4")
end

function test_png_8888_t3()
	do_png_8888("t3")
end
function test_png_5551_t3()
	do_png_5551("t3")
end
function test_png_4444_t3()
	do_png_4444("t3")
end
function test_png_8_t3()
	do_png_8("t3")
end
function test_png_8x_t3()
	do_png_8x("t3")
end

function test_png_remap_t3()
	do_png_remap("t3")
end


function test_png_grey_t3()
	do_png_8888("grey")
end


function test_premult_t3()
	do_premult("t3")
end

function test_png_8_attr_redux()
	do_png_8_attr_redux("t1")
end

function test_greyscale()
	local g=assert(grd.create("dat/grd/brokengreyscale.png"))
	g:convert("U8_RGBA_PREMULT")
	assert( g:save("dat/grd/brokengreyscale.out.png","png") )
end

function test_bw()
	local g=assert(grd.create("dat/grd/brokenbw.png"))
	g:convert("U8_RGBA_PREMULT")
	assert( g:save("dat/grd/brokenbw.out.png","png") )
end

function do_file_write(f,d)
	local fp=assert(io.open(f,"wb"))
	local d=assert(fp:write(d))
	fp:close()
end

function do_file_read(f)
	local fp=assert(io.open(f,"rb"))
	local d=assert(fp:read("*a"))
	fp:close()
	return d
end

function do_file_compare(f1,f2)
	local d1=do_file_read(f1)
	local d2=do_file_read(f2)
	return d1==d2
end

local function do_file_assert(g,name,test)
	assert( g:save("dat/grd/"..name.."."..test..".out.png","png") )	
	local fnamea,fnameb="dat/grd/"..name.."."..test..".out.png","dat/grd/"..name.."."..test..".chk.png"
	local diff=do_file_compare(fnamea,fnameb)
	if not diff then print("\n\nFILES DIFFER : "..fnamea.."   "..fnameb.."\n") end
	assert( diff )
end

function do_premult(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))

	g:convert("U8_RGBA_PREMULT")
	
	local c=assert(grd.create("U8_RGBA",320,240,1))
	
	assert( c:blit(g,0,0) )
	
	
	do_file_assert(c,name,"premult")
end

function do_png_mem(name)

	local g=assert(grd.create())
	local dat=do_file_read("dat/grd/"..name..".bse.png")
	g:load_data(dat,"png")

	do_file_assert(g,name,"mem")
end

function do_jpg_mem(name)

	local g=assert(grd.create())
	local dat=do_file_read("dat/grd/"..name..".bse.jpg")
	g:load_data(dat,"jpg")

	do_file_assert(g,name,"mem")
end


function do_jpg_8888(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	g:convert("U8_RGBA")

	do_file_assert(g,name,"8888")
end

function do_png_8888(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	g:convert("U8_RGBA")

	do_file_assert(g,name,"8888")
end

function do_jpg_5551(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U16_RGBA_5551") )
	assert( g:convert("U8_RGBA") )

	do_file_assert(g,name,"5551")
end

function do_png_5551(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U16_RGBA_5551") )
	assert( g:convert("U8_RGBA") )

	do_file_assert(g,name,"5551")
end

function do_png_4444(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U16_RGBA_4444") )
	assert( g:convert("U8_RGBA") )

	do_file_assert(g,name,"4444")
end

function do_jpg_8(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U8_INDEXED") )

	do_file_assert(g,name,"8")
end
	
function do_png_8(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_INDEXED") )

	do_file_assert(g,name,"8")
end
	
function do_jpg_8x(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U8_INDEXED") )
	assert( g:convert("U8_RGBA") )
	
	do_file_assert(g,name,"8x")
end

function do_png_8x(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_INDEXED") )
	assert( g:convert("U8_RGBA") )
	
	do_file_assert(g,name,"8x")
end

function do_gif(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.gif","gif"))

	do_file_assert(g,name,"gif")
end

function do_jpg()
	local wgrd=grd
	local	g=assert(wgrd.create("../mods/data/imgs/preloader/kittyscreen.jpg")) -- load it
--print( wstr.dump(g) )
	assert(g:convert(wgrd.FMT_U8_RGBA_PREMULT)) -- premult default
--print( wstr.dump(g) )
	assert( g:save("dat/grd/kittytest.jpg","jpg") )

--	do_file_assert(g,name,"jpg")

end

function do_png_wmem(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
--	print(wstr.dump(g))
	local d=assert( g:save({fmt="png"}) )

	do_file_assert(g,name,"wmem")
end

function do_gif_wmem(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.gif","gif"))
--	print(wstr.dump(g))
	local d=assert( g:save({fmt="gif"}) )
	do_file_write("dat/grd/"..name..".wmem.out.gif",d)
	assert( do_file_compare("dat/grd/"..name..".wmem.out.gif","dat/grd/"..name..".wmem.chk.gif") )
end

function do_gif_stream(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.gif","gif"))
--	print(wstr.dump(g))
--	local d=assert( g:save({fmt="gif"}) )
--	do_file_write("dat/grd/"..name..".stream.out.gif",d)

	local stream=g:stream({filename="dat/grd/"..name..".stream.out.gif"})
	stream.write(g:clip(0,0,0,g.width,g.height,1))
	stream.write(g:clip(0,0,1,g.width,g.height,1))
	stream.write(g:clip(0,0,2,g.width,g.height,1))
	stream.write(g:clip(0,0,3,g.width,g.height,1))
	stream.write(g:clip(0,0,2,g.width,g.height,1))
	stream.write(g:clip(0,0,1,g.width,g.height,1))
	stream.close(g)

	assert( do_file_compare("dat/grd/"..name..".stream.out.gif","dat/grd/"..name..".stream.chk.gif") )
end

function do_png_8_attr_redux(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_INDEXED") )
	assert( g:attr_redux(8,8,2) )

	do_file_assert(g,name,"8.attr")
end

function do_png_json(name)

	local json=[[{"test":"data","more":"data"}]]

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:save({filename="dat/grd/"..name..".json.png",fmt="png",json=json}) )
	local g=assert(grd.create("dat/grd/"..name..".json.png","png"))

	assert( g.json ) -- check we got some json back
--	print(wstr.dump(g.json))
end

function do_png_bump(name)

	local json=[[{"test":"data","more":"data"}]]

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_LUMINANCE") )
	local b=assert(g:create_normal())

	do_file_assert(b,name,"normal")

end

function do_png_remap(name)

	local ga=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( ga:convert("U8_RGBA") )

	local gb=assert(grd.create("U8_INDEXED",ga.width,ga.height,ga.depth))
	gb:palette(0,16,{
		0,0,0,0,
		0,0,0,255,
		255,0,0,255,
		128,0,0,255,
		0,255,0,255,
		0,128,0,255,
		0,0,255,255,
		0,0,128,255,
		255,64,64,255,
		128,64,64,255,
		64,255,64,255,
		64,128,64,255,
		64,64,255,255,
		64,64,128,255,
		64,64,64,255,
		128,128,128,255,
	})
	assert( ga:remap(gb) )

	do_file_assert(gb,name,"remap")
end


function do_png_slide(name,dx,dy,dz)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert(g:slide(dx,dy,dz))

	do_file_assert(g,name,"slide")
end

function do_png_clipdupe(name,x,y,z,w,h,d)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	local g=g:clip(x,y,z,w,h,d):duplicate()

	do_file_assert(g,name,"clipdupe")
end
