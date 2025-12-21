--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wgrd=require("wetgenes.grd")
local wzips=require("wetgenes.zips")
local wcsv=require("wetgenes.csv")
local wpath=require("wetgenes.path")

local bit=require("bit")
local NOT , AND , OR , XOR = bit.bnot , bit.band , bit.bor , bit.bxor

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

	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")

	local widgets_menuitem=oven.rebake("wetgenes.gamecake.widgets.menuitem")

	local finds=oven.rebake(oven.modname..".finds")
	local docs=oven.rebake(oven.modname..".docs")
	local show=oven.rebake(oven.modname..".show")
	local collect=oven.rebake(oven.modname..".collect")

	local ssettings=oven.rebake("wetgenes.gamecake.spew.settings")

	gui.master=gui.master or oven.rebake("wetgenes.gamecake.widgets").setup({font=4,skin=0})
	local gui_theme=ssettings.get("gui_theme","theme_dark_medium")
	gui.master:set_theme(gui.master.actions[gui_theme].json)

gui.loaded=false
gui.loads=function()
	if gui.loaded then return end gui.loaded=true
	oven.rebake("wetgenes.gamecake.widgets").loads()

	local filename="lua/swanky/edit/actions.csv"
	local text=assert(wzips.readfile(filename),"file not found: "..filename)
	gui.master.load_actions(wcsv.map(wcsv.parse(text)))

end

gui.setup=function()
	oven.console.linehook_safety=true

	gui.loads()

	gui.data_setup()

	gui.plan_windows()
	
--	gui.data_load("all")
--	gui.plan_windows_load()

	oven.console.linehook_safety=false
	return gui
end

gui.clean=function()
end

gui.msg=function(m)
	oven.console.linehook_safety=true

	if m.class=="action" and m.action==1 then -- deal with actions
		gui.action(m)
	end
		
	gui.master:msg(m)

	oven.console.linehook_safety=false
end


gui.do_actions={}
gui.cursor=nil
gui.update=function()
	oven.console.linehook_safety=true

--	local it=gui.texteditor
--	print( gui.testa.hx ,  gui.testb.hx , it.parent.hx , it.hx , it.scroll_widget.hx , it.scroll_widget.pan.hx )

	gui.master:update({hx=oven.win.width,hy=oven.win.height})

-- display cursor	
	if gui.cursor ~= gui.master.cursor then
		gui.cursor = gui.master.cursor
		wwin.cursor( gui.cursor or "arrow" )
	end


	for _,a in ipairs(gui.do_actions) do
		gui.action(a)
	end
	gui.do_actions={}


	oven.console.linehook_safety=false
end

gui.draw=function()
	oven.console.linehook_safety=true
	gl.PushMatrix()
	gui.master:draw()		
	gl.PopMatrix()
	oven.console.linehook_safety=false
end

	local datas=gui.master.datas

datas.set_infos({

	{
		id="test",
		help="This is a test.",
	},
	
})


-- create data
gui.data_setup=function()
	if not gui.datas then -- only setup once
	
		gui.datas=datas

		datas.new({id="list_mode"  ,class="list",  hooks=gui.hooks,num=1,list={
			{str="play"},
			{str="console"},
			{str="search"},
		}})

		datas.new({id="run_mode"  ,class="list",  hooks=gui.hooks,num=1,list={
			{str="auto"},
			{str="stoy"},
			{str="vtoy"},
			{str="fun64"},
		}})

		datas.new({id="run_state"  ,class="list",  hooks=gui.hooks,num=3,list={
			{str="stop"},
			{str="pause"},
			{str="play"},
		}})

		datas.new({id="run_auto"  ,class="number",  hooks=gui.hooks,num=1})

		datas.new({id="find_search"  ,class="string",  hooks=gui.hooks,str=""})
		datas.new({id="find_replace"  ,class="string",  hooks=gui.hooks,str=""})
		datas.new({id="find_files"  ,class="string",  hooks=gui.hooks,str=""})

	end
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



function gui.refresh_tree()
	collect.update_keepers()
	gui.master.ids.treefiles:refresh()
end

