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

function wtexteditor.update(texteditor)

	local txt=texteditor.txt

	if texteditor.txt_dirty then
		texteditor.master.throb=255
		texteditor.txt_dirty=false
		texteditor:texteditor_refresh()
		texteditor.master.request_layout=true
		texteditor.scroll_widget.pan:set_dirty()
	end

	local throb=(texteditor.master.throb>=128)
	if throb ~= texteditor.throb then -- dirty throb...
		texteditor.throb=throb
		texteditor.scroll_widget.pan:set_dirty()
	end
	
	if texteditor.key_mouse and not texteditor.master.mouse_left then -- catch mouse up nomatter where it was
	
		texteditor.float_cx=nil

		texteditor.key_mouse=false

		if texteditor.mark_area then
			txt.mark(unpack(texteditor.mark_area))
		end
		
		texteditor:scroll_to_view()
		texteditor:refresh()
		texteditor:set_dirty()

	end

	return texteditor.meta.update(texteditor)
end

function wtexteditor.pan_skin( oldskin )
	return function(pan)
		local panmtx=oven.gl.SaveMatrix() -- need to cache the current matrix for later draw call
		local panskin=oldskin(pan)
		return function()
			panskin()

			if pan.texteditor.throb then -- draw the blinking cursor
			
				local cx = pan.texteditor.txt.cx - pan.texteditor.cx
				local cy = pan.texteditor.txt.cy - pan.texteditor.cy
				

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

		widget.gutter=#string.format(" %d   ",widget.hy)

		local pan=widget.scroll_widget.pan
		pan.hx_max=(widget.txt.hx+widget.gutter+1)*8
		pan.hy_max=widget.txt.hy*16
	
	
		if widget.data then -- auto set data if txt changes
			local sa=widget.txt.get_text()
			if sa ~= widget.data:value() then
				widget.data:value(sa)
			end
		end

		widget.txt_dirty=true
	end

--print(act,w and w.id)
end

