

module(...,package.seeall)

local grd=require("wetgenes.grd")
local wstr=require("wetgenes.string")


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

function test_png_grey_t3()
	do_png_8888("grey")
end


function test_premult_t3()
	do_premult("t3")
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

function do_premult(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))

	g:convert("U8_RGBA_PREMULT")
	
	local c=assert(grd.create("U8_RGBA",320,240,1))
	
	assert( c:blit(g,0,0) )
	
	assert( c:save("dat/grd/"..name..".premult.out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".premult.out.png","dat/grd/"..name..".premult.chk.png") )

end

function do_png_mem(name)

	local g=assert(grd.create())
	local dat=do_file_read("dat/grd/"..name..".bse.png")
	g:load_data(dat,"png")
	assert( g:save("dat/grd/"..name..".mem.out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".mem.out.png","dat/grd/"..name..".mem.chk.png") )

end

function do_jpg_mem(name)

	local g=assert(grd.create())
	local dat=do_file_read("dat/grd/"..name..".bse.jpg")
	g:load_data(dat,"jpg")
	assert( g:save("dat/grd/"..name..".mem.out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".mem.out.png","dat/grd/"..name..".mem.chk.png") )

end


function do_jpg_8888(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	g:convert("U8_RGBA")
	assert( g:save("dat/grd/"..name..".out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".out.png","dat/grd/"..name..".chk.png") )

end

function do_png_8888(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	g:convert("U8_RGBA")
	assert( g:save("dat/grd/"..name..".out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".out.png","dat/grd/"..name..".chk.png") )

end

function do_jpg_5551(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U16_RGBA_5551") )
	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".5551.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".5551.out.png","dat/grd/"..name..".5551.chk.png") )
end

function do_png_5551(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U16_RGBA_5551") )
	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".5551.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".5551.out.png","dat/grd/"..name..".5551.chk.png") )
end

function do_png_4444(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U16_RGBA_4444") )
	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".4444.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".4444.out.png","dat/grd/"..name..".4444.chk.png") )
end

function do_jpg_8(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U8_INDEXED") )
	assert( g:save("dat/grd/"..name..".8.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8.out.png","dat/grd/"..name..".8.chk.png") )
end
	
function do_png_8(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_INDEXED") )
	assert( g:save("dat/grd/"..name..".8.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8.out.png","dat/grd/"..name..".8.chk.png") )
end
	
function do_jpg_8x(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("U8_INDEXED") )
	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".8x.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8x.out.png","dat/grd/"..name..".8x.chk.png") )
	
end

function do_png_8x(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.png","png"))
	assert( g:convert("U8_INDEXED") )
	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".8x.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8x.out.png","dat/grd/"..name..".8x.chk.png") )
	
end

function do_gif(name)

	local g=assert(grd.create("dat/grd/"..name..".bse.gif","gif"))

--print( wstr.dump(g) )

--	assert( g:convert("U8_RGBA") )
	assert( g:save("dat/grd/"..name..".out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".out.png","dat/grd/"..name..".chk.png") )
	
end
