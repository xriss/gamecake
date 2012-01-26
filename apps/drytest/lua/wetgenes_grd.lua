

module(...,package.seeall)

local grd=require("wetgenes.grd")



function test_jpg_8888_t1()
	do_jpg_8888("t1")
end
function test_jpg_1555_t1()
	do_jpg_1555("t1")
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
function test_jpg_1555_t2()
	do_jpg_1555("t2")
end
function test_jpg_8_t2()
	do_jpg_8("t2")	
end
function test_jpg_8x_t2()
	do_jpg_8x("t2")	
end

function test_png_8888_t3()
	do_png_8888("t3")
end
function test_png_1555_t3()
	do_png_1555("t3")
end
function test_png_8_t3()
	do_png_8("t3")
end
function test_png_8x_t3()
	do_png_8x("t3")
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


function do_jpg_8888(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:save("dat/grd/"..name..".out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".out.png","dat/grd/"..name..".chk.png") )

end

function do_png_8888(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.png","png"))
	assert( g:save("dat/grd/"..name..".out.png","png") )
	
	assert_true( do_file_compare("dat/grd/"..name..".out.png","dat/grd/"..name..".chk.png") )

end

function do_jpg_1555(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("GRD_FMT_U16_ARGB_1555") )
	assert( g:convert("GRD_FMT_U8_BGRA") )
	assert( g:save("dat/grd/"..name..".1555.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".1555.out.png","dat/grd/"..name..".1555.chk.png") )
end

function do_png_1555(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.png","png"))
	assert( g:convert("GRD_FMT_U16_ARGB_1555") )
	assert( g:convert("GRD_FMT_U8_BGRA") )
	assert( g:save("dat/grd/"..name..".1555.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".1555.out.png","dat/grd/"..name..".1555.chk.png") )
end

function do_jpg_8(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("GRD_FMT_U8_INDEXED") )
	assert( g:save("dat/grd/"..name..".8.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8.out.png","dat/grd/"..name..".8.chk.png") )
end
	
function do_png_8(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.png","png"))
	assert( g:convert("GRD_FMT_U8_INDEXED") )
	assert( g:save("dat/grd/"..name..".8.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8.out.png","dat/grd/"..name..".8.chk.png") )
end
	
function do_jpg_8x(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.jpg","jpg"))
	assert( g:convert("GRD_FMT_U8_INDEXED") )
	assert( g:convert("GRD_FMT_U8_BGRA") )
	assert( g:save("dat/grd/"..name..".8x.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8x.out.png","dat/grd/"..name..".8x.chk.png") )
	
end

function do_png_8x(name)

	local g=assert(grd.create("GRD_FMT_U8_BGRA","dat/grd/"..name..".bse.png","png"))
	assert( g:convert("GRD_FMT_U8_INDEXED") )
	assert( g:convert("GRD_FMT_U8_BGRA") )
	assert( g:save("dat/grd/"..name..".8x.out.png","png") )

	assert_true( do_file_compare("dat/grd/"..name..".8x.out.png","dat/grd/"..name..".8x.chk.png") )
	
end
