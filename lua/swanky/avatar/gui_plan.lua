--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs ; pcall( function() lfs=require("lfs") end ) -- may not have a filesystem
local wjson=require("wetgenes.json")

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wgrd=require("wetgenes.grd")
local wzips=require("wetgenes.zips")
local bitdown=require("wetgenes.gamecake.fun.bitdown")

local function dprint(a) print(wstr.dump(a)) end


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.fill=function(gui)

	local oven=gui.oven

	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local gl=oven.gl

	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wmenuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")
	local avatar=oven.rebake(oven.modname..".avatar")

	local zgui=oven.rebake("wetgenes.gamecake.zone.gui")

--	gui.pages=gui.pages or {}

	gui.plan_windows=function(master)
		master=master or gui.master
		gui.master=master

		local gsiz=master.grid_size or 24
print(gsiz)
		local def_window=function(parent,it)
			for n,v in pairs{
				class="window",
				hx=128,
				hy=128,
				px=0,
				py=0,
				solid=true,
			} do it[n]=it[n] or v end

			it.reset_px=it.px
			it.reset_py=it.py

			return parent.windows:add(it)
		end

		local def_title=function(parent,it)
			for n,v in pairs{
				hx=gsiz*5,
				hy=gsiz,
				px=2,
				py=2,
--				color=0,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end

		local def_button=function(parent,it)
			for n,v in pairs{
				class="button",
				hx=gsiz*5,
				hy=gsiz,
				px=2,
				py=2,
				color=0,
				solid=true,
				hooks=gui.hooks,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end

		local def_checkbox=function(parent,it)
			for n,v in pairs{
				class="checkbox",
				hx=gsiz,
				hy=gsiz,
				px=2,
				py=2,
				color=0,
				solid=true,
				hooks=gui.hooks,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end

		local def_menudrop=function(parent,it)
			for n,v in pairs{
				class="menudrop",
				hx=gsiz*5,
				hy=gsiz,
				px=2,
				py=2,
				color=0,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end

		local def_xslide=function(parent,it)
			for n,v in pairs{
				class="slide",
				hooks=gui.hooks,
				datx=it.data,
				hx=gsiz*5,
				hy=gsiz,
				px=2,
				py=2,
				color=0,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end


--		local screen=gui.master:add({size="full",class="screen",id="screen",solid=true})
		local screen=zgui.screen
		gui.screen=screen
--[[
		local split=screen:add({class="split",size="full",split_axis="y",split_max=gsiz})

		local topbar=split:add({size="fullx",hy=gsiz,color=1,fbo=true})
		local menu=topbar:add({hy=gsiz,color=0,style="flat",highlight="none",class="menubar",id="menubar",always_draw=true})

		wmenuitem.menu_add(menu,{top=menu,menu_data={
			menu_px=0,menu_py=1,
	--		func_text=func_text,
			hooks=gui.hooks,
			inherit=true,
			{id="topmenu",text="File",top_menu=true,menu_data={
				{id="theme",user="bright",text="Bright Theme"},
				{id="theme",user="dark",text="Dark Theme"},
				{id="layout",user="save",text="Save Layout"},
				{id="layout",user="load",text="Load Layout"},
				{id="layout",user="reset",text="Reset Layout"},
				{id="quit",user="quit",text="Quit"},
			}},
			{id="topmenu",text="Windows",top_menu=true,menu_data={
				{id="window",user="color",text="Color"},
--				{id="window",user="view",text="View"},
				{id="window",user="pose",text="Pose"},
				{id="window",user="part",text="Part"},
				{id="window",user="pose",text="Tweak"},
				{id="window",user="files",text="Files"},
			}},
		}})

		split:insert(screen.windows)
]]

		screen.msg=avatar.msg

		local canvas=def_window(screen,{px=0,py=0,hx=gsiz*5,size="fit",id="window_color",title="Color"}).win_canvas

		def_xslide(canvas,{data=gui.datas.get("bloom_pick")})
		def_xslide(canvas,{data=gui.datas.get("bloom_add")})

		def_menudrop(canvas,{data=gui.datas.get("material")})

		def_title(canvas,{text="color",hx=gsiz*2.5,})
		def_menudrop(canvas,{data=gui.datas.get("ramp_mode"),hx=gsiz*2.5,})

		def_xslide(canvas,{id="ramp_index",data=gui.datas.get("ramp_index")})

		def_button(canvas,{id="ramp_add",text="Add",hx=gsiz*1.5,})
		def_button(canvas,{id="ramp_del",text="Del",hx=gsiz*1.5,})
		def_button(canvas,{id="ramp_sort",text="Sort",hx=gsiz*2.0,})

		def_xslide(canvas,{color=0x3fcc8888,data=gui.datas.get("ramp_red")})
		def_xslide(canvas,{color=0x3f88cc88,data=gui.datas.get("ramp_grn")})
		def_xslide(canvas,{color=0x3f8888cc,data=gui.datas.get("ramp_blu")})
		def_xslide(canvas,{                 data=gui.datas.get("ramp_pos")})


		local canvas=def_window(screen,{px=gsiz*(30-3),py=0,hx=gsiz*5,size="fit",id="window_pose",title="Pose"}).win_canvas

		def_menudrop(canvas,{data=gui.datas.get("pose")})
		def_button(canvas,{id="random",text="Random"})

		local canvas=def_window(screen,{px=gsiz*(60-5),py=0,hx=gsiz*5,size="fit",id="window_tweak",title="Tweak"}).win_canvas

		def_menudrop(canvas,{data=gui.datas.get("tweak")})
		def_title(canvas,{text="scale"})
		def_xslide(canvas,{data=gui.datas.get("tweak_scale_x")})
		def_xslide(canvas,{data=gui.datas.get("tweak_scale_y")})
		def_xslide(canvas,{data=gui.datas.get("tweak_scale_z")})
		def_xslide(canvas,{data=gui.datas.get("tweak_scale")})
		def_title(canvas,{text="rotate"})
		def_xslide(canvas,{data=gui.datas.get("tweak_rotate_x")})
		def_xslide(canvas,{data=gui.datas.get("tweak_rotate_y")})
		def_xslide(canvas,{data=gui.datas.get("tweak_rotate_z")})
		def_title(canvas,{text="position"})
		def_xslide(canvas,{data=gui.datas.get("tweak_translate_x")})
		def_xslide(canvas,{data=gui.datas.get("tweak_translate_y")})
		def_xslide(canvas,{data=gui.datas.get("tweak_translate_z")})
		def_button(canvas,{id="tweak_reset",text="Reset"})


		local canvas=def_window(screen,{px=0,py=gsiz*12,hx=gsiz*7,size="fit",id="window_part",title="Part"}).win_canvas

		for i,name in pairs(avatar.part_names) do
			local count=avatar.part_counts[name] or 1
			for i=1,count do
				local s= (i==1) and "" or ("_"..i)
				def_checkbox(canvas,{data=gui.datas.get("part_flags_"..name..s),data_mask=1,text_true="L"})
				def_checkbox(canvas,{data=gui.datas.get("part_flags_"..name..s),data_mask=2,text_true="R"})
				def_menudrop(canvas,{data=gui.datas.get("part_"..name..s)})
			end
		end

		local canvas=def_window(screen,{px=gsiz*(60-10),py=gsiz*16,hx=gsiz*10,size="fit",id="window_files",title="Files"}).win_canvas

		def_button(canvas,{id="file_load",text="Load"})
		def_button(canvas,{id="file_save",text="Save"})

		canvas:add({hx=gsiz*10,hy=gsiz*14,class="file",id="file_load",hooks=gui.hooks,
			data_dir=gui.datas.get("file_dir"),
			data_name=gui.datas.get("file_name"),
--			history=gui.data.file_history,
			})


		gui.screen:windows_reset()


		gui.datas.set_string("ramp_mode","simple")

--screen:add_split({window=master.ids.window_color,split_axis="x",split_order=1})
--screen:add_split({window=master.ids.window_files,split_axis="x",split_order=1})

	end

	gui.show_request=function(opts)

		local master=gui.master
		local gsiz=master.grid_size or 24

		local def_window=function(parent,it)
			for n,v in pairs{
				class="window",
				hx=128,
				hy=128,
				px=0,
				py=0,
				solid=true,
				flags={nodrag=true},
			} do it[n]=it[n] or v end

			return parent:add{class="center",solid=true,transparent=0x88000000}:add(it)
		end

		local def_button=function(parent,it)
			for n,v in pairs{
				hx=gsiz*5,
				hy=gsiz,
				px=2,
				py=2,
				color=0,
				solid=true,
				hooks=gui.hooks,
			} do it[n]=it[n] or v end
			return parent:add_border(it)
		end

		local window=def_window(gui.screen,{px=gui.screen.hx/2,py=gui.screen.hy/2,hx=gsiz*10,size="fit",id="request",title=""})
		local canvas=window.win_canvas

		for i,v in ipairs(opts.lines or {} ) do

			local it={

				hx=gsiz*10,
				hy=gsiz,
				px=2,
				py=2,
--				color=0,
				text=v
			}
			canvas:add_border(it)
		end

		local hooks=function(act,w,dat)

			if act=="click" then

				window.parent:remove() -- hide/delete

				gui.master:layout() -- need to layout at least once to get everything in the right place

				if opts[w.id] then (opts[w.id])() end

			end

		end

		if opts.yes then
			def_button(canvas,{hooks=hooks,class="button",id="yes",text="Yes"})
		end
		if opts.ok then
			def_button(canvas,{hooks=hooks,class="button",id="ok",text="OK"})
		end
		if opts.no then
			def_button(canvas,{hooks=hooks,class="button",id="no",text="No"})
		end

		gui.master:layout() -- need to layout at least once to get everything in the right place
		gui.master:layout() -- need to layout at least once to get everything in the right place

	end

--[[
	gui.layout_filename=wwin.files_prefix.."avatar.layout.json"

	gui.plan_windows_load=function()

		if lfs then

print("Loading "..gui.layout_filename)

			local s=""
			local fp=io.open(gui.layout_filename,"r")
			if fp then
				s=fp:read("*all")
				fp:close()
			end
			local tab=wjson.decode(s)


			gui.master:call_descendents(function(w)
				if w.id and w.class=="window" then
					local v=tab and tab.windows and tab.windows[w.id]
					if v then
						w.px=v.px
						w.py=v.py
						w.hx=v.hx
						w.hy=v.hy
						w.hidden=v.hidden
					end
				end
			end)

			gui.master:layout()
			gui.master:call_descendents(function(w) w:set_dirty() end)
		end

	end

	gui.plan_windows_save=function()

		local tab={windows={}}

		gui.master:call_descendents(function(w)
			if w.id and w.class=="window" then
				tab.windows[w.id]={
					px=w.px,py=w.py,
					hx=w.hx,hy=w.hy,
					hidden=w.hidden,
				}
			end
		end)

		if lfs then

print("Saving "..gui.layout_filename)

		local fp=io.open(gui.layout_filename,"w")
			if fp then
				fp:write(wjson.encode(tab,{pretty=true}))
				fp:close()
			end

		end

	end

	gui.plan_windows_reset=function()
		local gsiz=gui.master.grid_size or 24
		local x,y=gsiz,gsiz
		gui.master:call_descendents(function(w)
			if w.id and w.class=="window" then
				w.px=w.reset_px
				w.py=w.reset_py
				w.hx=w.win_fbo.hx
				w.hy=w.win_fbo.hy
				w.hidden=false
			end
		end)

		gui.screen:windows_reset()

	end
]]


	return gui
end
