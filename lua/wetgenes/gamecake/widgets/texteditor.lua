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

function wtexteditor.pan_skin( oldskin )
	return function(widget)

		local x,y,hx,hy=widget.px,widget.py,widget.hx,widget.hy

		x=x+64-2
		y=y+64
		hx=4
		hy=16

		local ta=oven.gl.apply_modelview( {x    ,y+hy  ,0,1} )
		local tb=oven.gl.apply_modelview( {x    ,y     ,0,1} )
		local tc=oven.gl.apply_modelview( {x+hx ,y+hy  ,0,1} )
		local td=oven.gl.apply_modelview( {x+hx ,y     ,0,1} )
		
		local r,g,b,a=0.5,0.5,0.5,0.5

		local t={
			ta[1],	ta[2],	ta[3],	r,g,b,a,
			tb[1],	tb[2],	tb[3],	r,g,b,a,
			tc[1],	tc[2],	tc[3],	r,g,b,a,
			td[1],	td[2],	td[3],	r,g,b,a,
		}

		local ret=oldskin(widget)
		return function()
			ret()

			oven.gl.Color(1,1,1,1)
			oven.cake.canvas.flat.tristrip("rawrgba",t)

		end
	end
end

function wtexteditor.draw(widget)

--[[

	local cc=widget.master.get_color(nil,widget.text_color)
	cc[1]=cc[1]*0.25
	cc[2]=cc[2]*0.25
	cc[3]=cc[3]*0.25
	cc[4]=cc[4]*0.25

	widget.panoverlay.quad_over={
		color={1,1,1,1},
		x0=16,y0=16,
		x1=64,y1=64,
	}
	
	widget.scroll_widget.pan.skin=nil

		if widget.quad_over then -- custom quad in this widget, for use EG as a cursor
			local q=widget.quad_over
			gl.Color( unpack(q.color) )
			draw_quad(q.x0,q.y0,q.x1,q.y0,q.x1,q.y1,q.x0,q.y1)
			print(q.x0,q.y0,q.x1,q.x2)
		end

]]

	return widget.meta.draw(widget)
end

wtexteditor.refresh=function(widget)
	widget:texteditor_refresh()
	widget.master.request_layout=true
--	widget:resize()
--	widget:layout()
--	widget:build_m4()
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
	
	widget.master.request_layout=true
--	widget:layout()

	return widget.lines and table.concat(widget.lines) or ""
end

function wtexteditor.cursor(widget)
	local cursor={}


	return cursor
end

function wtexteditor.area(widget)
	local area={}


	return area
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
	
	widget.cursor=wtexteditor.cursor(widget) -- cursor location and mode
	widget.area=wtexteditor.area(widget) -- selection area


	widget.scroll_widget.pan.pan_refresh=function(pan) return widget:texteditor_refresh() end -- we will do the scroll


	widget.scroll_widget.pan.skin=wtexteditor.pan_skin( widget.scroll_widget.pan.skin )

--	wtexteditor:redo_text(def.text or "") -- set starting text

	widget:texteditor_refresh()

	return widget
end


	return wtexteditor
end
