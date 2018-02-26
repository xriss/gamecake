

module(...,package.seeall)

local wstr=require("wetgenes.string")

local grd=require("wetgenes.grd")

local grddiff=require("wetgenes.grddiff")
local grdpaint=require("wetgenes.grdpaint")

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

function test_grddiff_history()

	local g=grd.create("U8_INDEXED",128,128,1)
	g:palette(0,16,{
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

	local history=grddiff.history(g)
	local canvas=grdpaint.canvas(g)
	
	for i=0,15 do
		local x,y=i*8,i*8

		history.draw_pull_frame(0)
		canvas.color(i)
		canvas.box(x,y,x+7,y+7)
		history.draw_push_frame()

		assert( history.grd:save("dat/grd/diff.base."..i..".out.png","png") )
	end

	for i=14,0,-1 do
		history.undo()
		assert( history.grd:save("dat/grd/diff.undo."..i..".out.png","png") )
		assert_true( do_file_compare("dat/grd/diff.base."..i..".out.png","dat/grd/diff.undo."..i..".out.png") )
	end
	for i=1,15 do
		history.redo()
		assert( history.grd:save("dat/grd/diff.redo."..i..".out.png","png") )
		assert_true( do_file_compare("dat/grd/diff.base."..i..".out.png","dat/grd/diff.redo."..i..".out.png") )
	end

	io.open("dat/grd/diff.base.lua","w"):write( wstr.dump(history.list) )

end
