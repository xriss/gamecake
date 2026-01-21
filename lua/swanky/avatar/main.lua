#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wxox=require("wetgenes.xox")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,main)
	main=main or {}
	main.modname=M.modname

	oven.modname="swanky.avatar"

	local gl=oven.gl
	local cake=oven.cake
	local sheets=cake.sheets
	local opts=oven.opts
	local canvas=cake.canvas
	local views=cake.views
	local font=canvas.font
	local flat=canvas.flat

	oven.ups.keymap(1,"full") -- 1up has basic keyboard mappings
	oven.upnet=oven.rebake("wetgenes.gamecake.upnet")

	local view=views.create({
		parent=views.get(),
		mode="full",
		vx=oven.win.width,
		vy=oven.win.height,
		vz=oven.win.height*4,
		fov=0,
	})

--	local skeys=oven.rebake("wetgenes.gamecake.spew.keys")
--	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")
--	skeys.setup({max_up=1}) -- also calls srecaps.setup
--	skeys.set_opts("yestyping",true) -- disable widget moves with keys/joystick



	local main_zone=oven.rebake(oven.modname..".main_zone")

	local avatar=oven.rebake(oven.modname..".avatar")
	local gui=oven.rebake(oven.modname..".gui")


	main.view=view


	main.loads=function()

		oven.cake.fonts.loads({1,"Vera"}) -- load 1st builtin font, a basic 8x8 font

		avatar.loads()
		gui.loads()

		main_zone.loads()

	end

	main.setup=function()


		main.loads(oven)

		avatar.setup()
		gui.setup()

		main_zone.setup()

		oven.upnet.setup()

	end


	main.clean=function()

		main_zone.clean()

	end

	main.mouse={view.vx/2,view.vy/2}

	main.msg=function(m)

		view.msg(m) -- fix mouse coords

		if m.class=="mouse" then
			main.mouse[1]=m.x or avatar.mouse[1]
			main.mouse[2]=m.y or avatar.mouse[2]
		end

--		if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys

		gui.msg(m)

--print(gui.master.over)

		if not gui.master.over then -- gui is not in control
			main_zone.msg(m)
		end

	end


	local ii=0
	local frame=0

	main.update=function()

--		srecaps.step()

		gui.update()

		frame=frame+0.2
		if frame>=24 then frame=frame-24 end

		ii=ii+1/4


		avatar.update()

		main_zone.update()


	end

	main.random=function()
		avatar.random()
	end
	main.snap=function()
		oven.snaps.save()
	end

	main.draw=function()


--  we want the main view to track the window size

		oven.win:info()
		view.vx=oven.win.width
		view.vy=oven.win.height
		view.vz=oven.win.height*4

		views.push_and_apply(view)
		canvas.gl_default() -- reset gl state

		gl.ClearColor(pack.argb4_pmf4(0xf000))
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		gl.PushMatrix()

		font.set(cake.fonts.get("Vera")) -- default font
		font.set_size(16,0)

		canvas.gl_default() -- reset gl state


		gl.Translate(view.vx*0.5,view.vy*0.5,view.vz*0.0) -- top left corner is origin

--		local ss=( view.vx>view.vy and view.vx or view.vy) /1024
--		gl.Scale(ss,ss,ss)

		gl.PushMatrix()
		main_zone.draw()
--		avatar.draw()
		gl.PopMatrix()

		gl.PopMatrix()

		if not gui.master.hidden then
			gui.draw()
		end

		views.pop_and_apply()

	end

	return main
end

