--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")

local _,lfs=pcall( function() return require("lfs") end )


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
		local t=wstr.split(s,"/")
		for i=#t,1,-1 do local v=t[i]
			if t[i]=="" and t[i-1]=="" then -- remove double dash
				table.remove(t,i)
			end
		end
		if #t>1 then
			widget.data_name:value(t[#t])
			t[#t]=nil
			widget.data_dir:value(table.concat(t,"/"))
		elseif #t==1 then
			widget.data_name:value(t[1])
		end
	end
	if widget.refresh then widget:refresh() end
	return widget.data_dir:value() .."/".. widget.data_name:value()
end

wfile.refresh=function(widget)
	widget:file_scan()
	widget:file_refresh()
	widget:layout()
	widget:build_m4()
end



wfile.file_scan=function(widget,cd)
	
	cd=cd or widget.data_dir:value()
	widget.files={}
	pcall( function()
		for n in lfs.dir(cd) do
			if n~="." and n~=".." then
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
			return b.name > a.name
		else
			return b.mode > a.mode
		end
	end)
	
	return widget
end

wfile.file_dir=function(widget,u)

	local t=wstr.split(widget.data_dir:value(),"/")
	for i=#t,1,-1 do local v=t[i]
		if t[i]=="" and t[i-1]=="" then
			table.remove(t,i)
		end
	end

	if type(u)=="string" then
		widget.data_dir:value( u )
		widget.history[ u ]=true
	else
		if u.name=="." then
		elseif u.name==".." then
			t[#t]=nil
			widget.data_dir:value( table.concat(t,"/") )
		else
			t[#t+1]=u.name
			widget.data_dir:value(  table.concat(t,"/") )
		end
--dprint(t)				
		if widget.data_dir:value()=="" then widget.data_dir:value( "/" ) end

		widget.history[ widget.data_dir:value() ]=true
	end
	
	widget:refresh()
end


wfile.file_hooks=function(widget,act,w)
--print(act,w.id)
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
				widget:file_refresh()
				widget:layout()
				widget:build_m4()
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
					widget:call_hook("file_name_click")
				else
					widget:file_dir(u)
					widget:call_hook("file_dir_click")
				end
			end
		end
	end
end

wfile.file_refresh=function(widget)

	local pan=widget.scroll_widget.pan
	pan:remove_all()
	
	if widget.view=="history" then
	
		local t={}
		for n,b in pairs(widget.history) do
			t[#t+1]=n
		end
--				print(wstr.dump(t))
		table.sort(t)
		for i,v in ipairs(t) do
				pan:add({hx=pan.hx,hy=20,text=v,text_align="left",hooks=widget.file_hooks,user=v,id="goto",
				text_color=0xff000044,
				text_color_over=0xff000088,
				})
		end
	
	else
	
		for i,t in ipairs(widget.files) do
			if t.mode=="file" then
				pan:add({hx=pan.hx,hy=20,text=t.name,text_align="left",hooks=widget.file_hooks,user=t,
				text_color=0xff004400,
				text_color_over=0xff008800,
				})
			elseif t.mode=="directory" then
				pan:add({hx=pan.hx,hy=20,text=t.name,text_align="left",hooks=widget.file_hooks,user=t,
				text_color=0xff000044,
				text_color_over=0xff000088,
				})
			else
				pan:add({hx=pan.hx,hy=20,text=t.name,text_align="left",hooks=widget.file_hooks,user=t,
				text_color=0xff440000,
				text_color_over=0xff440000,
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
	widget.history=def.history or {}
	widget.files={}
	widget.view="file"


	widget.data_dir  = def.data_dir  or wdata.new_data({class="string",str="."})
	widget.data_name = def.data_name or wdata.new_data({class="string",str=""})

	widget.history[ widget.data_dir:value() ]=true

-- external functions, use can be expected to call these
	widget.path			=	wfile.path
	widget.refresh		=	wfile.refresh


-- internal functions
	widget.file_scan		=	wfile.file_scan
	widget.file_dir			=	wfile.file_dir
	widget.file_refresh		=	wfile.file_refresh
	widget.file_hooks		=	function(act,w) return wfile.file_hooks(widget,act,w) end


	widget:add({hx=widget.hx/2,hy=25,color=0xffcccccc,text="parent",hooks=widget.file_hooks,id="parent"})
	widget:add({hx=widget.hx/2,hy=25,color=0xffcccccc,text="history",hooks=widget.file_hooks,id="history"})

	widget:add({hx=widget.hx,hy=5})

	widget:add({hx=widget.hx,hy=25,class="textedit",color=0xffcccccc,data=widget.data_dir,clip2=true,hooks=widget.file_hooks,id="dir"})
	
	widget:add({hx=widget.hx,hy=5})

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy-90,class="scroll"})

	widget:add({hx=widget.hx,hy=5})

	widget:add({hx=widget.hx,hy=25,class="textedit",color=0xffcccccc,data=widget.data_name,clip2=true})

	widget:file_scan()
	widget:file_refresh()


	return widget
end


	return wfile
end
