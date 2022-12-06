--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc

	local screen=oven.rebake("wetgenes.gamecake.zone.screen")
	local shadow=oven.rebake("wetgenes.gamecake.zone.shadow")


-- create data
B.setup=function(zgui)
	log("setup",M.modname)

	local datas=zgui.master.datas
	
	datas.new({id="screen_gamma",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_draw.GAMMA,min=0,max=4,step=0.01})
	zgui.value["screen_gamma"]=function(it)
		screen.shader_qs.zone_screen_draw.GAMMA=it:value()
	end

	datas.new({id="shadow_scale",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SCALE,min=0.00,max=2,step=0.01})
	zgui.value["shadow_scale"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.SHADOW_SCALE=it:value()
	end

	datas.new({id="ao_scale",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SCALE,min=0.00,max=2,step=0.01})
	zgui.value["ao_scale"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SCALE=it:value()
	end

	datas.new({id="ao_clip",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_CLIP,min=0.00,max=1,step=0.01})
	zgui.value["ao_clip"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_CLIP=it:value()
	end

	datas.new({id="bloom_scale",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_draw.BLOOM_SCALE,min=0,max=4,step=0.01})
	zgui.value["bloom_scale"]=function(it)
		screen.shader_qs.zone_screen_draw.BLOOM_SCALE=it:value()
	end


	datas.new({id="camera_fov",class="number",hooks=zgui.hooks,num=screen.camera_fov,min=0,max=8,step=0.01})
	zgui.value["camera_fov"]=function(it)
		screen.camera_fov=it:value()
	end

	datas.new({id="screen_scale",class="number",hooks=zgui.hooks,num=0,min=-5,max=1,step=1,
		tostring=function(dat,num) return string.format("%0.2f",math.pow(2.0, (num or dat.num) )) end})
	zgui.value["screen_scale"]=function(it)
		screen.base_scale=math.pow(2.0,it:value())
	end

	datas.new({id="ao_size",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SIZE,min=0.01,max=0.5,step=0.01})
	zgui.value["ao_size"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SIZE=it:value()
	end

	datas.new({id="ao_samples",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.AO_SAMPLES,min=1,max=64,step=1})
	zgui.value["ao_samples"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.AO_SAMPLES=it:value()
	end
			
	datas.new({id="shadow_samples",class="number",hooks=zgui.hooks,num=screen.shader_qs.zone_screen_build_occlusion.SHADOW_SAMPLES,min=1,max=64,step=1})
	zgui.value["shadow_samples"]=function(it)
		screen.shader_qs.zone_screen_build_occlusion.SHADOW_SAMPLES=it:value()
	end
			
	datas.new({id="shadow_mapsize",class="number",hooks=zgui.hooks,num=12,min=8,max=16,step=1,
		tostring=function(dat,num) return string.format("%d",math.pow(2.0, (num or dat.num) )) end})
	zgui.value["shadow_mapsize"]=function(it)
		shadow.mapsize=math.pow(2.0,it:value())
	end

	datas.new({id="shadow_maparea",class="number",hooks=zgui.hooks,num=9,min=4,max=16,step=1,
		tostring=function(dat,num) return string.format("%d",math.pow(2.0, (num or dat.num) )) end})
	zgui.value["shadow_maparea"]=function(it)
		shadow.maparea=math.pow(2.0,it:value()-1)
	end

	datas.new({id="screen_mode",class="string",hooks=zgui.hooks,str=oven.console.screen_mode()})
	oven.console.screen_mode_data=datas.get("screen_mode")
	zgui.click["screen_mode_change"]=function(it)
		datas.get("screen_mode"):value( oven.console.screen_mode(true) )
	end


	local plan_windows=function()
	
		local datas=zgui.master.datas

		local def=require("wetgenes.gamecake.widgets.defs").create(zgui.master.grid_size)

		def.set({
			class="*",
			hooks=zgui.hooks,
			hx=6,
			hy=1,
		})

		local win=def.add(zgui.screen.windows,{

			class="window",px=1,py=1,id="window_screen",title="Screen",hidden=true,
			hx=12,hy=13,size="fit",

			{text="Mode"},
			{class="button",id="screen_mode_change",data="screen_mode",color=0},
			{text="Vertical FOV"},
			{class="slide",data="camera_fov",datx="camera_fov",color=0},
			{text="Screen Scale"},
			{class="slide",data="screen_scale",datx="screen_scale",color=0},
			{text="Screen Gamma"},
			{class="slide",data="screen_gamma",datx="screen_gamma",color=0},
			{text="Bloom Scale"},
			{class="slide",data="bloom_scale",datx="bloom_scale",color=0},
			{text="Shadow Scale"},
			{class="slide",data="shadow_scale",datx="shadow_scale",color=0},
			{text="AO Scale"},
			{class="slide",data="ao_scale",datx="ao_scale",color=0},
			{text="AO Clip"},
			{class="slide",data="ao_clip",datx="ao_clip",color=0},
			{text="AO Size"},
			{class="slide",data="ao_size",datx="ao_size",color=0},
			{text="AO Samples"},
			{class="slide",data="ao_samples",datx="ao_samples",color=0},
			{text="Shadow Samples"},
			{class="slide",data="shadow_samples",datx="shadow_samples",color=0},
			{text="Shadow Map Size"},
			{class="slide",data="shadow_mapsize",datx="shadow_mapsize",color=0},
			{text="Shadow Map Area"},
			{class="slide",data="shadow_maparea",datx="shadow_maparea",color=0},

		})

	end

	zgui.plan_windows_list.screen=plan_windows

end



	return B
end
