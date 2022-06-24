--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wpath=require("wetgenes.path")

local _,lfs=pcall( function() return require("lfs") end ) ; lfs=_ and lfs

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wfile)

	wfile=wfile or {} 
	wfile.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")

function wfile.update(widget)
	return widget.meta.update(widget)
end

function wfile.draw(widget)
	return widget.meta.draw(widget)
end

--function wfile.layout(widget)
--	widget.meta.layout(widget)
--	widget.meta.build_m4(widget)
--end

-- get or set path
wfile.path=function(widget,s)
	if s then -- set
		local p=wpath.parse(wpath.resolve(s))
		widget.data_dir:value(p.dir)
		widget.data_name:value(p.file)
	end
	if widget.refresh then widget:refresh() end
	return wpath.resolve( widget.data_dir:value() , widget.data_name:value() )
end

-- get or set dirname
wfile.dir=function(widget,s)
	if s then -- set
		local p=wpath.parse(wpath.resolve(s))
		widget.data_dir:value(p.dir)
	end
	if widget.refresh then widget:refresh() end
	return wpath.parse(wpath.resolve( widget.data_dir:value() , widget.data_name:value() )).dir
end

-- get or set filename
wfile.file=function(widget,s)
	if s then -- set
		local p=wpath.parse(wpath.resolve(s))
		widget.data_name:value(p.file)
	end
	if widget.refresh then widget:refresh() end
	return wpath.parse(wpath.resolve( widget.data_dir:value() , widget.data_name:value() )).file
end


wfile.refresh=function(widget)
	widget:file_scan()
	widget:file_refresh()
	widget.master.request_layout=true
--	widget:resize()
--	widget:layout()
--	widget:build_m4()
end



wfile.file_scan=function(widget,cd)
	
	cd=cd or widget.data_dir:value()
	widget.files={}
	pcall( function()
		for n in lfs.dir(cd) do
			if n~="." then
				local t=lfs.attributes(cd.."/"..n)
				if t then
					t.name=n
					widget.files[#widget.files+1]=t
				end
			end
		end
	end)
	
	table.sort(widget.files,function(a,b)
		if a.mode == b.mode then
			return b.name:lower() > a.name:lower()
		else
			return b.mode > a.mode
		end
	end)
	
	return widget
end

wfile.file_dir=function(widget,u)

	widget.scroll_widget.daty:value(0) -- reset scroll on click

	local ps=wpath.split( widget.data_dir:value() )

	if type(u)=="string" then
		widget.data_dir:value( wpath.resolve(u) )
		widget.history[ u ]=true
	else
		if u.name=="." then
		elseif u.name==".." then
			widget.data_dir:value( wpath.resolve(ps,"..","") )
		else
			widget.data_dir:value( wpath.resolve(ps,u.name,"") )
		end

		widget.history[ widget.data_dir:value() ]=true
	end
	
	widget:refresh()
end


wfile.file_hooks=function(widget,act,w)
	if act=="unfocus_edit" or act=="timedelay" then
		if w.id=="dir" then
			widget:file_dir({name="."})
		end
	end
	if act=="click" then
		if w.id then
			if w.id=="parent" then
				widget.view="file"
				widget:file_dir({name=".."})
			elseif w.id=="history" then
				if widget.view=="history" then
					widget.view="view"
				else
					widget.view="history"
				end
				widget:refresh()
			elseif w.id=="goto" then
				widget.view="file"
				widget:file_dir(w.user)
			end
		else
			local u=w.user
			if type(u)=="table" then
--	print(u.name,u.file)
				if u.mode=="file" then
					widget.data_name:value( u.name )
					widget:call_hook_later("file_name_click")
				else
					widget:file_dir(u)
					widget:call_hook_later("file_dir_click")
				end
			end
		end
	end
	if act=="confirm" then
		if w.id then
			if w.id=="file" then
				widget:call_hook_later("file_name_click")
			elseif w.id=="dir" then
				widget:call_hook_later("file_dir_click")
			end
		end
	end
end

wfile.file_refresh=function(widget)

	local pan=widget.scroll_widget.pan
	pan:remove_all()
	
	local ss=widget.master.theme.grid_size

	if widget.view=="history" then
	
		local t={}
		for n,b in pairs(widget.history) do
			t[#t+1]=n
		end
--				print(wstr.dump(t))
		table.sort(t)
		for i,v in ipairs(t) do
				pan:add({class="button",hx=pan.hx,hy=ss,text=v,text_align="left",hooks=widget.file_hooks,user=v,id="goto",
				color=0x1f000000,
				})
		end
	
	else
	
		for i,t in ipairs(widget.files) do
			if t.mode=="file" then
				pan:add({class="button",hx=pan.hx,hy=ss,text=t.name,text_align="left",hooks=widget.file_hooks,user=t,
				color=0,
				})
			elseif t.mode=="directory" then
				pan:add({class="button",hx=pan.hx,hy=ss,text=t.name,text_align="left",hooks=widget.file_hooks,user=t,
				color=0x1f000000,
				})
			end
		end
		
	end

end


function wfile.setup(widget,def)

	widget.class="file"
	
--	widget.key=wfile.key
--	widget.mouse=wfile.mouse
	widget.update=wfile.update
	widget.layout=wfill.layout
	widget.draw=wfile.draw

-- auto add the draging button as a child
	widget.history=widget.history or {}
	widget.files={}
	widget.view="file"

	widget.file_hooks		=	function(act,w) return wfile.file_hooks(widget,act,w) end

	widget.data_dir  = widget.data_dir  or wdata.new_data({class="string",str=wpath.currentdir(),master=widget.master,hooks=widget.file_hooks})
	widget.data_name = widget.data_name or wdata.new_data({class="string",str="",master=widget.master,hooks=widget.file_hooks})

	widget.history[ widget.data_dir:value() ]=true

-- external functions, use can be expected to call these
	widget.path				=	wfile.path
	widget.dir				=	wfile.dir
	widget.file				=	wfile.file
	widget.refresh			=	wfile.refresh


-- internal functions
	widget.file_scan		=	wfile.file_scan
	widget.file_dir			=	wfile.file_dir
	widget.file_refresh		=	wfile.file_refresh

	local ss=widget.master.theme.grid_size
	local ss1=ss/24
	local ss2=ss/12


	widget:add_indent({hx=widget.hx,hy=ss,class="textedit",color=0,data=widget.data_name,clip2=true,hooks=widget.file_hooks,id="file"},ss1)

--	widget:add({hx=widget.hx,hy=5})

	widget:add_indent({hx=widget.hx,hy=ss,class="textedit",color=0,data=widget.data_dir,clip2=true,hooks=widget.file_hooks,id="dir"},ss1)
	
--	widget:add({hx=widget.hx,hy=5})

	widget:add_indent({hx=widget.hx/4,hy=ss,class="button",color=0,text="Parent",hooks=widget.file_hooks,id="parent"},ss1)
	widget:add_indent({hx=widget.hx/2,hy=ss},ss1)
	widget:add_indent({hx=widget.hx/4,hy=ss,class="button",color=0,text="History",hooks=widget.file_hooks,id="history"},ss1)

--	widget:add({hx=widget.hx,hy=5})

	widget.scroll_widget=widget:add_indent({hx=widget.hx,hy=widget.hy-(ss*3),class="scroll"},ss1)


	widget:file_scan()
	widget:file_refresh()


	return widget
end


	return wfile
end
