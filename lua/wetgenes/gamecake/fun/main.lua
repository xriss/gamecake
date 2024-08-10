-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wzips=require("wetgenes.zips")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

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
		fov=0,
	})

	local view_debug=views.create({
		mode="win",
		win=oven.win,
		vx=opts.width,
		vy=opts.height,
		vz=opts.height*4,
		fov=0,
	})

--	local skeys=oven.rebake("wetgenes.gamecake.spew.keys").setup{max_up=6,pad_map=2} -- upto 6 players, two on keyboard 4-6 on controllers
--	local sscores=oven.rebake("wetgenes.gamecake.spew.scores").setup(6)

	-- create 1up only by default ( multiple controller use will manifest more )
	oven.ups.reset()
	oven.ups.manifest(1)
	oven.ups.keymap(1,"full") -- single player has full keymap

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

	
	oven.rebake("wetgenes.gamecake.mods.console").data.fun=main.system.code -- use fun.whatever in console to tweak fun globals
	
	local screen=main.system.components.screen
	local args=oven.opts.args
	-- do resize only if we did not force a window size on the command line
	if not ( args["win-hx"] or args["win-hy"] or args["win-px"] or args["win-py"] ) then
		oven.win:resize(screen.hx*screen.scale,screen.hy*screen.scale)
	end
	opts.width=screen.hx*screen.scale
	opts.height=screen.hy*screen.scale
	opts.screen_scale=screen.scale

	view=views.create({
		mode="win",
		win=oven.win,
--		parent=view.parent,
--		mode="full",
		vx=screen.hx,
		vy=screen.hy,
		vz=screen.hy*4,
		fov=0,
	})

	oven.msg_view=view -- fix mouse coords using this view
	
end


main.clean=function()

	if main.system then
		main.system.clean()
	end

end

main.msg=function(m)

--	view.msg(m) -- fix mouse coords
--	print(wstr.dump(m))

--	if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys
	
	main.system.msg(m)
	
	if m.class=="key" then
		if m.action==-1 then
			if m.keyname=="f1" then -- toggle debug
			
				main.debug_flag=not main.debug_flag
				
			end
		end
	end
	
end

main.update=function()

--	srecaps.step()

	main.system.update()
	
end

main.draw=function()
	
	local screen=main.system.components.screen
	local tiles=main.system.components.tiles

	gl.ClearColor(pack.argb4_pmf4(0xf000))
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	if main.debug_flag then
	
		if screen and tiles then
	
			view_debug.mode=function(view) -- put debug view on the left side of the screen and fit tile texture

				view.win:info()
				view.hx=view.win.width/2
				view.hy=view.win.height
				view.px=0
				view.py=0

				view.vx=tiles.hx
				view.vy=tiles.hy
				view.vz=tiles.hy*4
				view.fov=0

			end

			views.push_and_apply(view_debug)
			canvas.gl_default() -- reset gl state
				
	--		gl.ClearColor(pack.argb4_pmf4(0xf000))
	--		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

			gl.PushMatrix()
			
			font.set(cake.fonts.get(1)) -- default font
			font.set_size(32,0) -- 32 pixels high

			gl.Translate( 0 , 0 ,1)
			main.system.draw_debug()
			
			gl.PopMatrix()

			views.pop_and_apply()
			
		end

	end


	if main.debug_flag then
	
		view.mode=function(view) -- put main game on the right side of the screen

			view.win:info()
			view.hx=view.win.width/2
			view.hy=view.win.height
			view.px=view.hx
			view.py=0

		end

	else
	
		view.mode="win"

	end

	views.push_and_apply(view)

--	layout.apply( opts.width,opts.height,1/4,opts.height*4 )
	canvas.gl_default() -- reset gl state
		
	gl.PushMatrix()
	
	font.set(cake.fonts.get(1)) -- default font
	font.set_size(32,0) -- 32 pixels high

	gl.Translate( view.vx/2 , view.vy/2 ,1)
--	gl.Scale(opts.screen_scale,opts.screen_scale,1)
	main.system.draw()
	
	gl.PopMatrix()

	views.pop_and_apply()
	
end
		
	return main
end

