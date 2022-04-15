--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local function print(...) _G.print(...) end
local function dprint(a) print(require("wetgenes.string").dump(a)) end

local wwin=require("wetgenes.win")
local wstring=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wutf=require("wetgenes.txt.utf")
local wjson=require("wetgenes.json")

local _,lfs=pcall( function() return require("lfs") end ) ; lfs=_ and lfs

local ls=function(...) print(wstring.dump({...})) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,wtexteditor)

	wtexteditor=wtexteditor or {} 
	wtexteditor.modname=M.modname
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local wfill=oven.rebake("wetgenes.gamecake.widgets.fill")

function wtexteditor.update(texteditor)

	local txt=texteditor.txt

	if texteditor.last_refresh_pan_hx ~= texteditor.scroll_widget.pan.hx then texteditor.txt_dirty=true end

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
		texteditor.mark_area=nil
		
		texteditor:scroll_to_view()
		texteditor.txt_dirty=true

	end


	return texteditor.meta.update(texteditor)
end

function wtexteditor.pan_skin( oldskin )
	return function(pan)
		local panmtx=oven.gl.SaveMatrix() -- need to cache the current matrix for later draw call
		local panskin=oldskin(pan)
		return function()
			panskin()

			local cache=pan.texteditor.txt and pan.texteditor.txt.get_cache( pan.texteditor.txt.cy )
			if pan.texteditor.throb and cache then -- draw the blinking cursor


				oven.gl.PushMatrix()
				oven.gl.LoadMatrix(panmtx)


				local cx=pan.texteditor.cursor_cx
				local cy=pan.texteditor.cursor_cy
				if cx and cy and cx>=1 and cx<=pan.hx/8 and cy>=1 and cy<=pan.hy/16 then -- visible cursor

					local x,y,hx,hy=pan.px,pan.py,pan.hx,pan.hy

					x=x+(cx-1+pan.texteditor.gutter)*8-2
					y=y+(cy-1)*16
					hx=2
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
end


wtexteditor.texteditor_hooks=function(widget,act,w)

	if act=="txt_changed" then

		if widget.opts.gutter_disable then
			widget.gutter=0
		else
			widget.gutter=#string.format(" %d   ",widget.hy)
		end
		
		local pan=widget.scroll_widget.pan
		pan.hx_max=(widget.txt.hx+widget.gutter+2)*8
		pan.hy_max=widget.txt.hy*16
		
		if widget.opts.word_wrap then pan.hx_max=0 end -- no x scroll when word wrapping
	
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


wtexteditor.texteditor_refresh_swed=function(widget,swed)

	local tokefind=function(i,c,p)
		if c.tokens and c.string then
			local start=1
			local idx=1
			while true do
				local s,e=string.find(c.tokens,p,start)
				if not s then break end
				if idx==i then return c.cb[s],c.cb[e] end
				idx=idx+1
				start=e+1
			end
		end
	end


	if swed.mode=="+" then
		swed.fakeline=4 -- hide text and leave space for this many lines
	end
	
	local sx=math.floor((widget.sx or 1)*8)
	local sy=math.floor((widget.sy or 1)*16)
	widget.over.text_size=sy


	local hx=widget.hx-8
	local gx=0
	if not widget.opts.gutter_disable then
		gx=widget.gutter*sx
	end
	local px=gx
	local py=(swed.idx-widget.cy-1)*sy
							
	if not swed.data then -- reuse old data
		swed.data={}
		swed.data.hook_slide=function(hook,data)
			if hook=="value" then
				local n=data:value()
				local cy=swed.idx+1
				local c=widget.txt.get_cache( cy )
				local fb,tb=tokefind(data.user,c,"0+")
				if fb and tb then
					local sn=string.format("%.2f",n)
					if not string.find(sn,"%.") then sn=sn.."." end -- must have a .
					widget.txt.tweak_string(cy,fb,tb,sn)
					widget.txt_dirty=true
				end
			end
		end
		swed.data.hook_show=function(hook,data)
			if hook=="value" then
				local oldmode=swed.mode
				if data:value()==0 then swed.mode="-" else swed.mode="+" end
				if oldmode~=swed.mode then
					local c=widget.txt.get_cache( swed.idx )
					local xe=#c.string
					local es=wjson.encode(swed.config,{sort=true,white=" "})
					widget.txt.tweak_string(swed.idx,7,xe, swed.mode .. es .. widget.txt.endline )
					widget.txt_dirty=true
				end
			end
		end
		swed.data.show=wdata.new_data({max=1,min=0,num=(swed.mode=="+") and 1 or 0,step=1,master=widget.master,hooks=swed.data.hook_show})
		for i=1,4 do
		swed.data[i]=wdata.new_data({user=i,max=1,min=0,num=0,step=0.01,master=widget.master,hooks=swed.data.hook_slide})
		end
	end
	
