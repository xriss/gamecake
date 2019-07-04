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
	local area=widget.area

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
				if area.fx and area.fy and area.tx and area.ty then
					local flip=false
					if     y==area.fy and y==area.ty then  if i>=area.fx and i< area.tx then flip=true end -- single line
					elseif y==area.fy                then  if i>=area.fx                then flip=true end -- first line
					elseif y==area.ty                then  if i< area.tx                then flip=true end -- last line
					elseif y>area.fy  and y<area.ty  then                                    flip=true end -- middle line
					if flip then ps[pl+3] = math.floor(ps[pl+3]/16) + (ps[pl+3]%16)*16 end
				end
				pl=pl+3
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[y-py]={text=v,s=s}
	end

end

function wtexteditor.lines(texteditor,lines)

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

	lines.add_string=function(idx,str)
		table.insert( lines.strings , idx , str)
		lines.hy=lines.hy+1
	end

	lines.del_string=function(idx)
		table.remove( lines.strings , idx )
		lines.hy=lines.hy-1
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

function wtexteditor.cursor(texteditor,cursor)
	
	local lines=texteditor.lines

	cursor.x=1
	cursor.y=1
	
	cursor.get_hx=function(y)
		y=y or cursor.y
		local s=lines.get_string(y)
		local hx=s and #s or 0
		while hx>0 do
			local endswith=s:byte(hx)
			if endswith==10 or endswith==13 then hx=hx-1
			else break end -- ignore any combination of CR or LF at end of line
		end
		if hx > lines.hx then lines.hx=hx end -- fix max
		return hx
	end

	cursor.clip=function(x,y)

		if y>lines.hy then y=lines.hy end
		if y<1 then y=1 end

		local hx=cursor.get_hx(y)

		if x>hx+1 then x=hx+1 end
		if x<1 then x=1 end

		return x,y
	end

	cursor.moved=function()
	
		cursor.x,cursor.y=cursor.clip(cursor.x,cursor.y)

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
		
			if dx~=0 then texteditor.scroll_widget.datx:inc(dx*8) end
			if dy~=0 then texteditor.scroll_widget.daty:inc(dy*16) end

		end


	end

	cursor.insert=function(s)
	
		local sa=lines.get_string(cursor.y) or ""
		local sb=sa:sub(0,cursor.x-1)
		local sc=sa:sub(cursor.x)
		
		lines.set_string(cursor.y,sb..s..sc)
	
		cursor.x=cursor.x+1
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()
	end

	cursor.newline=function()
	
		local sa=lines.get_string(cursor.y) or ""
		local sb=sa:sub(0,cursor.x-1) or ""
		local sc=sa:sub(cursor.x) or ""
		
		lines.set_string(cursor.y,sb.."\n")

		cursor.y=cursor.y+1
		cursor.x=1

		lines.add_string(cursor.y,sc)
		
		lines.changed_lines()
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()
	
	end

	cursor.merge_lines=function()

		local sa=lines.get_string(cursor.y) or ""
		cursor.y=cursor.y-1
		local hx=cursor.get_hx()
		local sb=lines.get_string(cursor.y) or ""
		cursor.x=hx+1
		lines.set_string(cursor.y,sb:sub(1,hx)..sa)
		lines.del_string(cursor.y+1)
		lines.changed_lines()
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()
		
	end

	cursor.backspace=function()
		if cursor.x==1 and cursor.y>1 then
			cursor.merge_lines()
			return
		end

		local sa=lines.get_string(cursor.y) or ""
		local sb=sa:sub(0,cursor.x-2)
		local sc=sa:sub(cursor.x)
		
		lines.set_string(cursor.y,sb..sc)
	
		cursor.x=cursor.x-1
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()

	end

	cursor.delete=function()
		local hx=cursor.get_hx()
		if cursor.x==hx+1 and cursor.y<lines.hy then
			cursor.y=cursor.y+1
			cursor.merge_lines()
			return
		end

		local sa=lines.get_string(cursor.y) or ""
		local sb=sa:sub(0,cursor.x-1)
		local sc=sa:sub(cursor.x+1)
		
		lines.set_string(cursor.y,sb..sc)
	
		cursor.moved()
	
		texteditor:refresh()
		texteditor:set_dirty()
	end

	return cursor
end

