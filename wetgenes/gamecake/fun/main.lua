-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wzips=require("wetgenes.zips")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local bitdown_font_4x6=require("wetgenes.gamecake.fun.bitdown_font_4x6")
local bitdown_font_4x8=require("wetgenes.gamecake.fun.bitdown_font_4x8")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,main)
	main=main or {}
	main.modname=M.modname
	
	oven.modgame="fun"
	
	local gl=oven.gl
	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
--	local layouts=cake.layouts
	local views=cake.views
	local font=canvas.font
	local flat=canvas.flat

--	local layout=layouts.push_child{} -- we shall have a child layout to fiddle with
	local view=views.create({
		parent=views.get(),
		mode="full",
		vx=opts.width,
		vy=opts.height,
		vz=opts.height*4,
		fov=1/4,
	})

	local skeys=oven.rebake("wetgenes.gamecake.spew.keys").setup{max_up=6,pad_map=2} -- upto 6 players, two on keyboard 4-6 on controllers
	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps").setup(6)
	local sscores=oven.rebake("wetgenes.gamecake.spew.scores").setup(6)


main.loads=function()

	oven.cake.fonts.loads({1}) -- load 1st builtin font, a basic 8x8 font
	
	oven.cake.images.loads({
	})
	
end
		
main.setup=function()

	main.loads()

	main.system=oven.rebake("wetgenes.gamecake.fun.system").load_and_setup()
--		.load_and_setup("test",
--		"lua/"..(M.modname):gsub("%.","/"):gsub("[^/]+$",""))

end


main.clean=function()

	main.system.clean()
	
end

main.msg=function(m)
--	print(wstr.dump(m))

	view.msg(m) -- fix mouse coords

	if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys
	
	main.system.msg(m)
	
end

main.update=function()

	srecaps.step()

	main.system.update()
	
end

main.draw=function()
	
	views.push_and_apply(view)

--	layout.apply( opts.width,opts.height,1/4,opts.height*4 )
	canvas.gl_default() -- reset gl state
		
	gl.ClearColor(pack.argb4_pmf4(0xf000))
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.PushMatrix()
	
	font.set(cake.fonts.get(1)) -- default font
	font.set_size(32,0) -- 32 pixels high

	gl.Translate(opts.width/2,opts.height/2,1)
	gl.Scale(opts.screen_scale,opts.screen_scale,1)
	main.system.draw()
	
	gl.PopMatrix()

	views.pop_and_apply()
	
end
		
	return main
end