-- basic container widgets
	swed.wgutter = widget.over:add{class="fill",hx=gx,   hy=sy*4,px=0, py=py}
	swed.wtext   = widget.over:add{class="fill",hx=hx-px,hy=sy*4,px=px,py=py}
	
	swed.wgutter:add{hx=gx-sx*2,hy=sy*1}
	swed.wgutter:add{class="checkbox",hx=sx*2,hy=sy*1,color=0,text_false="+",text_true="-",data=swed.data.show}

	if swed.mode=="+" then
		for i=1,4 do
		swed.wtext:add{class="slide",hx=sx*32,hy=sy*2,color=0,datx=swed.data[i],data="datx"}
		end
	end
	
	widget.over:layout()

--	dump(pan)
end


wtexteditor.texteditor_refresh=function(widget)

	widget.over:clean_all()

	widget.cursor_cx=nil
	widget.cursor_cy=nil

	local pan=widget.scroll_widget.pan
	local txt=widget.txt

	widget.last_refresh_pan_hx=pan.hx

	pan.lines={}
	
	local cx=math.floor(pan.pan_px/8)
	local cy=math.floor(pan.pan_py/16)

	widget.cx=cx -- remember the scroll positions in characters
	widget.cy=cy

	local cursor_x = 0
	local cursor_y = 0
	local cache=pan.texteditor.txt.get_cache( pan.texteditor.txt.cy )
	if cache then
		cursor_x = 1 + ( cache.cx[pan.texteditor.txt.cx] or 0 )
		cursor_y = pan.texteditor.txt.cy
	end

	pan.background_tile=0x00010000 -- default tile, remember it is abgr so backwards

	local wy=1
	for y=cy+1,cy+256 do
		local ps={}
		local pl=0
		local sx=cx
		local fakeline
		local cache=widget.txt.get_cache_lex(y)
		if cache then

			if cache.swed then
				wtexteditor.texteditor_refresh_swed(widget,cache.swed)
				fakeline=cache.swed.fakeline
			end

			if not widget.opts.gutter_disable then

				local vn=tostring(y)
				vn=string.rep(" ",widget.gutter-3-#vn)..vn.." "
				for i=1,#vn do
					if pl>=512*3 then break end -- max width
					ps[pl+1]=string.byte(vn,i,i)
					ps[pl+2]=0
					ps[pl+3]=3
					ps[pl+4]=2
					pl=pl+4
				end
				
				ps[pl+1]=32
				ps[pl+2]=0
				ps[pl+3]=1
				ps[pl+4]=0
				pl=pl+4
				
				ps[pl+1]=32
				ps[pl+2]=0
				ps[pl+3]=1
				ps[pl+4]=0
				pl=pl+4

			end


			if fakeline then

				for i=2,fakeline do
				
					if pl==0 then
						if not widget.opts.gutter_disable then
							for i=1,widget.gutter do
								ps[pl+1]=32
								ps[pl+2]=0
								ps[pl+3]=1
								ps[pl+4]=0
								pl=pl+4
							end
						end
					end
				
					local s=string.char(unpack(ps))
					pan.lines[wy]={text=v,s=s,y=y,x=sx}
					wy=wy+1

					ps={}
					pl=0

				end
			else
			for x=cx,cx+512 do

				if cursor_x-1 == x and cursor_y == y then
					widget.cursor_cx=x-sx+1
					widget.cursor_cy=wy
				end

				if pl>=512*3 then break end -- max width
				local i=cache.xc[x]
				if not i then break end -- max width
				local code=cache.codes[i]
				local toke=cache.tokens and string.sub(cache.tokens,i,i)

				code=wutf.map_unicode_to_latin0[code] or code or 127
				if code<32 then code=32 end -- control codes are space
				if  (code>127 and code<128+32) or code>255 then code=127 end -- missing glyphs are 127

				ps[pl+1]=code
				ps[pl+2]=0
				ps[pl+3]=1
				ps[pl+4]=0
				
				if     toke=="k" then	ps[pl+3]=6  -- keyword
				elseif toke=="g" then	ps[pl+3]=7  -- global
				elseif toke=="c" then	ps[pl+3]=8  -- comment
				elseif toke=="s" then	ps[pl+3]=9  -- string
				elseif toke=="0" then	ps[pl+3]=10 -- number
				elseif toke=="p" then	ps[pl+3]=11 -- punctuation
				end

				
				if txt.fx and txt.fy and txt.tx and txt.ty then
					local flip=false
					if     y==txt.fy and y==txt.ty then if i>=txt.fx and i< txt.tx then flip=true end -- single line
					elseif y==txt.fy               then if i>=txt.fx               then flip=true end -- first line
					elseif y==txt.ty               then if i< txt.tx               then flip=true end -- last line
					elseif y>txt.fy  and y<txt.ty  then                                 flip=true end -- middle line
					if flip then ps[pl+3],ps[pl+4] = ps[pl+4],ps[pl+3] end
				end
				pl=pl+4

				if widget.opts.word_wrap then
					local pmax=(math.floor(pan.hx/8)-1)*4
					if pmax > (widget.gutter*4) and pl > pmax then -- wrap
					
						local s=string.char(unpack(ps))
						pan.lines[wy]={text=v,s=s,y=y,x=sx}
						wy=wy+1

						ps={}
						pl=0
						sx=x+1

						if not widget.opts.gutter_disable then
							for i=1,widget.gutter do
								ps[pl+1]=32
								ps[pl+2]=0
								ps[pl+3]=1
								ps[pl+4]=0
								pl=pl+4
							end
						end

					end
				end

			end
			end
		end
		local s=string.char(unpack(ps))
		pan.lines[wy]={text=v,s=s,y=y,x=sx}
		wy=wy+1
	end

end

function wtexteditor.mouse(pan,act,_x,_y,keyname)

	if pan.meta.mouse(pan,act,_x,_y,keyname) then -- let children have precedence
		return
	end

-- ignore clicks when we are not focused?
--[[
	print(pan.master.focus == pan,pan.master.active == pan,pan.master.over == pan)
	if pan.master.focus ~= pan and pan.master.active ~= pan then
		return true
	end
]]	
	if keyname=="right" and act==1 then
		log("texteditor","righty clicky")
		pan.master.later_append(function()
			log("texteditor","righty clicky later")
		end)
		return
	end
	
	if pan.master.old_over==pan and pan.parent.daty and pan.parent.daty.class=="number" then
		if keyname=="wheel_add" and act==-1 then
			pan.parent.daty:dec(16*4)
			return
		elseif keyname=="wheel_sub" and act==-1  then
			pan.parent.daty:inc(16*4)
			return
		end
	end

	local texteditor=pan.texteditor
	local txt=texteditor.txt	
	local x,y=pan:mousexy(_x,_y)

	local dx,dy=math.floor(x/8-0.5),math.floor(y/16)
	
	dx=dx-texteditor.gutter+1-texteditor.cx
	dy=dy+1-texteditor.cy

	local line=pan.lines[ dy ]
	
	if line then
		dy=line.y
		dx=line.x+dx
	end
	
	local cache=txt.get_cache( dy )
	if cache then
		if dx > #cache.xc then dx=#cache.xc end
		dx=cache and cache.xc[dx] or 0
	else
		dx=0
	end
	
	if act==1 and texteditor.master.over==pan and keyname=="left" then -- click to activate
	
		texteditor.float_cx=nil

		texteditor.key_mouse=1

		texteditor.mark_area={dy,dx,dy,dx}

		txt.mark(unpack(texteditor.mark_area))

		texteditor:scroll_to_view()
		texteditor.txt_dirty=true

	elseif (act and act>1) and texteditor.master.over==pan and keyname=="left" then -- double click
	
		texteditor.key_mouse=act

		texteditor.float_cx=nil

		txt.markauto(dy,dx,act) -- select word

		texteditor.mark_area={txt.markget()}
		texteditor.mark_area_auto={txt.markget()}

		texteditor:scroll_to_view()
		texteditor.txt_dirty=true

	elseif act==0 and texteditor.key_mouse then -- drag, but only while over widget

		if texteditor.key_mouse and texteditor.mark_area then
		
			texteditor.float_cx=nil

			if texteditor.key_mouse > 1 then -- special select
			
				txt.markauto(dy,dx,texteditor.key_mouse) -- select word

				txt.markmerge(texteditor.mark_area_auto[1],texteditor.mark_area_auto[2],
				texteditor.mark_area_auto[3],texteditor.mark_area_auto[4],txt.markget())

				texteditor.mark_area={txt.markget()}
			else

				texteditor.mark_area[3],texteditor.mark_area[4]=dy,dx
				
				txt.mark(unpack(texteditor.mark_area))
				
			end

			texteditor:scroll_to_view()
			texteditor.txt_dirty=true
		end
	
	end

end

function wtexteditor.scroll_to_bottom(texteditor)
	local d=texteditor.scroll_widget.daty
	d:set(d.max or 1)
end

function wtexteditor.scroll_to_view(texteditor,cy,cx)
	local txt=texteditor.txt
	local pan=texteditor.scroll_widget.pan
	
	cx=cx or txt.cx
	cy=cy or txt.cy
	
	local dx=cx-texteditor.cx
	local dy=cy-texteditor.cy
	
	local hx=math.floor(texteditor.scroll_widget.pan.hx/8)-texteditor.gutter+1
	local hy=math.floor(texteditor.scroll_widget.pan.hy/16)
	
	if dy<4 then

		local d=-(dy-4)
--		print("dec",d)
		pan.parent.daty:dec(16*d)

	elseif dy>hy-3 then

		local d=(dy-hy+3)
--		print("inc",d)
		pan.parent.daty:inc(16*d)

	end

	local edge=math.floor(hx/3)
	if edge>8 then edge=8 end
	if edge<1 then edge=1 end
	if dx<edge+1 then
		pan.parent.datx:dec(8*(edge+1-dx))
	elseif dx>hx-edge then
		pan.parent.datx:inc(8*(dx-(hx-edge)))
	end
	
end

function wtexteditor.key(pan,ascii,key,act)
--print("gotkey",ascii,act,key)

	local master=pan.master
	local texteditor=pan.texteditor
	local txt=texteditor.txt


	local cpre=function()
		if master.key_shift then
			if not texteditor.mark_area then
				texteditor.mark_area={txt.cy,txt.cx,txt.cy,txt.cx}
			end
		else
			texteditor.mark_area=false
			txt.mark()
		end
	end
	local cpost=function()
		if master.key_shift and texteditor.mark_area then
				texteditor.mark_area[3]=txt.cy
				texteditor.mark_area[4]=txt.cx
				txt.mark(unpack(texteditor.mark_area))
		end
		
		texteditor:scroll_to_view()
	end



	if (act==1 or act==0) and ( not ascii or ascii=="" ) then

		if key=="left" then

			texteditor.float_cx=nil

			cpre()
			txt.cy,txt.cx=txt.clip_left(txt.cy,txt.cx)
			cpost()

		elseif key=="right" then
			
			texteditor.float_cx=nil

			cpre()
			txt.cy,txt.cx=txt.clip_right(txt.cy,txt.cx)
			cpost()

		elseif key=="up" then
		
			local cache=txt.get_cache(txt.cy)
			if not texteditor.float_cx then
				texteditor.float_cx = cache and cache.cx[txt.cx] or txt.cx
			end

			cpre()
			local cache=txt.get_cache(txt.cy-1)
			txt.cy,txt.cx=txt.clip( txt.cy-1 , cache and cache.xc[texteditor.float_cx] or texteditor.float_cx )
			cpost()

		elseif key=="down" then

			local cache=txt.get_cache(txt.cy)
			if not texteditor.float_cx then
				texteditor.float_cx = cache and cache.cx[txt.cx] or txt.cx
			end

			cpre()
			local cache=txt.get_cache(txt.cy+1)
			txt.cy,txt.cx=txt.clip( txt.cy+1 , cache and cache.xc[texteditor.float_cx] or texteditor.float_cx )
			cpost()

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

		end
		
	end



	if not texteditor.opts.readonly then -- allow changes

		if ascii and ascii~="" then -- not a blank string

			texteditor.txt_dirty=true
			
			local c=wutf.code(ascii)
			if c>=32 then
			
				texteditor.float_cx=nil

				txt.undo.cut()
				txt.undo.insert_char(ascii)

				texteditor:scroll_to_view()

			end
			
		elseif act==1 or act==0 then

			texteditor.txt_dirty=true

			if master.key_control then
			
	--print(key)		
				if key=="c" then	-- copy
				
					local s=txt.undo.copy()

					if s then wwin.set_clipboard(s) end

				elseif key=="x" then	-- cut

					local s=txt.undo.cut()
					
					if s then wwin.set_clipboard(s) end

					texteditor:scroll_to_view()
				
				elseif key=="v" then	-- paste
				
					if wwin.has_clipboard() then -- only if something to paste
						local s=wwin.get_clipboard()
						txt.undo.replace(s)
						texteditor:scroll_to_view()
					end

				elseif key=="z" then	-- undo

						txt.undo.undo()

				elseif key=="y" then	-- redo

						txt.undo.redo()

				end
			
			elseif key=="enter" or key=="return" then

				texteditor.float_cx=nil

				txt.undo.cut()
				txt.undo.insert_newline()
				texteditor:scroll_to_view()

			elseif key=="back" then

				texteditor.float_cx=nil

				if not txt.undo.cut() then -- just delete selection?
					txt.undo.backspace()
				end
				
				texteditor:scroll_to_view()

			elseif key=="delete" then

				texteditor.float_cx=nil

				if not txt.undo.cut() then -- just delete selection?
					txt.undo.delete()
				end
				
				texteditor:scroll_to_view()

			elseif key=="tab" then

				texteditor.float_cx=nil

				if txt.marked() then -- select full lines

					local fy,fx,ty,tx=txt.markget()
					if tx>1 then ty=ty+1 end
					txt.mark(fy,0,ty,0)

					if master.key_shift then

						local s=txt.undo.copy()
						local ls=wstring.split_lines(s)
						for i=1,#ls do
							if string.sub(ls[i],1,1)=="\t" then
								ls[i]=string.sub(ls[i],2)
							end
						end
						s=table.concat(ls)

						txt.undo.replace(s)

						txt.mark(fy,0,ty,0)

						texteditor:scroll_to_view()

					else

						local s=txt.undo.copy()
						local ls=wstring.split_lines(s)
						for i=1,#ls do
							ls[i]="\t"..ls[i]
						end
						s=table.concat(ls)

						txt.undo.replace(s)

						txt.mark(fy,0,ty,0)

						texteditor:scroll_to_view()

					end

				else

					txt.undo.insert_char("\t")
					texteditor:scroll_to_view()

				end

			end

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

	widget.over=widget.parent:add({px=widget.px,py=widget.py,hx=widget.hx,hy=widget.hy,size="full",fbo=true})

-- options about how we behave

	local opts=def.opts or {}
	widget.opts={}
	widget.opts.readonly		=	opts.readonly
	widget.opts.gutter_disable	=	opts.gutter_disable
	widget.opts.word_wrap		=	opts.word_wrap

	widget.class="texteditor"
	
	widget.update=wtexteditor.update
	widget.layout=wtexteditor.layout
	widget.draw=wtexteditor.draw
	widget.refresh=wtexteditor.refresh

-- internal functions
	widget.texteditor_refresh	=	wtexteditor.texteditor_refresh
	widget.texteditor_hooks		=	function(act,w) return wtexteditor.texteditor_hooks(widget,act,w) end
	widget.scroll_to_view		=	wtexteditor.scroll_to_view
	widget.scroll_to_bottom		=	wtexteditor.scroll_to_bottom


	widget.scroll_widget=widget:add({hx=widget.hx,hy=widget.hy,class="scroll",size="full",scroll_pan="tiles",color=widget.color})

	widget.scroll_widget.datx.step=8
	widget.scroll_widget.daty.step=16
	widget.scroll_widget.datx.scroll=1
	widget.scroll_widget.daty.scroll=1
	
	widget.set_txt=function(txt)
		if widget.txt then -- remove old hooks
			widget.txt.hooks.changed=nil
		end
		
		widget.txt=txt
		widget.txt.hooks.changed=function(txt) return wtexteditor.texteditor_hooks(widget,"txt_changed") end

		wtexteditor.texteditor_hooks(widget,"txt_changed")
	end

	widget.set_txt( require("wetgenes.txt").construct() )

	

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

	widget.scroll_widget.pan.drag=function()end -- fake drag so we are treated as drag able

--	if not widget.gutter_disable then
--		widget.gutter=#(" 01   ")
--	end

	if def.data then -- set starting text
		widget.txt.set_text( def.data:value() )
	end

-- background foreground colour pairs
	local theme={
		dark={
			0xff444444,0xffaaaaaa,	-- text			0,1
			0xff555555,0xff333333,	-- gutter		2,3
			0xff333333,0xffbbbbbb,	-- hilite		4,5
			0xffdd7733,	-- keyword		0,6
			0xffddaa33,	-- global		0,7
			0xff888888,	-- comment		0,8
			0xff66aa33,	-- string		0,9
			0xff5599cc,	-- number		0,10
			0xff999999,	-- punctuation	0,11
			0xff000000,	-- 	0,12
			0xff000000,	-- 	0,13
			0xff000000,	-- 	0,14
			0xff000000,	-- 	0,15
		},
		lite={
			0xffaaaaaa,0xff444444,	-- text			0,1
			0xff777777,0xff999999,	-- gutter		2,3
			0xffbbbbbb,0xff333333,	-- hilite		4,5
			0xffaa6622,	-- keyword		0,6
			0xffcc9922,	-- global		0,7
			0xff777777,	-- comment		0,8
			0xff559922,	-- string		0,9
			0xff4488bb,	-- number		0,10
			0xff666666,	-- punctuation	0,11
			0xff000000,	-- 	0,12
			0xff000000,	-- 	0,13
			0xff000000,	-- 	0,14
			0xff000000,	-- 	0,15
		},
	}

	local p={}
	for i,v in ipairs(theme.dark) do
		local l=#p
		p[l+1],p[l+2],p[l+3],p[l+4]=pack.argb_pmb4(v)
	end
	widget.scroll_widget.pan.colormap_grd:pixels(0,0,#p/4,1,p)

	return widget
end


	return wtexteditor
end
