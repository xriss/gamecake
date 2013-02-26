-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local snames=require("wetgenes.gamecake.spew.names")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local screen_size=256

local t_randumbs={
	"Holding the {adjective} {thing}.",
	"Giant {adjective} {thing} aproaching.",
	"This is not my {adjective} {thing}.",
	"Beware of {adjective} {thing}.",
	"Lost {adjective} {thing}.",
	"Found {adjective} {thing}.",
	"Searching for {adjective} {thing}.",
	"Testing {adjective} {thing}.",
	"Checking {adjective} {thing}.",
	"Dropped {adjective} {thing}.",
	"Disabled {adjective} {thing}.",
	"Enabled {adjective} {thing}.",
	"Buying {adjective} {thing}.",
	"Selling {adjective} {thing}.",
}

local ts={"Bios OK.","Looking for boot.","Other shoe found.","Bable mode initiated!"}



M.bake=function(oven,main)
	local main=main or {}
		
	local gl=oven.gl
	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local layout=cake.layouts.create{}
	
main.loads=function()
	oven.cake.fonts.loads({1}) -- load 1st builtin font, a basic 8x8 font
end

main.setup=function()

	if main.setup_done then return end -- warning, this is called repeatedly...

	main.setup_done=true
	main.loads()
end

main.clean=function()
	main.setup_done=false
end

main.msg=function(m)
end

main.update=function()

	local s=t_randumbs[math.random(1,#t_randumbs)]
	local t={}
	t.thing    =snames.random_noun()
	t.adjective=snames.random_adjective()
	
	s=wstr.replace(s,t)
	for i,v in ipairs( wstr.smart_wrap(s,30) ) do
		ts[#ts+1]=v
	end
--	ts[#ts+1]=""
	
	
	while #ts>(32-3) do
		table.remove(ts,1)
	end

end


main.count=0

-- this is the only function we expect to be called
-- and we expect it to be called very sporadically
main.draw=function()

	layout.viewport() -- did our window change?
	layout.project23d(screen_size,screen_size,1/4,screen_size*4)
	canvas.gl_default() -- reset gl state

	gl.ClearColor(pack.argb4_pmf4(0xf001))
	gl.Clear(gl.COLOR_BUFFER_BIT)--+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( layout.pmtx )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()
	gl.Translate(-screen_size/2,-screen_size/2,-screen_size*2) -- top left corner is origin

	gl.PushMatrix()
	
	font.set(cake.fonts.get(1)) -- default font
	font.set_size(8,0) -- 32 pixels high

	gl.Color(pack.argb4_pmf4(0xf18f))

	font.set_xy( 8 , 8 )
	font.draw("MemCheck: "..main.count)
--print("MemCheck: "..main.count.." : "..tostring(oven.preloader_enabled))

	for i=1,#ts do
		font.set_xy( 8, 16+i*8 )
		font.draw(ts[i])
	end

	gl.PopMatrix()	

	main.count=main.count+1
end
		
	return main
end

