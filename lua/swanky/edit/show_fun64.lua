--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wstr=require("wetgenes.string")
local function ls(...) print(wstr.dump({...})) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


-- keep track of all the open documents

M.bake=function(oven,show_fun64)
	local show_fun64=show_fun64 or {}
	show_fun64.oven=oven
	
	show_fun64.modname=M.modname

	local gl=oven.gl

	local main=oven.rebake(oven.modname..".main")
	local show=oven.rebake(oven.modname..".show")
	local gui=oven.rebake(oven.modname..".gui")
	local system=oven.rebake("wetgenes.gamecake.fun.system")
	
	-- smart input that can be disabled on loss of focus
	system.ups_empty=oven.ups.manifest(-1) -- create an empty
	system.ups=function(idx)
		if gui.master.focus==gui.master.ids.run or main.fullshow then
			return oven.ups.up(idx)
		else
			return system.ups_empty
		end
	end


	show_fun64.widget_msg=function(w,m)
		if system.is_setup then
			pcall(function()
				system.msg(m)
			end)
		end
	end

	show_fun64.update_draw=function()
	
		if system.is_setup then
			gui.master.ids.runtext.hidden=true
		else
			gui.master.ids.runtext.hidden=false
		end
		
		local t=gui.datas.get("run_play")
--		print(t.str,t.num)

		local setup=function(str)
			str=str or gui.master.ids.texteditor.txt.get_text()
			show_fun64.last_str=str
			system.clean()
			system.setup(str)
		end
		local update=function()
			if system.is_setup then
				system.update()
			end
		end
		local draw=function()
			if system.is_setup then				
				system.draw_into_screen()
			end
		end
		local wrap=function(f)
			return function(...)
				local suc,err = pcall(f,...)
				if not suc then
					gui.master.ids.runtext.txt.set_text(err,"error.txt")
				end
			end
		end
		setup=wrap(setup)
		update=wrap(update)
		draw=wrap(draw)

		if t.str=="restart" then

			setup()
			t:value("play")

		elseif t.str=="pause" then

			draw()

		elseif t.str=="play" then

			update()
			draw()

		elseif t.str=="autoplay" then

			local str=gui.master.ids.texteditor.txt.get_text()
			if show_fun64.last_str ~= str then
				setup(str)
			end

			update()
			draw()

		end

	end
	
	show_fun64.widget_draw=function(px,py,hx,hy) -- draw a widget of this size using opengl
	
		if system.is_setup then

			local screen=system.components.screen
			local sx=hx/screen.hx
			local sy=hy/screen.hy
			local ss=sx
			if sy<sx then ss=sy end

--[[
			local fhy=math.floor(hx*screen.hy/screen.hx)
			if fhy~=gui.master.ids.dock2.hy then
				gui.master.ids.dock2.hy=fhy
				gui.master:layout()
			end
]]

			gl.PushMatrix()
			gl.Translate(hx/2,hy/2,0)
			gl.Scale(ss)
			system.components.screen.draw_screen()
			gl.PopMatrix()
			
		end

	end
	
	return show_fun64
end
