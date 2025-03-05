--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,gui)

	local gui=gui or {}
	gui.oven=oven
	oven.gui=gui	-- this is our global gui

	gui.modname=M.modname

	local cake=oven.cake
	local gl=oven.gl

	local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")


gui.show=function(yes)
	if not gui.master then return false end -- must be setup
	local ret=not gui.master.hidden

	if type(yes)=="boolean" then
		gui.master.hidden=not yes
	else
		gui.master.hidden=not gui.master.hidden
	end

	if gui.master.hidden then
		gui.master.over=nil
		if oven.win then oven.win:relative_mouse(true) end
		if oven.ups then oven.ups.enable_mouse=true end
	else
		if oven.win then oven.win:relative_mouse(false) end
		if oven.ups then oven.ups.enable_mouse=false end
	end

	if gui.master.over then
		if oven.ups then oven.ups.enable_key=false end
	else
		if oven.ups then oven.ups.enable_key=true end
	end

	-- maybe need to flag fake mouse from joypad here, or make joypad work gui somehow?

	return ret
end


gui.loads=function()
	oven.rebake("wetgenes.gamecake.widgets").loads()
	oven.cake.fonts.loads({"Vera"})
end


gui.theme=function(def)
	if not gui.master then gui.setup() end
	gui.master:clean_all()
	gui.master:set_theme(def)
	gui.plan_windows(gui.master)
end

gui.setup=function(...)
	log("setup",M.modname)

	gui.loads()

	if gui.master then return end -- only setuip once

	gui.master=gui.master or oven.rebake("wetgenes.gamecake.widgets").setup({font="Vera",skin=0,no_keymove=true})
	gui.master:set_theme(oven.rebake("wetgenes.gamecake.spew.settings").get("gui_theme","theme_dark_medium"))


	oven.rebake("wetgenes.gamecake.zone.gui_options").setup(gui)
	oven.rebake("wetgenes.gamecake.zone.gui_user").setup(gui)
	oven.rebake("wetgenes.gamecake.zone.gui_spew").setup(gui)
	oven.rebake("wetgenes.gamecake.zone.gui_screen").setup(gui)

	for i,v in ipairs({...}) do v.setup(gui) end -- user windows

--	gui.data_setup()

	gui.plan_windows()

	local datas=gui.master.datas
	if gui.master.ids.window_user then
		gui.master.ids.window_user.hidden=false
		if #datas.get("user_session"):value() > 8 then -- try auto login
			if datas.get("user_auto_login"):value()~=0 then
				if gui.click["user_login_session"] then
					(gui.click["user_login_session"])(nil,"auto")
				end
			end
		end
	end

	if gui.click["layout_load"] then
		(gui.click["layout_load"])(nil,"auto")
	end

	return gui
end


gui.clean=function()
end


gui.msg=function(m)

	if m.class=="key" and m.action==1 and m.keyname=="tab" then
		gui.show()
	end

	return gui.master:msg(m)

end


gui.cursor=nil
gui.update=function()

	gui.master:update({hx=oven.win.width,hy=oven.win.height})

-- display cursor
	if gui.cursor ~= gui.master.cursor then
		gui.cursor = gui.master.cursor
		wwin.cursor( gui.cursor or "arrow" )
	end

end


gui.draw=function()
	gl.PushMatrix()
	gui.master:draw()
	gl.PopMatrix()
end


gui.widgets_of_dat_id=function(id)
	local its={}
	local idx=0
	gui.master:call_descendents(function(w)
		if	( w.data and w.data.id==id ) or
			( w.datx and w.datx.id==id ) or
			( w.daty and w.daty.id==id ) then
			its[w]=w
		end
	end)
	return pairs(its)
end


gui.click={}
gui.value={}
gui.confirm={}
function gui.hooks(act,w,dat)

--log("dump",act,w.id)

	if act=="click" then

		local f=gui.click[w.id]
		if f then f(w) end

	end

	if act=="value" then

		local f=gui.value[w.id]
		if f then f(w) end

	end

	if act=="confirm" then

		local f=gui.confirm[w.id]
		if f then f(w) end

	end

end

gui.plan_windows_list={}

gui.plan_windows=function()

	local datas=gui.master.datas

	local gsiz=gui.master.grid_size

	local def=require("wetgenes.gamecake.widgets.defs").create()

	def.set({
		class="*",
		hooks=gui.hooks,
		hx=gsiz*5,
		hy=gsiz*1,
	})

	gui.screen=gui.master:add({size="full",class="screen",id="screen"})

	local menu =gui.screen.windows:add({hx=gsiz*1,hy=gsiz*1,class="menubar",id="menubar",always_draw=true})

	widgets_menuitem.menu_add(menu,{top=menu,menu_data={
		menu_px=0,menu_py=1,
		hooks=gui.hooks,
		inherit=true,
		{id="topmenu",text=">",hy=gsiz,hx=gsiz,top_menu=true,menu_data=function() return gui.screen:window_menu() end}, -- list all windows
	}})

	-- call extra window planing
	for n,v in pairs(gui.plan_windows_list) do v() end

	gui.screen:windows_reset()

end


	return gui
end