function gui.action(m)

	if m.id=="file_quit" then
	
		oven.next=true

	elseif m.id=="file_new" then

		docs.manifest():show()

	elseif m.id=="file_open" then

		gui.screen.dialogs:show({
			lines={"Load..."
			},
			file={},
			cancel=function()end,
			hooks=function(act,it)
				local window=it ; while not window.close_request and window.parent~=window do window=window.parent end
				if act=="file_name_click" then
					local path=window.file:path()
					gui.master.later_append(function()
						docs.manifest(path):show()
					end)
					window:close_request()
				end
				if act=="click" then
					window.close_request(it.id)
				end
			end
		})

	elseif m.id=="file_close" then
		if docs.doc then
			docs.doc:close()
		end
	elseif m.id=="file_reload" then
		if docs.doc then
			docs.doc:reload()
		end
	elseif m.id=="file_save" then
		if docs.doc then
			docs.doc:save()
		end
	elseif m.id=="file_saveall" then

		docs.save_all()

	elseif m.id=="file_saveas" then

		gui.screen.dialogs:show({
			lines={"Save..."
			},
			file={},
			ok=function(window)
				local path=window.file:path()
				window.master.later_append(function()
					if docs.doc then
						docs.doc:save(path)
					end
				end)
			end,
			cancel=function()end,
		})

	elseif m.id:sub(1,6)=="theme_" then -- all themes
		local a=gui.master.actions[m.id]
		if a then
			gui.theme(a.json)
			ssettings.set("gui_theme",m.id)
		end

	elseif m.id=="search_find" then

		local texteditor=gui.master.ids.texteditor
		local txt=texteditor.txt

		local word=txt.copy() or ""
		if word~="" then -- search for selected?

			local dir=wpath.dir(txt.doc.filename)
			dir=wpath.unslash(dir)
--			gui.datas.set_string("find_files",dir)

print("search_find",word,dir)

--[[
			local find=finds.get(dir,word)
			if not find then
				find=finds.create({dir=dir,word=word})
				find:add_item()
				find:scan()
			end
]]

--			gui.refresh_tree()
--			gui.master.request_redraw=true

		end


		-- show search
		gui.datas.set_string("list_mode","search")

	elseif m.id=="find_goto" then

		local texteditor=gui.master.ids.texteditor
		local txt=texteditor.txt

		txt.find_next()
		texteditor:mark_sync()

	elseif m.id=="find_replace" then

		local texteditor=gui.master.ids.texteditor
		local txt=texteditor.txt

		if texteditor:allow_changes() then


			local s=gui.datas.get_string("find_search")
			local r=gui.datas.get_string("find_replace")
			local old=txt.copy() or ""

			if old~=s then -- just searched, so replace that 
				txt.find_next()
			end
			txt.undo.replace_and_select(r)

			texteditor:mark_sync()
		end

	elseif m.id=="find_replace_all" then

		local texteditor=gui.master.ids.texteditor
		local txt=texteditor.txt

		if texteditor:allow_changes() then

			local s=gui.datas.get_string("find_search")
			local r=gui.datas.get_string("find_replace")

			local t = txt.get_text() -- get all the text

			s=s:gsub("%p","%%%0") -- escape any paterns
			r=r:gsub("%%","%%%%")

			t=t:gsub( s , r ) -- simple replace

			txt.mark(0,0,txt.hy+1,0) -- update text with a full file undo
			txt.undo.replace(t)

			texteditor:mark_sync()
		end

	end

end

function gui.theme(def)
		gui.master:clean_all()
		gui.master:set_theme(def)
		gui.plan_windows(gui.master)
end

function gui.hooks(act,w,dat)

	if act=="line_click" and dat and dat.read_file then

--print(act,dat.path)
	
		if not dat.dir then -- a file click

			local path=dat.path
			w.master.later_append(function()
			
				local doc=docs.manifest(path)
				doc:show()

				gui.refresh_tree()
			end)
		end

--[[
		if dat.mode=="file_find" then

			local path=dat.path
			w.master.later_append(function()
			
				local doc=docs.manifest(path)
				local txt=doc.txt
				doc:show()
				local texteditor=gui.master.ids.texteditor

				local hy=dat.bpos[1]
				local hxa=dat.bpos[2]
				local hxb=dat.bpos[3]

				txt.mark(hy,hxa,hy,hxb)
				texteditor:mark_sync()
				texteditor:scroll_to_view()
				texteditor.txt_dirty=true

				gui.refresh_tree()

			end)			
		end
]]

	elseif act=="click" then

		if w.action then -- auto trigger action
			gui.master.push_action_msg(w.id,w.user)
		end

