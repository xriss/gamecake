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
	
	gui.modname=M.modname

	local cake=oven.cake
	local gl=oven.gl

	local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

	local screen=oven.rebake("wetgenes.gamecake.zone.screen")
	local shadow=oven.rebake("wetgenes.gamecake.zone.shadow")


-- create data
gui.data_setup=function()
--	if not gui.datas then -- only setup once
	
	local datas=gui.master.datas
	
	datas.new({id="screen_gamma",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_draw.GAMMA,min=0,max=4,step=0.01})
	gui.value["screen_gamma"]=function(it)
		screen.shader_qs.zone_screen_draw.GAMMA=it:value()
	end

	datas.new({id="shadow_scale",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SCALE,min=0.00,max=2,step=0.01})
	gui.value["shadow_scale"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.SHADOW_SCALE=it:value()
	end

	datas.new({id="ao_scale",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SCALE,min=0.00,max=2,step=0.01})
	gui.value["ao_scale"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SCALE=it:value()
	end

	datas.new({id="ao_clip",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_CLIP,min=0.00,max=1,step=0.01})
	gui.value["ao_clip"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_CLIP=it:value()
	end

	datas.new({id="bloom_scale",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_draw.BLOOM_SCALE,min=0,max=4,step=0.01})
	gui.value["bloom_scale"]=function(it)
		screen.shader_qs.zone_screen_draw.BLOOM_SCALE=it:value()
	end


	datas.new({id="camera_fov",class="number",hooks=gui.hooks,num=screen.camera_fov,min=0,max=8,step=0.01})
	gui.value["camera_fov"]=function(it)
		screen.camera_fov=it:value()
	end

	datas.new({id="screen_scale",class="number",hooks=gui.hooks,num=0,min=-5,max=1,step=1,
		tostring=function(dat,num) return string.format("%0.2f",math.pow(2.0, (num or dat.num) )) end})
	gui.value["screen_scale"]=function(it)
		screen.base_scale=math.pow(2.0,it:value())
	end

	datas.new({id="ao_size",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SIZE,min=0.01,max=0.5,step=0.01})
	gui.value["ao_size"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SIZE=it:value()
	end

	datas.new({id="ao_samples",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SAMPLES,min=1,max=64,step=1})
	gui.value["ao_samples"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SAMPLES=it:value()
	end
			
	datas.new({id="shadow_samples",class="number",hooks=gui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SAMPLES,min=1,max=64,step=1})
	gui.value["shadow_samples"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.SHADOW_SAMPLES=it:value()
	end
			
	datas.new({id="shadow_mapsize",class="number",hooks=gui.hooks,num=12,min=8,max=16,step=1,
		tostring=function(dat,num) return string.format("%d",math.pow(2.0, (num or dat.num) )) end})
	gui.value["shadow_mapsize"]=function(it)
		shadow.mapsize=math.pow(2.0,it:value())
	end

	datas.new({id="shadow_maparea",class="number",hooks=gui.hooks,num=9,min=4,max=16,step=1,
		tostring=function(dat,num) return string.format("%d",math.pow(2.0, (num or dat.num) )) end})
	gui.value["shadow_maparea"]=function(it)
		shadow.maparea=math.pow(2.0,it:value()-1)
	end

	datas.new({id="screen_mode",class="string",hooks=gui.hooks,str=oven.console.screen_mode()})
	oven.console.screen_mode_data=datas.get("screen_mode")
	gui.click["screen_mode_change"]=function(it)
		datas.get("screen_mode"):value( oven.console.screen_mode(true) )
	end


end


gui.loads=function()
	oven.rebake("wetgenes.gamecake.widgets").loads()
end


gui.setup=function()

	gui.master=gui.master or oven.rebake("wetgenes.gamecake.widgets").setup({font="Vera",text_size=20,grid_size=40,skin=0,no_keymove=true})

	gui.loads()

	gui.data_setup()

	gui.plan_windows()
	
	return gui
end


gui.clean=function()
end


gui.msg=function(m)
		
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
		{id="topmenu",text=">",top_menu=true,menu_data=function() return gui.screen:window_menu() end}, -- list all windows
	}})


	local canvas=def.add(gui.screen.windows,{class="window",px=gsiz*1,py=gsiz*1,hx=gsiz*5,size="fit",id="window_screen",title="Screen",hidden=true}).win_canvas

	def.add(canvas,{text="Mode",hx=gsiz*3})
	def.add(canvas,{class="button",hx=gsiz*2,id="screen_mode_change",data=datas.get("screen_mode"),color=0})
	def.add(canvas,{text="Vertical FOV"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("camera_fov"),color=0})
	def.add(canvas,{text="Screen Scale"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("screen_scale"),color=0})
	def.add(canvas,{text="Screen Gamma"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("screen_gamma"),color=0})
	def.add(canvas,{text="Bloom Scale"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("bloom_scale"),color=0})
	def.add(canvas,{text="Shadow Scale"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("shadow_scale"),color=0})
	def.add(canvas,{text="AO Scale"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("ao_scale"),color=0})
	def.add(canvas,{text="AO Clip"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("ao_clip"),color=0})
	def.add(canvas,{text="AO Size"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("ao_size"),color=0})
	def.add(canvas,{text="AO Samples"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("ao_samples"),color=0})
	def.add(canvas,{text="Shadow Samples"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("shadow_samples"),color=0})
	def.add(canvas,{text="Shadow Map Size"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("shadow_mapsize"),color=0})
	def.add(canvas,{text="Shadow Map Area"})
	def.add(canvas,{class="slide",data="datx",datx=datas.get("shadow_maparea"),color=0})

	gui.screen:windows_reset()

end


	return gui
end
