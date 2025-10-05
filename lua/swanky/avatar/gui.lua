--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wgrd=require("wetgenes.grd")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,gui)
	local gui=gui or {}
	gui.oven=oven

	gui.modname=M.modname

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl

	local zgui=oven.rebake("wetgenes.gamecake.zone.gui")

	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")

--	gui.master=gui.master or oven.rebake("wetgenes.gamecake.widgets").setup({font="Vera",text_size=16,grid_size=32,skin=0})
	gui.master=zgui.master

	require(oven.modname..".gui_code").fill(gui)
	require(oven.modname..".gui_data").fill(gui)
	require(oven.modname..".gui_plan").fill(gui)
--	require(oven.modname..".gui_cache").fill(gui)

gui.loads=function()
	oven.rebake("wetgenes.gamecake.widgets").loads()

end

gui.setup=function()

	zgui.setup()

	gui.loads()

	gui.data_setup()

	gui.plan_windows(zgui.master)

	gui.data_load("all")

--	gui.plan_windows_load()

	zgui.click.layout_load({})

	zgui.show(true)

	return gui
end

gui.clean=function()
end


gui.msg=function(m)

	zgui.msg(m)

--	gui.master:msg(m)

end

--gui.button_gui=false
gui.cursor=nil
gui.update=function()

--	gui.master:update({hx=oven.win.width,hy=oven.win.height})

-- display cursor
	if gui.cursor ~= gui.master.cursor then
		gui.cursor = gui.master.cursor
		wwin.cursor( gui.cursor or "arrow" )
	end

	zgui.update()

--[[
	local button_gui=false
	if not gui.master.hidden then
		button_gui=true
--		if gui.master.over then button_gui=true end
--		if gui.master.focus then button_gui=true end
--		print( gui.master.focus and gui.master.focus.id )
	end
	if oven.escmenu and oven.escmenu.show then button_gui=true end
	if gui.button_gui ~= button_gui then
		gui.button_gui = button_gui
		oven.ups.msg_set("gui",gui.button_gui)
	end
]]

end

gui.draw=function()
--	gl.PushMatrix()
--	gui.master:draw()
--	gl.PopMatrix()
	return zgui.draw()
end


	return gui
end
