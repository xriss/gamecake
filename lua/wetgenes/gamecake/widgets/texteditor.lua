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

	local throb=(widget.master.throb>=128)
	if throb ~= widget.throb then -- dirty throb...
		widget.throb=throb
		widget.scroll_widget.pan:set_dirty()
	end

	return widget.meta.update(widget)
end

function wtexteditor.pan_skin( oldskin )
	return function(pan)
		local panmtx=oven.gl.SaveMatrix() -- need to cache the current matrix for later draw call
		local panskin=oldskin(pan)
		return function()
			panskin()

			if pan.texteditor.throb then -- draw the blinking cursor
			
				local cx = pan.texteditor.cursor.x - pan.texteditor.lines.px
				local cy = pan.texteditor.cursor.y - pan.texteditor.lines.py
				

				oven.gl.PushMatrix()
				oven.gl.LoadMatrix(panmtx)

				if cx>=1 and cx<=pan.hx/8 and cy>=1 and cy<=pan.hy/16 then -- visible cursor

					local x,y,hx,hy=pan.px,pan.py,pan.hx,pan.hy

					x=x+(cx-1+pan.texteditor.lines.gutter)*8-2
					y=y+(cy-1)*16
					hx=4
					hy=16

					oven.gl.Color(0.5,0.5,0.5,0.5)
					oven.cake.canvas.flat.tristrip("pos",{
						x		,y+hy	,0,
						x		,y		,0,
						x+hx	,y+hy	,0,
						x+hx	,y		,0,
					})

				end

				oven.gl.PopMatrix()

			end
		end
	end
end