--print("CLICK",w.id)

		if w.id=="font_size" then
		
			gui.font_size=w.user

		elseif w.id=="run_play_restart" then

			gui.datas.set_string("run_state","play")
			show.start_doc(0)

		elseif w.id=="run_play_start" then

			gui.datas.set_string("run_state","play")
			show.start_doc()

		elseif w.id=="run_play_pause" then

			local run_state=gui.datas.get_string("run_state")
			
			if run_state=="pause" then
				gui.datas.set_string("run_state","play")
			else
				gui.datas.set_string("run_state","pause")
			end
--			gui.master.dirty_by_data(gui.datas.get("run_state"))

		elseif w.id=="run_play_stop" then
			
			gui.datas.set_string("run_state","stop")
			
--			gui.master.dirty_by_data(gui.datas.get("run_state"))

		elseif w.id=="find_goto" then
		
			gui.do_actions[#gui.do_actions+1]={id=w.id}

		elseif w.id=="find_replace" then

			gui.do_actions[#gui.do_actions+1]={id=w.id}

		elseif w.id=="find_replace_all" then

			gui.do_actions[#gui.do_actions+1]={id=w.id}

		end


	end


	if act=="value" then

		if w.id=="run_state" then -- change

			gui.master.dirty_by_data("run_state")

		elseif w.id=="list_mode" then -- change

			gui.master.ids.bar2:set_dirty()

			gui.master.ids.console.hidden=true
			gui.master.ids.search.hidden=true
			gui.master.ids.play.hidden=true

			gui.master.ids.run_play_autoplay.hidden=true
			gui.master.ids.run_play_start.hidden=true
			gui.master.ids.run_play_stop.hidden=true
			gui.master.ids.run_play_pause.hidden=true
			gui.master.ids.run_play_restart.hidden=true

			if w.str=="console" then

				gui.master.ids.console.hidden=false

			elseif w.str=="search" then

				gui.master.ids.search.hidden=false

			elseif w.str=="play" then

				gui.master.ids.play.hidden=false

				gui.master.ids.run_play_autoplay.hidden=false
				gui.master.ids.run_play_start.hidden=false
				gui.master.ids.run_play_stop.hidden=false
				gui.master.ids.run_play_pause.hidden=false
				gui.master.ids.run_play_restart.hidden=false

			end

		end

--print("VALUE",w.id)

	end

	
end

	gui.plan_windows=function(master)	
		master=master or gui.master

		local gsiz=master.theme.grid_size

local lay=
{
	id="screen",
	size="full",class="screen",solid=true,
	{
		id="split",
		class="split",size="full",split_axis="y",
		{
			id="top",
			size="fullx fity",style="flat",highlight="none",color=0,
			fbo=true,
			{
				id="topbar",
				size="minmax",smode="topleft",hx_min=gsiz*60,hy_max=gsiz*1,class="fill",
				{
					hx=gsiz*30,hy=gsiz*1,class="three",
					{
						hx=gsiz*4,hy=gsiz*1,class="menubar",id="menubar",always_draw=true,
					},
					{
						hx=gsiz*1,hy=gsiz*1,text="Welcome to swed",id="infobar",solid=true,fbo=true, -- clip
					},
				},
				{
					px=0,py=0,hx=gsiz*30,hy=gsiz*1,class="fill",id="infobar_part2",
				},
			},
		},
		{
			class="split_drag",split_axis="x",split_order=2,
			{
				id="edit",
				style="flat",hx=400,highlight="none",color=0,
				{
					id="texteditor",
					class="texteditor",color=0,hooks=gui.hooks,
					--fbo=true, --  scale using fbo so it is smoothed
				}
			},
			{
--[[
				class="split",split_axis="y",split_order=1,split_num=gsiz*1,hx=200,
				{
					id="bar1",class="fill",
					style="flat",highlight="none",color=0,
					{
						class="menudrop",hx=gsiz*3,hy=gsiz,color=0,
						data=gui.datas.get("list_mode"),
					},
				},
				{
]]
					class="split",split_axis="y",split_order=2,split_num=gsiz*1,hx=300,
					{
						id="dock_split",
						class="split_drag",split_axis="y",split_order=2,--split_aspect=1.00,
						{
							id="dock1",
							{
								id="list",
								size="full",style="flat",hx=300,highlight="none",color=0,
								{
									id="treefiles",hidden=false,
									class="tree",size="full",hooks=gui.hooks,
									items=collect.mounts,
								},
							},
						},
						{
							id="dock2",
							style="flat",highlight="none",color=0,
							hy=300,
							{
								size="full",
								{
									id="play",hidden=false,
									color=0xff000000,
									skin=0,
									size="full",
--									sx=1,sy=1,
--									smode="topleft",
									{
										id="runfbo",
										size="full",
										hz=4096, -- depth please
										fbo=true, -- we assume we are in an fbo when we render
										{
											id="run",
											size="full",
											solid=true,
											can_focus=true,
										},
									},
									{
										id="runtext",hidden=true,
										class="texteditor",size="full",style="flat",color=0,
										opts={readonly=true,gutter_disable=true,word_wrap=true},
									},
								},
								{
									id="console",hidden=true,
									class="texteditor",size="full",style="flat",color=0,
									opts={console=true,gutter_disable=true,word_wrap=true},
									--fbo=true, --  scale using fbo so it is smoothed
									console_command=gui.console_command,
								},
								{
									id="search",hidden=true,class="fill",
									size="full",style="flat",
									fbo=true,
									{
										class="text",size="fullx",hx=gsiz*1,hy=gsiz*1,
										text=" Search for: ",
									},
									{
										class="textedit",size="fullx",hx=gsiz*1,hy=gsiz*1,
										color=0,
										data="find_search",
									},
									{
										class="text",size="fullx",hx=gsiz*1,hy=gsiz*1,
										text=" Replace with: ",
									},
									{
										class="textedit",size="fullx",hx=gsiz*1,hy=gsiz*1,
										color=0,
										data="find_replace",
									},
									{
										class="text",size="fullx",hx=gsiz*1,hy=gsiz*1,
										text=" Look in files: ",
									},
									{
										class="textedit",size="fullx",hx=gsiz*1,hy=gsiz*1,
										color=0,
										data="find_files",
									},
									{
										class="text",size="fullx",hx=gsiz*1,hy=gsiz*1,
										text="",
									},
									{
										id="find_goto",hooks=gui.hooks,
										class="button",hx=gsiz*3,hy=gsiz*1,color=1,
										text="find",
									},
									{
										id="find_replace",hooks=gui.hooks,
										class="button",hx=gsiz*3,hy=gsiz*1,color=1,
										text="replace",
									},
									{
										id="find_replace_all",hooks=gui.hooks,
										class="button",hx=gsiz*5,hy=gsiz*1,color=1,
										text="replace all",
									},
								}
							}
						},							
					},
					{
						id="bar2",class="fill",
						style="flat",highlight="none",color=0,
						fbo=true,
						{
							class="menudrop",hx=gsiz*3,hy=gsiz,color=0,
							data="list_mode",
							menu_px=0,menu_py=-2.9, -- menu position hacks
						},
						{
							id="run_play_autoplay",hooks=gui.hooks,
							class="checkbox",hx=gsiz*1,hy=gsiz*1,color=1,
							text_true=">>",
							text_false="--",
							data="run_auto",
						},
						{
							id="run_play_restart",hooks=gui.hooks,
							class="button",hx=gsiz*1,hy=gsiz*1,color=1,
							text="<",
						},
						{
							id="run_play_stop",hooks=gui.hooks,
							class="button",hx=gsiz*1,hy=gsiz*1,color=1,
							text="0",
							data="run_state",
							data_selected="stop",
						},
						{
							id="run_play_pause",hooks=gui.hooks,
							class="button",hx=gsiz*1,hy=gsiz*1,color=1,
							text="||",
							data="run_state",
							data_selected="pause",
						},
						{
							id="run_play_start",hooks=gui.hooks,
							class="button",hx=gsiz*1,hy=gsiz*1,color=1,
							text=">",
							data="run_state",
							data_selected="play",
						},
					},
--				},
			},
		},
	},
}

		gui.screen=gui.master:add(lay)

--[[
		gui.menu_datas={
			font_size={
				{id="font_size",user=0.00,text="Font Size from theme"},
				{id="font_size",user=1.00,text="Font Size 16px"},
				{id="font_size",user=1.25,text="Font Size 20px"},
				{id="font_size",user=1.50,text="Font Size 24px"},
				{id="font_size",user=1.75,text="Font Size 28px"},
				{id="font_size",user=2.00,text="Font Size 32px"},
			},
		}
]]

		widgets_menuitem.menu_add(gui.master.ids.menubar,{top=gui.master.ids.menubar,menu_data={
--			menu_px=0,menu_py=1,
	--		func_text=func_text,
			hooks=gui.hooks,
			inherit=true,

			{id="menu_file",top_menu=true,menu_data={
				{id="file_new"},
				{id="file_open"},
				{id="file_close"},
				{id="file_reload"},
				{id="file_save"},
				{id="file_saveas"},
				{id="file_saveall"},
				{id="menu_collection",menu_data={
--					{id="collection_name"},
					{id="collection_switch"},
					{id="collection_purge_file"},
					{id="collection_purge_all"},
				}},
				{id="menu_theme",text="Theme",menu_data={
					{id="theme_dark_tiny"},
					{id="theme_dark_small"},
					{id="theme_dark_medium"},
					{id="theme_dark_large"},
					{id="theme_dark_huge"},
					{id="theme_bright_tiny"},
					{id="theme_bright_small"},
					{id="theme_bright_medium"},
					{id="theme_bright_large"},
					{id="theme_bright_huge"},
				}},
				{id="file_quit"},
			}},
--			{id="menu_window",text="Windows",top_menu=true,menu_data={
--				{id="dialog",user="1",text="Dialogue 1"},
--			}},
			{id="menu_edit",top_menu=true,menu_data={
				{id="select_all"},
				{id="clip_copy"},
				{id="clip_cut"},
				{id="clip_paste"},
				{id="clip_cutline"},
				{id="edit_justify"},
				{id="edit_align"},
				{id="history_undo"},
				{id="history_redo"},
			}},

			{id="menu_search",top_menu=true,menu_data={
				{id="search_find"},
				{id="search_next"},
				{id="search_prev"},
			}},

			{id="menu_view",top_menu=true,menu_data={
				{id="view_hex"},
				{id="view_txt"},
				{id="view_txt_wrap"},
				{id="view_lex_txt"},
				{id="view_lex_lua"},
				{id="view_lex_js"},
				{id="view_lex_glsl"},
			}},

--			{id="menu_font",text="Font",top_menu=true,menu_data=gui.menu_datas.font_size},
--[[
			{id="topmenu",text="Run",top_menu=true,menu_data={
				{id="run",user="hide",text="Hide"},
				{id="run",user="glsl",text="GLSL"},
			}},
]]
		}})



--for n,v in pairs(gui.master.ids) do print(n) end

-- special preview draw
		gui.master.ids.run.draw=function(w)
			show.widget_draw(w.px,w.py,w.hx,w.hy)
		end
		gui.master.ids.run.msg=function(widget,m)
			show.widget_msg(widget,m)
		end

-- add super resize hook to top bar
		do
			local old=gui.master.ids.split.resize
			gui.master.ids.split.resize=function(widget) -- super custom layout
				local old_hx_min=gui.master.ids.topbar.hx_min
				if gui.screen.hx < gsiz*24 then
--print("double bar")
					gui.master.ids.topbar.hx_min=gsiz*24
					gui.master.ids.topbar.hy_max=gsiz*2
				else
--print("single bar")
					gui.master.ids.topbar.hx_min=gsiz*48
					gui.master.ids.topbar.hy_max=gsiz*1
				end
				if old_hx_min ~= gui.master.ids.topbar.hx_min then -- double call on state change
					old(widget)
				end
				old(widget)
			end
		end

-- font resize
		gui.font_size=0
		gui.master.ids.texteditor.hook_resize=function(it)
			local ss=gui.font_size
			if ss==0 then
				ss=gui.master.text_size/16
			end
			it.smode="topleft"
			it.hx=math.ceil(it.parent.hx/ss)
			it.hy=math.ceil(it.parent.hy/ss)
			it.sx=ss
			it.sy=ss
		end
		gui.master.ids.console.hook_resize=gui.master.ids.texteditor.hook_resize

		oven.console.linehook=gui.console_linehook
		oven.console.call.gtoken=gui.console_gtoken
		oven.console.call.gist=gui.console_gist
--		docs.refresh()

		gui.screen:windows_reset()

	end

	gui.console_gtoken=function(s)
	PRINT("GTOKEN",s)
		gui.GTOKEN=s
	end
	gui.console_gist=function(s)
	PRINT("GIST",s)
		gui.GIST=s
	end

	gui.console_linehook=function(s)
		if oven.console.linehook_safety then return end
		local console=gui.master.ids.console
		if console then
			console.txt.append_text(s)
--				console:layout()
			console:scroll_to_bottom()
			console.txt_dirty=true
		end
	end
	gui.console_command=function(s)
		s=s:match( "^>%s*(.-)%s*$" )
		if s then
			oven.console.linehook_safety=false
			oven.console.dump_eval(s)
			oven.console.linehook_safety=true
		end
	end


	return gui
end
