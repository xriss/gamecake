





module(...,package.seeall)

local grd=require("wetgenes.grd")





function test_jpeg_load()


	local g=assert(grd.create("GRD_FMT_U8_BGRA",apps.dir.."dat/jpeg.jpg","jpg"))
	assert( g:save(apps.dir.."dat/jpeg.png","png") )
	
	
	assert( g:convert("GRD_FMT_U16_ARGB_1555") )
	assert( g:convert("GRD_FMT_U8_BGRA") )
	assert( g:save(apps.dir.."dat/jpeg.1555.png","png") )


end

