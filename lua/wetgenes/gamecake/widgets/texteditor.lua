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

	local lines=widget.lines or {}

	local pan=widget.scroll_widget.pan

	pan.lines={}
	
	local px=math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)

	local gutter_width=#(tostring(#lines))+1

	for y=py+1,py+256 do
		local ps={}
		local pl=0

		local v=tostring(y)
		v=string.rep(" ",gutter_width-#v)..v.." "
		for i=1,#v do
			if pl>=256*3 then break end -- max width
			ps[pl+1]=string.byte(v,i,i)
			ps[pl+2]=0
			ps[pl+3]=0x34
			pl=pl+3
		end
		
		ps[pl+1]=32
		ps[pl+2]=0
		ps[pl+3]=0x00
		pl=pl+3
		
		ps[pl+1]=32
		ps[pl+2]=0
		ps[pl+3]=0x00
		pl=pl+3

		local v=lines[y]
		if v then
			for i=px+1,#v do
				if pl>=256*3 then break end -- max width
				ps[pl+1]=string.byte(v,i,i)
				ps[pl+2]=0
				ps[pl+3]=0x01
				pl=pl+3
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[y-py]={text=v,s=s}
	end



--	for i,v in ipairs(widget.lines or {}) do
--		pan:add({hx=pan.hx,hy=20,text=v,text_align="left",color=0,})
--	end
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
	
	local max_hx=0
	local max_hy=#widget.lines
	for i,v in ipairs(widget.lines) do
		local lv=#v
		if lv > max_hx then max_hx=lv end
	end
	
	widget.scroll_widget.pan.hx_max=max_hx*8
	widget.scroll_widget.pan.hy_max=max_hy*16
	
	widget:layout()

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

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",size="full",scroll_pan="tiles"})


	widget.scroll_widget.pan.pan_refresh=function(pan) return widget:texteditor_refresh() end -- we will do the scroll


--	wtexteditor:redo_text(def.text or "") -- set starting text

	widget:texteditor_refresh()

	return widget
end


	return wtexteditor
end
