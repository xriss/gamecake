--
-- (C) 2017 Kriss@XIXs.com
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


M.bake=function(oven,wtexteditor)

	wtexteditor=wtexteditor or {} 
	wtexteditor.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")

function wtexteditor.update(widget)
	return widget.meta.update(widget)
end

function wtexteditor.draw(widget)
	return widget.meta.draw(widget)
end

wtexteditor.refresh=function(widget)
	widget:texteditor_refresh()
	widget:resize()
	widget:layout()
	widget:build_m4()
end


wtexteditor.texteditor_hooks=function(widget,act,w)
print(act,w.id)
end

wtexteditor.texteditor_refresh=function(widget)

	local pan=widget.scroll_widget.pan
	pan:remove_all()

	for i,v in ipairs(widget.lines or {}) do
		pan:add({hx=pan.hx,hy=20,text=v,text_align="left",color=0,})
	end
--[[
	for i,t in ipairs(widget.texteditors) do
		if t.mode=="texteditor" then
			pan:add({class="button",hx=pan.hx,hy=20,text=t.name,text_align="left",hooks=widget.texteditor_hooks,user=t,
			color=0,
			})
		elseif t.mode=="directory" then
			pan:add({class="button",hx=pan.hx,hy=20,text=t.name,text_align="left",hooks=widget.texteditor_hooks,user=t,
			color=0x1f000000,
			})
		end
	end
]]

end

function wtexteditor.redo_text(widget,text)

	if text then -- set new text
		widget.lines=wstr.split_lines(text)
		widget:refresh()
	end
print(#widget.lines)

	return widget.lines and table.concat(widget.lines) or ""
end

function wtexteditor.setup(widget,def)

	widget.class="texteditor"
	
	widget.update=wtexteditor.update
	widget.layout=wfill.layout
	widget.draw=wtexteditor.draw
	widget.refresh=wtexteditor.refresh

-- external functions, user can be expected to call these
	widget.redo_text			=	wtexteditor.redo_text

-- internal functions
	widget.texteditor_refresh	=	wtexteditor.texteditor_refresh
	widget.texteditor_hooks		=	function(act,w) return wtexteditor.texteditor_hooks(widget,act,w) end

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",size="full"})


--	wtexteditor:redo_text(def.text or "") -- set starting text

	widget:texteditor_refresh()

	return widget
end


	return wtexteditor
end