function wtexteditor.area(texteditor,area)

	local cursor=texteditor.cursor

	area.mark=function(fx,fy,tx,ty)
		if not fx then -- unmark
			area.fx=nil
			area.fy=nil
			area.tx=nil
			area.ty=nil
			return
		end
		area.fx,area.fy=cursor.clip(fx,fy)
		area.tx,area.ty=cursor.clip(tx,ty)
		
		local flip=false
		if area.fy==area.ty and area.fx>area.tx then flip=true
		elseif                  area.fy>area.ty then flip=true end
		if flip then
			area.fx,area.tx=area.tx,area.fx
			area.fy,area.ty=area.ty,area.fy
		end

-- print( area.fx , area.fy , area.tx , area.ty )

	end

	return area
end


function wtexteditor.mouse(pan,act,_x,_y,key)

	local wheel_acc=function()

		pan.wheel_speed=pan.wheel_speed or 1
		pan.wheel_stamp=pan.wheel_stamp or 0

		local t=os.time()
		if pan.wheel_stamp+1 > t then
			pan.wheel_speed=pan.wheel_speed+1
		else
			pan.wheel_speed=1
		end
		pan.wheel_stamp=t
	end
	
	if pan.master.old_over==pan and pan.parent.daty and pan.parent.daty.class=="number" then
		if key=="wheel_add" and act==-1 then
--			wheel_acc()
			pan.parent.daty:dec(16*4)
			return
		elseif key=="wheel_sub" and act==-1  then
--			wheel_acc()
			pan.parent.daty:inc(16*4)
			return
		end
	end

--print(pan,key)


	local texteditor=pan.texteditor
	local area=texteditor.area
	local px=-math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)
	
	local x,y=pan:mousexy(_x,_y)
	local dx,dy=math.floor(x/8),math.floor(y/16)
	
	dx=dx-texteditor.lines.gutter+1
	dy=dy+1

	if texteditor.master.over==pan.parent or act==-1 then

		if act==1 then

			pan.area_click={dx,dy,dx,dy}

			texteditor:refresh()
			texteditor:set_dirty()

		elseif act==0 then -- drag

			if pan.area_click then
				pan.area_click[3],pan.area_click[4]=dx,dy
				
				area.mark(unpack(pan.area_click))

				texteditor:refresh()
				texteditor:set_dirty()
			end
		
		elseif act==-1 and pan.area_click then -- final
		
			area.mark(unpack(pan.area_click))
			pan.area_click=false

			texteditor:refresh()
			texteditor:set_dirty()
		end
	end

	return pan.meta.mouse(pan,act,_x,_y,keyname)
end


function wtexteditor.key(pan,ascii,key,act)
--print("gotkey",ascii,act)

	local texteditor=pan.texteditor
	local cursor=texteditor.cursor
	local lines=texteditor.lines



	if ascii and ascii~="" then -- not a blank string

		local c=string.byte(ascii)
		
		if c>=32 and c<128 then
		
			cursor.insert(ascii)

		end
		
	elseif act==1 or act==0 then

--print(key)

		if key=="left" then

			if cursor.x<=1 and cursor.y>1 then
				cursor.y=cursor.y-1
				cursor.x=cursor.get_hx()+1
				cursor.moved()
			else
				cursor.x=cursor.x-1
				cursor.moved()
			end
						
		elseif key=="right" then
			
			local hx=cursor.get_hx()+1
			if cursor.x>=hx and cursor.y<lines.hy then
				cursor.y=cursor.y+1
				cursor.x=1
				cursor.moved()
			else
				cursor.x=cursor.x+1
				cursor.moved()
			end

		elseif key=="up" then

			cursor.y=cursor.y-1
			cursor.moved()

		elseif key=="down" then

			cursor.y=cursor.y+1
			cursor.moved()

		elseif key=="enter" or key=="return" then

			cursor.newline()

		elseif key=="home" then

			cursor.x=1
			cursor.moved()
		
		elseif key=="end" then
				
			cursor.x=cursor.get_hx()+1
			cursor.moved()

		elseif key=="back" then

			cursor.backspace()

		elseif key=="delete" then

			cursor.delete()

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
	
	widget.lines  = {} -- pre init so we can reference each other in the setup functions
	widget.area   = {}
	widget.cursor = {}

	wtexteditor.lines(widget,widget.lines) -- lines of text
	wtexteditor.cursor(widget,widget.cursor) -- cursor location and mode
	wtexteditor.area(widget,widget.area) -- selection area


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
