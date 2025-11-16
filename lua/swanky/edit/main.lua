#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wxox=require("wetgenes.xox")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wpath=require("wetgenes.path")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,main)
	main=main or {}
	main.modname=M.modname
	
	oven.modname="swanky.edit"
	
	if oven.console then
		oven.console.input_disable=true
	end
	if oven.escmenu then -- if we have an escape menu
		oven.escmenu.active=false -- then disable it
	end

	local gl=oven.gl
	local cake=oven.cake
	local sheets=cake.sheets
	local opts=oven.opts
	local canvas=cake.canvas
	local views=cake.views
	local font=canvas.font
	local flat=canvas.flat

	local view=views.create({
		parent=views.get(),
		mode="full",
		vx=oven.win.width,
		vy=oven.win.height,
		vz=8192,
		fov=0,
	})

--	local skeys=oven.rebake("wetgenes.gamecake.spew.keys")
--	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")
--	skeys.setup({max_up=1}) -- also calls srecaps.setup
--	skeys.set_opts("yestyping",true) -- disable widget moves with keys/joystick



--	local beep=oven.rebake(oven.modname..".beep")
	local gui=oven.rebake(oven.modname..".gui")
	local cmd=oven.rebake(oven.modname..".cmd")
	local docs=oven.rebake(oven.modname..".docs")
	local finds=oven.rebake(oven.modname..".finds")
	local collect=oven.rebake(oven.modname..".collect")

	local show=oven.rebake(oven.modname..".show")

	main.loads=function()

		oven.cake.fonts.loads({1,4}) -- load 1st builtin font, a basic 8x8 font

		gui.loads()

	end
			
	main.setup=function()
	
		collect.setup()

		main.loads(oven)
		
		show.setup()

		gui.setup()

		cmd.start()
		
		docs.show() -- hide main text as we have nothing to show yet

		docs.config_load() -- load old docs ( collect has prepped the data )

		if cmd.args then -- load doc from command line

			local fname=cmd.args.data[1]
			if fname then
				docs.manifest(fname):show()
			end

		end

		
		gui.master.set_focus( gui.master.ids.texteditor.scroll_widget.pan )
		gui.refresh_tree()
		
		if cmd.args and cmd.args.data.run then
			main.fullshow=true
		end

	end


	main.clean=function()

	end

	main.fullshow=false
	main.msg=function(m)

		-- toggle fullshow
		if m.class=="key" and m.keyname=="escape" and m.action==-1 then
			main.fullshow=not main.fullshow
		end


		view.msg(m) -- fix mouse coords

--		if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys

		if main.fullshow then

			show.widget_msg(gui.master,m)
			
		else

			gui.msg(m)

		end
--dprint(m)

	end



	main.update=function()
	
		if main.fullshow then
			oven.frame_rate_wakeup() -- do not sleep in this mode

			oven.console.input_disable=false

			show.update()
		
		else
			oven.console.input_disable=true
			oven.console.show=false
			oven.console.show_hud=false

--		srecaps.step()
		
--		finds.update()
		
		docs.update()

		gui.update()
		
		show.update()
		

		if not gui.master.focus  then -- auto focus text editor when no other focus
			if not gui.master.ids.texteditor.hidden then -- except if it is hidden
				gui.master.set_focus( gui.master.ids.texteditor.scroll_widget.pan )
			end
		end

		gui.master.ids.runfbo:set_dirty()
		gui.master.ids.run:set_dirty()

		end
	end

	main.draw=function()

		local state=gui.datas.get_string("run_state")
		if state=="play" then -- update and draw
			show.update_draw() -- called outside of the gui to prepare fbos
		end

--  we want the main view to track the window size

		oven.win:info()
		view.vx=oven.win.width
		view.vy=oven.win.height
		view.vz=8192
		
		views.push_and_apply(view)
		canvas.gl_default() -- reset gl state
			
		gl.ClearColor(pack.argb4_pmf4(0xf000))
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

--		gl.PushMatrix()
		
		font.set(cake.fonts.get(4)) -- default font
		font.set_size(16,0)
		
--		canvas.gl_default() -- reset gl state

		
--		gl.Translate(view.vx*0.5,view.vy*0.9,view.vz*0.0) -- top left corner is origin

--[[
		local ss=view.vy*0.125*gui.datas.get_value("zoom")*0.01
		gl.Scale(ss,ss,ss)
		gl.Rotate(90,1,0,0)
		gl.Rotate( -gui.datas.get_value("rotate") ,0,0,1)
]]		

--		beep.draw()

--		gl.PopMatrix()

		if main.fullshow then

			show.widget_draw(0,0,view.vx,view.vy)
			
		else

			gui.draw()

		end
		
		views.pop_and_apply()
		
	end

	return main
end