function wtexteditor.draw(widget)
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

	local strings=widget.lines.strings or {}

	local pan=widget.scroll_widget.pan

	pan.lines={}
	
	local px=-math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)

	widget.lines.px=px -- remember the scroll positions in characters
	widget.lines.py=py

	for y=py+1,py+256 do
		local ps={}
		local pl=0

		local v=tostring(y)
		v=string.rep(" ",widget.lines.gutter-3-#v)..v.." "
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

		local v=strings[y]
		if v then
			for i=px+1,#v do
				if pl>=256*3 then break end -- max width
				ps[pl+1]=string.byte(v,i,i) or 32
				ps[pl+2]=0
				ps[pl+3]=0x01
				pl=pl+3
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[y-py]={text=v,s=s}
	end

end

function wtexteditor.lines(texteditor)
	local lines={}

	lines.strings={}
	lines.hx=0 -- widest string
	lines.hy=0 -- number of strings

	lines.px=0 -- scroll position
	lines.py=0
	
	lines.gutter=0 -- gutter width

	lines.get_string=function(idx)
		return lines.strings[idx]
	end

	lines.set_string=function(idx,str)
		lines.strings[idx]=str
	end

	lines.changed_lines=function()

		lines.gutter=#(tostring(lines.hy))+4

		texteditor.scroll_widget.pan.hx_max=lines.hx*8
		texteditor.scroll_widget.pan.hy_max=lines.hy*16
		
		texteditor.master.request_layout=true

		texteditor:refresh()

	end


	lines.set=function(text)

		if text then -- set new text
			lines.strings=wstr.split_lines(text)
		end
		
		lines.hx=0
		lines.hy=#lines.strings
		for i,v in ipairs(lines.strings) do
			local lv=#v
			if lv > lines.hx then lines.hx=lv end
		end
		
		lines.changed_lines()
		
	end

	lines.get=function()
		return table.concat(lines.strings) or ""
	end

	return lines
end

function wtexteditor.cursor(texteditor)
	local cursor={}
	
	cursor.x=1
	cursor.y=1
	
	cursor.moved=function()

		local lines=texteditor.lines

		if cursor.y>lines.hy then cursor.y=lines.hy end
		if cursor.y<1 then cursor.y=1 end

		local s=lines.get_string(cursor.y)
		local hx=s and #s or 0
		if hx > lines.hx then lines.hx=hx end

		if cursor.x>hx+1 then cursor.x=hx+1 end
		if cursor.x<1 then cursor.x=1 end

		texteditor.master.throb=0
		texteditor.scroll_widget.pan:set_dirty()

		local cx = cursor.x - lines.px
		local cy = cursor.y - lines.py
		
		local pan=texteditor.scroll_widget.pan
		
		local hx=math.floor(pan.hx/8)  - lines.gutter
		local hy=math.floor(pan.hy/16)
		local dx=0
		local dy=0
		if cx<1 then dx=cx-1 elseif cx>hx then dx=cx-hx end
		if cy<1 then dy=cy-1 elseif cy>hy then dy=cy-hy end
		
		if dx~=0 or dy~=0 then -- scroll

print(dx,dy,hx,hy)
		
			if dx~=0 then texteditor.scroll_widget.datx:inc(dx*8) end
			if dy~=0 then texteditor.scroll_widget.daty:inc(dy*16) end

		end


	end

	cursor.insert=function(s)
	
		local sa=texteditor.lines.get_string(cursor.y) or ""
		local sb=sa:sub(0,cursor.x-1)
		local sc=sa:sub(cursor.x)
		
		texteditor.lines.set_string(cursor.y,sb..s..sc)
	
		cursor.x=cursor.x+1
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()
	end


	return cursor
end

function wtexteditor.area(widget)
	local area={}


	return area
end


function wtexteditor.mouse(pan,act,_x,_y,key)

	if pan.master.old_over==pan and pan.parent.daty and pan.parent.daty.class=="number" then
		if key=="wheel_add" and act==-1 then
			pan.parent.daty:dec()
			return
		elseif key=="wheel_sub" and act==-1  then
			pan.parent.daty:inc()
			return
		end
	end

--print(pan,key)

	return pan.meta.mouse(pan,act,_x,_y,keyname)
end


function wtexteditor.key(pan,ascii,key,act)
--print("gotkey",ascii,act)

	local texteditor=pan.texteditor
	local cursor=texteditor.cursor



	if ascii and ascii~="" then -- not a blank string

		local c=string.byte(ascii)
		
		if c>=32 and c<128 then
		
			cursor.insert(ascii)

		end
		
	elseif act==1 or act==0 then
	
		if key=="left" then

			cursor.x=cursor.x-1
			cursor.moved()
						
		elseif key=="right" then
				
			cursor.x=cursor.x+1
			cursor.moved()

		elseif key=="up" then

			cursor.y=cursor.y-1
			cursor.moved()

		elseif key=="down" then

			cursor.y=cursor.y+1
			cursor.moved()

		end
		
	end

	return true

end

function wtexteditor.setup(widget,def)

	widget.class="texteditor"
	
	widget.update=wtexteditor.update
	widget.layout=wfill.layout
	widget.draw=wtexteditor.draw
	widget.refresh=wtexteditor.refresh

-- internal functions
	widget.texteditor_refresh	=	wtexteditor.texteditor_refresh
	widget.texteditor_hooks		=	function(act,w) return wtexteditor.texteditor_hooks(widget,act,w) end

	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",size="full",scroll_pan="tiles"})
	
	widget.lines  = wtexteditor.lines(widget)  -- lines of text
	widget.cursor = wtexteditor.cursor(widget) -- cursor location and mode
	widget.area   = wtexteditor.area(widget)   -- selection area


	widget.scroll_widget.pan.pan_refresh=function(pan) return widget:texteditor_refresh() end -- we will do the scroll


	widget.scroll_widget.pan.skin=wtexteditor.pan_skin( widget.scroll_widget.pan.skin )
	widget.scroll_widget.pan.texteditor=widget

--	wtexteditor:redo_text(def.text or "") -- set starting text

	widget:texteditor_refresh()

	widget.scroll_widget.pan.solid=true
	widget.scroll_widget.pan.can_focus=true

	widget.scroll_widget.pan.key=wtexteditor.key
	widget.scroll_widget.pan.mouse=wtexteditor.mouse


	return widget
end


	return wtexteditor
end