wtexteditor.texteditor_refresh=function(widget)

	local pan=widget.scroll_widget.pan
	local txt=widget.txt

	pan.lines={}
	
	local cx=math.floor(pan.pan_px/8)
	local cy=math.floor(pan.pan_py/16)

	widget.cx=cx -- remember the scroll positions in characters
	widget.cy=cy

	pan.background_tile=0xcc0000 -- default tile

	for y=cy+1,cy+256 do
		local ps={}
		local pl=0

		local cache=widget.txt.get_cache(y)
		if cache then

			local vn=tostring(y)
			vn=string.rep(" ",widget.gutter-3-#vn)..vn.." "
			for i=1,#vn do
				if pl>=256*3 then break end -- max width
				ps[pl+1]=string.byte(vn,i,i)
				ps[pl+2]=0
				ps[pl+3]=0xde
				pl=pl+3
			end
			
			ps[pl+1]=32
			ps[pl+2]=0
			ps[pl+3]=0xce -- (y%16)*16
			pl=pl+3
			
			ps[pl+1]=32
			ps[pl+2]=0
			ps[pl+3]=0xce
			pl=pl+3

			for x=cx,cx+256 do
				if pl>=256*3 then break end -- max width
				local i=cache.xc[x]
				if not i then break end -- max width
				local code=cache.codes[i]
				ps[pl+1]=code or 32
				ps[pl+2]=0
				ps[pl+3]=0xce
				if txt.fx and txt.fy and txt.tx and txt.ty then
					local flip=false
					if     y==txt.fy and y==txt.ty then if i>=txt.fx and i< txt.tx then flip=true end -- single line
					elseif y==txt.fy               then if i>=txt.fx               then flip=true end -- first line
					elseif y==txt.ty               then if i< txt.tx               then flip=true end -- last line
					elseif y>txt.fy  and y<txt.ty  then                                 flip=true end -- middle line
					if flip then ps[pl+3] = 0xd0 + ps[pl+3]%16  --[[math.floor(ps[pl+3]/16) + (ps[pl+3]%16)*16]] end
				end
				pl=pl+3
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[y-cy]={text=v,s=s}
	end

end

function wtexteditor.mouse(pan,act,_x,_y,keyname)

	if pan.meta.mouse(pan,act,_x,_y,keyname) then return end -- let children have precidence

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
		if keyname=="wheel_add" and act==-1 then
--			wheel_acc()
			pan.parent.daty:dec(16*4)
			return
		elseif keyname=="wheel_sub" and act==-1  then
--			wheel_acc()
			pan.parent.daty:inc(16*4)
			return
		end
	end
--print(pan,act,pan.master.over==pan)
--	if key=="wheel_add" or key=="wheel_sub" then return end

--print(pan,key)


	local texteditor=pan.texteditor
	local txt=texteditor.txt
	local px=-math.floor(pan.pan_px/8)
	local py=math.floor(pan.pan_py/16)
	
	local x,y=pan:mousexy(_x,_y)
	local dx,dy=math.floor(x/8),math.floor(y/16)
	
	dx=dx-texteditor.gutter+1
	dy=dy+1
	
	if act==1 and texteditor.master.over==pan and keyname=="left" then -- click to activate
	
		texteditor.float_cx=nil

		texteditor.key_mouse=true

		texteditor.mark_area={dx,dy,dx,dy}

		txt.mark(unpack(texteditor.mark_area))

		texteditor:scroll_to_view()
		texteditor:refresh()
		texteditor:set_dirty()

	elseif act==0 and texteditor.key_mouse then -- drag, but only while over widget

		if texteditor.key_mouse and texteditor.mark_area then

			texteditor.float_cx=nil

			texteditor.mark_area[3],texteditor.mark_area[4]=dx,dy
			
			txt.mark(unpack(texteditor.mark_area))

			texteditor:scroll_to_view()
			texteditor:refresh()
			texteditor:set_dirty()
		end
	
	end

end

function wtexteditor.scroll_to_view(texteditor,cx,cy)
	local txt=texteditor.txt
	local pan=texteditor.scroll_widget.pan
	
	cx=cx or txt.cx
	cy=cy or txt.cy
	
	local dx=cx-texteditor.cx
	local dy=cy-texteditor.cy
	
	local hx=math.floor(texteditor.scroll_widget.pan.hx/8)
	local hy=math.floor(texteditor.scroll_widget.pan.hy/16)
	
	if dy<4 then

		local d=-(dy-4)
--		print("dec",d)
		pan.parent.daty:dec(16*d)

	elseif dy>hy-4 then

		local d=(dy-hy+4)
--		print("inc",d)
		pan.parent.daty:inc(16*d)

	end
	
	
end

function wtexteditor.key(pan,ascii,key,act)
--print("gotkey",ascii,act,key)

	local texteditor=pan.texteditor
	local txt=texteditor.txt


	local cpre=function()
		if texteditor.key_shift then
			if not texteditor.mark_area then
				texteditor.mark_area={txt.cx,txt.cy,txt.cx,txt.cy}
			end
		else
			texteditor.mark_area=false
			txt.mark()
		end
	end
	local cpost=function()
		if texteditor.key_shift and texteditor.mark_area then
				texteditor.mark_area[3]=txt.cx
				texteditor.mark_area[4]=txt.cy
				txt.mark(unpack(texteditor.mark_area))
		end
		
		texteditor:scroll_to_view()
	end


	if ascii and ascii~="" then -- not a blank string

		texteditor.txt_dirty=true

		local c=string.byte(ascii)
		
		if c>=32 and c<128 then
		
			texteditor.float_cx=nil

			txt.insert_char(ascii)
			texteditor:scroll_to_view()

		end
		
	elseif act==1 or act==0 then

		texteditor.txt_dirty=true

		if     key=="shift_l"   or key=="shift_r"   then	texteditor.key_shift=true
		elseif key=="control_l" or key=="control_r" then	texteditor.key_control=true
		elseif key=="alt_l"     or key=="alt_r"     then	texteditor.key_alt=true
		elseif key=="left" then

			texteditor.float_cx=nil

			cpre()
			txt.cx,txt.cy=txt.clip_left(txt.cx,txt.cy)
			cpost()

		elseif key=="right" then
			
			texteditor.float_cx=nil

			cpre()
			txt.cx,txt.cy=txt.clip_right(txt.cx,txt.cy)
			cpost()

		elseif key=="up" then
		
			texteditor.float_cx=texteditor.float_cx or txt.cx

			cpre()
			txt.cx,txt.cy=txt.clip_up(texteditor.float_cx,txt.cy)
			cpost()

		elseif key=="down" then

			texteditor.float_cx=texteditor.float_cx or txt.cx

			cpre()
			txt.cx,txt.cy=txt.clip_down(texteditor.float_cx,txt.cy)
			cpost()

		elseif key=="enter" or key=="return" then

			texteditor.float_cx=nil

			txt.insert_newline()
			texteditor:scroll_to_view()

		elseif key=="home" then

			texteditor.float_cx=nil

			txt.cx=1
			txt.clip()
			texteditor:scroll_to_view()
		
		elseif key=="end" then
				
			texteditor.float_cx=nil

			txt.cx=txt.get_hx()+1
			txt.clip()
			texteditor:scroll_to_view()

		elseif key=="back" then

			texteditor.float_cx=nil

			txt.backspace()
			texteditor:scroll_to_view()

		elseif key=="delete" then

			texteditor.float_cx=nil

			txt.delete()
			texteditor:scroll_to_view()

		elseif key=="tab" then

			texteditor.float_cx=nil

			txt.insert_char("\t")
			texteditor:scroll_to_view()

		end
		
	elseif act==-1 then

		if     key=="shift_l"   or key=="shift_r"   then	texteditor.key_shift=false
		elseif key=="control_l" or key=="control_r" then	texteditor.key_control=false
		elseif key=="alt_l"     or key=="alt_r"     then	texteditor.key_alt=false
		end
		
	end

	return true

end

function wtexteditor.layout(widget)

	widget.scroll_widget.hx=widget.hx
	widget.scroll_widget.hy=widget.hy

	return widget.meta.layout(widget)
end


function wtexteditor.setup(widget,def)

	widget.class="texteditor"
	
	widget.update=wtexteditor.update
	widget.layout=wtexteditor.layout
	widget.draw=wtexteditor.draw
	widget.refresh=wtexteditor.refresh

-- internal functions
	widget.texteditor_refresh	=	wtexteditor.texteditor_refresh
	widget.texteditor_hooks		=	function(act,w) return wtexteditor.texteditor_hooks(widget,act,w) end
	widget.scroll_to_view		=	wtexteditor.scroll_to_view


	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",size="full",scroll_pan="tiles",color=widget.color})
	
	widget.txt=require("wetgenes.txt").construct()
	
	widget.txt.hooks.changed=function(txt) return wtexteditor.texteditor_hooks(widget,"txt_changed") end

	widget.gutter=0
	widget.ducts=0

	widget.px=0
	widget.py=0

	widget.scroll_widget.pan.pan_refresh=function(pan) return widget:texteditor_refresh() end -- we will do the scroll

	widget.scroll_widget.pan.skin=wtexteditor.pan_skin( widget.scroll_widget.pan.skin )
	widget.scroll_widget.pan.texteditor=widget

--	wtexteditor:redo_text(def.text or "") -- set starting text

	widget:texteditor_refresh()

	widget.scroll_widget.pan.solid=true
	widget.scroll_widget.pan.can_focus=true

	widget.scroll_widget.pan.key=wtexteditor.key
	widget.scroll_widget.pan.mouse=wtexteditor.mouse

	widget.gutter=#(" 01   ")
	
	if def.data then -- set starting text
		widget.txt.set_text( def.data:value() )
	end
	
	return widget
end


	return wtexteditor
end
