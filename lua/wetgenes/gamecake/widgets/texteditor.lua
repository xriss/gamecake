--
-- (C) 2019 Kriss@XIXs.com
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

	if widget.txt_dirty then
		widget.master.throb=255
		widget.txt_dirty=false
		widget:texteditor_refresh()
		widget.master.request_layout=true
		widget.scroll_widget.pan:set_dirty()
	end

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
			
				local cx = pan.texteditor.txt.cx - pan.texteditor.px
				local cy = pan.texteditor.txt.cy - pan.texteditor.py
				

				oven.gl.PushMatrix()
				oven.gl.LoadMatrix(panmtx)

				if cx>=1 and cx<=pan.hx/8 and cy>=1 and cy<=pan.hy/16 then -- visible cursor

					local x,y,hx,hy=pan.px,pan.py,pan.hx,pan.hy

					x=x+(cx-1+pan.texteditor.gutter)*8-2
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

	if act=="txt_changed" then
	
		widget.gutter=#string.format(" %d  ",widget.hy)

		widget.txt_dirty=true
	end

--print(act,w and w.id)
end

wtexteditor.texteditor_refresh=function(widget)

	local strings=widget.txt.strings or {}

	local pan=widget.scroll_widget.pan
	local txt=widget.txt

	pan.lines={}
	
	local px=-math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)

	widget.px=px -- remember the scroll positions in characters
	widget.py=py

	for y=py+1,py+256 do
		local ps={}
		local pl=0

		local v=tostring(y)
		v=string.rep(" ",widget.gutter-3-#v)..v.." "
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
				if txt.fx and txt.fy and txt.tx and txt.ty then
					local flip=false
					if     y==txt.fy and y==txt.ty then if i>=txt.fx and i< txt.tx then flip=true end -- single line
					elseif y==txt.fy               then if i>=txt.fx               then flip=true end -- first line
					elseif y==txt.ty               then if i< txt.tx               then flip=true end -- last line
					elseif y>txt.fy  and y<txt.ty  then                                 flip=true end -- middle line
					if flip then ps[pl+3] = math.floor(ps[pl+3]/16) + (ps[pl+3]%16)*16 end
				end
				pl=pl+3
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[y-py]={text=v,s=s}
	end

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
	local txt=texteditor.txt
	local px=-math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)
	
	local x,y=pan:mousexy(_x,_y)
	local dx,dy=math.floor(x/8),math.floor(y/16)
	
	dx=dx-texteditor.gutter+1
	dy=dy+1

	if texteditor.master.over==pan.parent or act==-1 then

		if act==1 then

			pan.area_click={dx,dy,dx,dy}

			texteditor:refresh()
			texteditor:set_dirty()

		elseif act==0 then -- drag

			if pan.area_click then
				pan.area_click[3],pan.area_click[4]=dx,dy
				
				txt.mark(unpack(pan.area_click))

				texteditor:refresh()
				texteditor:set_dirty()
			end
		
		elseif act==-1 and pan.area_click then -- final
		
			txt.mark(unpack(pan.area_click))
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
	local txt=texteditor.txt

	if ascii and ascii~="" then -- not a blank string

		texteditor.txt_dirty=true

		local c=string.byte(ascii)
		
		if c>=32 and c<128 then
		
			txt.insert_char(ascii)

		end
		
	elseif act==1 or act==0 then

		texteditor.txt_dirty=true

		if key=="left" then

			if txt.cx<=1 and txt.cy>1 then
				txt.cy=txt.cy-1
				txt.cx=txt.get_hx()+1
				txt.clip()
			else
				txt.cx=txt.cx-1
				txt.clip()
			end
						
		elseif key=="right" then
			
			local hx=txt.get_hx()+1
			if txt.cx>=hx and txt.cy<txt.hy then
				txt.cy=txt.cy+1
				txt.cx=1
				txt.clip()
			else
				txt.cx=txt.cx+1
				txt.clip()
			end

		elseif key=="up" then

			txt.cy=txt.cy-1
			txt.clip()

		elseif key=="down" then

			txt.cy=txt.cy+1
			txt.clip()

		elseif key=="enter" or key=="return" then

			txt.insert_newline()

		elseif key=="home" then

			txt.cx=1
			txt.clip()
		
		elseif key=="end" then
				
			txt.cx=txt.get_hx()+1
			txt.clip()

		elseif key=="back" then

			txt.backspace()

		elseif key=="delete" then

			txt.delete()

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
	
	widget.txt=require("wetgenes.txt").construct()
	
	widget.txt.hooks.changed=function(txt) return wtexteditor.texteditor_hooks(widget,"txt_changed") end

	widget.gutter=0
	widget.ducts=0

	widget.px=0
	widget.py=0

--[[
	wtexteditor.lines(widget,widget.lines) -- lines of text
	wtexteditor.cursor(widget,widget.cursor) -- cursor location and mode
	wtexteditor.area(widget,widget.area) -- selection area
]]



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
