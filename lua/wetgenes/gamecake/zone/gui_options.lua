-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wjson = require("wetgenes.json")

local log,dump=require("wetgenes.logs"):export("log","dump")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc


B.setup=function(zgui)
	log("setup",M.modname)

	local datas=zgui.master.datas

	local ssettings=oven.rebake("wetgenes.gamecake.spew.settings")

	local list={
		{ str="Dark small",    id="theme_dark_small",    },
		{ str="Dark medium",   id="theme_dark_medium",   },
		{ str="Dark large",    id="theme_dark_large",    },
		{ str="Bright small",  id="theme_bright_small",  },
		{ str="Bright medium", id="theme_bright_medium", },
		{ str="Bright large",  id="theme_bright_large",  },
	}
	for i,v in ipairs(list) do v.num=i end
	local t=ssettings.get("gui_theme")
	local ti=1
	for i,v in ipairs(list) do if v.id==t then ti=i end end
	datas.new({id="gui_theme_name",class="list",hooks=zgui.hooks,num=ti,list=list})
	zgui.value["gui_theme_name"]=function(it)
		local m=it.list[it.num]
		local a=zgui.master.actions[m.id]
		if a then
			zgui.theme(a.json)
			ssettings.set("gui_theme",m.id)
		end
		print(m.id)
	end

	zgui.click["layout_save"]=function(it)
		local windows=zgui.screen:get_windows()
		local window_layout={}
		for i,window in ipairs(windows) do
			local layout={depth=i}
			for _,n in ipairs{"id","hidden","px","py","hx","hy"} do
				layout[n]=window[n]
			end
			window_layout[window.id]=layout
		end
		ssettings.set("window_layout",window_layout)
	end
	zgui.click["layout_load"]=function(it)
		local windows=zgui.screen:get_windows()
		local window_layout=ssettings.get("window_layout",{})
		for i,window in ipairs(windows) do
			local layout=window_layout[window.id]
			if layout then
				for _,n in ipairs{"hidden","px","py","hx","hy"} do
					if window.id=="window_options" and n=="hidden" then
						-- do not change hidden state of options window
					else
						window[n]=layout[n]
					end
				end
			end
		end
		zgui.master.request_layout=true
	end
	zgui.click["layout_reset"]=function(it)
		local windows=zgui.screen:get_windows()
--		local window_layout=zgui.master.window_layout or {} -- default layout data that we can copy paste from settings file
		for i,window in ipairs(windows) do
			window.px=24*i
			window.py=24*i
			window.hx=window.win_fbo.hx
			window.hy=window.win_fbo.hy
			window.hidden=false
--[[
			local layout=window_layout[window.id] or window.reset_layout
			if layout then
				for _,n in ipairs{"hidden","px","py","hx","hy"} do
					if window.id=="window_options" and n=="hidden" then
						-- do not change hidden state of options window
					else
						window[n]=layout[n]
					end
				end
			end
]]
		end
		zgui.master.request_layout=true
	end

	zgui.click["file_quit"]=function(it)
		oven.next=true
	end

	local plan_windows=function()

		local def=require("wetgenes.gamecake.widgets.defs").create(zgui.master.grid_size)

		def.set({
			class="*",
			hooks=zgui.hooks,
			hx=12,
			hy=1,
		})

		local win=def.add(zgui.screen.windows,{

			class="window",px=4,py=2,id="window_options",title="Options",hidden=true,
			hx=12,hy=8,
			{
				class="fill",size="fit",

				{class="text",hx=3,text="Layout",},
				{class="button",hx=3,id="layout_save",text="Save",color=0},
				{class="button",hx=3,id="layout_load",text="Load",color=0},
				{class="button",hx=3,id="layout_reset",text="Reset",color=0},

				{class="text",hx=6,text_align="right",text="Theme"},
				{class="menudrop",hx=6,id="gui_theme_change",data="gui_theme_name",color=0},

				{hx=4},
				{class="button",hx=4,id="file_quit",text="Quit",color=0xcccc0000},
				{hx=4},
			},

		})

	end

	zgui.plan_windows_list.options=plan_windows

end


return B
end
