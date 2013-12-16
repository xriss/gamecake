--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")

-- widget class master
-- the master widget

local wstr=require("wetgenes.string")



--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmaster)
wmaster=wmaster or {}

local gl=oven.gl
local cake=oven.cake
local canvas=oven.canvas

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

local mkeys=oven.rebake("wetgenes.gamecake.mods.keys")

--
-- add meta functions
--
function wmaster.setup(widget,def)

	widget.solid=true -- catch background clicks

	local master=widget
	local meta=widget.meta
--	local win=def.win

--meta.setup(widget,def)

--	widget.hx=0
--	widget.hy=0
--	widget.sx=1
--	widget.sy=1

	master.throb=0
--	master.fbo=_G.win.fbo(0,0,0) -- use an fbo
--	master.fbo=framebuffers.create(0,0,0)
	master.dirty=true

-- the master gets some special overloaded functions to do a few more things
	function master.update(widget,resize)
	
		local tim=os.time()
		for w,t in pairs(master.timehooks) do
			if t<=tim then
--print("tim",t)
				w:call_hook("timedelay",t)
				master.timehooks[w]=nil
			end
		end
	
		if resize then
			if widget.hx==resize.hx and widget.hy==resize.hy then
			else
				widget.hx=resize.hx
				widget.hy=resize.hy
				widget:layout()
			end
		end
	
		local throb=(widget.throb<128)
		
		widget.throb=widget.throb-4
		if widget.throb<0 then widget.throb=255 end
		
		if throb ~= (widget.throb<128) then -- dirty throb...
			local w=widget.focus
			if w and w.class=="textedit" then
				w:set_dirty()
			end
			if w~=widget.edit then
				w=widget.edit
				if w and w.class=="textedit" then
					w:set_dirty()
				end
			end
		end

		meta.update(widget)
	end
	
	function master.layout(widget)
--print("master layout")
		meta.layout(widget)
		meta.build_m4(widget)
		master.remouse(widget)
--exit()
	end

	local dirty_fbos={}
	local find_dirty_fbos -- to recurse is defined...
	find_dirty_fbos=function(widget)
		if widget.fbo and widget.dirty then
			dirty_fbos[ #dirty_fbos+1 ]=widget
		end
		for i,v in ipairs(widget) do
			find_dirty_fbos(v)
		end
	end

	function master.draw(widget)

		dirty_fbos={}
		find_dirty_fbos(widget)

		gl.Disable(gl.CULL_FACE)
		gl.Disable(gl.DEPTH_TEST)

		gl.PushMatrix()
		
		
		if #dirty_fbos>0 then
			for i=#dirty_fbos,1,-1 do -- call in reverse so sub fbos can work
				meta.draw(dirty_fbos[i]) -- dirty, so this only builds the fbo
			end
		end

		meta.draw(widget)
		
		gl.PopMatrix()
	end
	
	function master.msg(widget,m)
--exit()
--print(string.char(27).."[2J"..string.char(27).."[H")
		if m.class=="key" then
			widget:key(m.ascii,m.keyname,m.action)
		elseif m.class=="mouse" then
			widget:mouse(m.action,m.x,m.y,m.keyname)
		elseif m.class=="joystick" then
		
			local joydir=mkeys.joystick_msg_to_key(m)
			
			if master.last_joydir~=joydir then -- only when we change
				if master.last_joydir then -- first clear any previous key
					master.key(widget,"",master.last_joydir,-1)
				end
				master.last_joydir=joydir
				if joydir then
					master.key(widget,"",joydir,1) -- then send any new key
				end
			end

		elseif m.class=="joykey" then
		
			if m.action==1 then master.last_keycode=m.keycode end -- slight debounce hack?

--print(wstr.dump(m))

			if m.keycode==4 or ( (oven.opts.smell=="gamestick") and (m.keycode==97) ) then --back
				if master.go_back_id then
					local v=master.ids and master.ids[master.go_back_id]
					if v then
						if m.action==-1 then
							v:call_hook("click")
						end
					end
				end
			elseif m.keycode==108 then --forward
				if master.go_forward_id then
					local v=master.ids and master.ids[master.go_forward_id]
					if v then
						if m.action==-1 then
							v:call_hook("click")
						end
					end
				end
			else
--print(m.keycode)
				if m.keycode==0 then -- ignore
				else
					if m.action==-1 and master.last_keycode==m.keycode then -- key set
						master.last_keycode=nil
						master.key(widget,"","return",1)
						master.key(widget,"","return",-1)
					end
				end
			end

		end
	end

	function master.set_focus_edit(edit)
		if master.edit==edit then return end -- no change

		if master.edit then
			master.edit:call_hook("unfocus_edit")
			master.edit=nil
		end

		if edit then -- can set to nil
			widget.edit=edit
			master.edit:call_hook("focus_edit")
		end
	end
	
	function master.set_focus(focus)
	
		if master.focus==focus then return end -- no change
	
		if master.focus then
			master.focus:call_hook("unfocus")
			master.focus=nil
		end

		if focus then -- can set to nil
			widget.focus=focus
			master.focus:call_hook("focus")
			if focus.class=="textedit" then -- also set edit focus
				master.set_focus_edit(focus)
			end
		end
	
	end
--
-- handle key input
--
	function master.key(widget,ascii,key,act)

		if master.focus then -- key focus, steals all the key presses until we press enter again
		
			master.focus:key(ascii,key,act)
			
		
		else
			if master.edit then
				if	key=="left" or
					key=="right" or
					key=="up" or
					key=="down" or
					key=="return" then
					-- ignore
				else
					master.edit:key(ascii,key,act)
				end
			end
		
			if act==-1 then
				if key=="space" or key=="return" then

					if master.over and master.over.can_focus then
						master.set_focus(master.over)
					end

					if master.over then
						master.over:call_hook("click")
					end
					return
				end
			
			elseif act==1 then
			
--print(1,master.over)
			
				local vx=0
				local vy=0
				if key=="left"  then vx=-1 end
				if key=="right" then vx= 1 end
				if key=="up"    then vy=-1 end
				if key=="down"  then vy= 1 end
				
				if vx~=0 or vy~=0 then -- move hover selection
				
					if master.over then
						local over=master.over
						local best={}
						local ox=over.pxd+(over.hx/2)
						local oy=over.pyd+(over.hy/2)

--print("over",ox,oy)

						master:call_descendents(function(w)
							if w.solid and w.hooks then
								local wx=w.pxd+(w.hx/2)
								local wy=w.pyd+(w.hy/2)
								local dx=wx-ox
								local dy=wy-oy
								local dd=0
								if vx==0 then dd=dd+dx*dx*8 else dd=dd+dx*dx end
								if vy==0 then dd=dd+dy*dy*8 else dd=dd+dy*dy end
--print(w,wx,wy,dx,dy,by,by)
								if	( dx<0 and vx<0 ) or
									( dx>0 and vx>0 ) or
									( dy<0 and vy<0 ) or
									( dy>0 and vy>0 ) then -- right direction
									
									if best.over then
										if best.dd>dd then -- closer
											best.over=w
											best.dd=dd
										end
									else
										best.over=w
										best.dd=dd
									end
								end
							end
						end)
						if best.over then
							over:set_dirty()
							best.over:set_dirty()
							master.over=best.over
							if master.over then master.over:call_hook("over") end
						end
					end
					if not master.over then
						master:call_descendents(function(v)
							if not master.over then
								if v.solid and v.hooks then
									master.over=v
									v:set_dirty()
									master.over:call_hook("over")
								end
							end
						end)
					end
					
				end
--print(2,master.over)
			end
		end

	end

--
-- set the mouse position to its last position
-- call this after adding/removing widgets to make sure they highlight properly
--	
	function master.remouse(widget)
		local p=widget.last_mouse_position -- or {0,0}
		if p then
			widget.mouse(widget,nil,p[1],p[2],nil)
		end
	end
--
-- handle mouse input
--	
	function master.mouse(widget,act,x,y,keyname)

--		meta.build_m4(widget)

--print()	

--widget.hx=16384
--widget.hy=16384

		master.last_mouse_position={x,y}
	
--		if widget.state=="ready" then
		
--print("active",master.active,master.active and master.active.class,
--master.active and master.active.parent,master.active and master.active.parent.class)
			
--			if master.active then
--				meta.mouse(master.active,0,x,y,keyname)
--			end
			
			local old_active=master.active
			local old_over=master.over
--			for i,v in ipairs(widget) do
				meta.mouse(widget,act,x,y,keyname)
--			end
			
			if master.dragging() then -- slide :)
			
				local w=master.active
				local p=w.parent

--				local x=p.mousex
--				local y=p.mousey

		local rx,ry=w.parent:mousexy(x,y)
		local x,y=rx-master.active_x,ry-master.active_y

				
--				local minx=p.pxd
--				local miny=p.pyd
--				local maxx=p.pxd+p.hx-w.hx
--				local maxy=p.pyd+p.hy-w.hy

				local maxx=p.hx-w.hx
				local maxy=p.hy-w.hy

--print("slide",miny,maxy)
				
				w.px=x
				w.py=y
				
				if w.px<0    then w.px=0 end
				if w.px>maxx then w.px=maxx end
				if w.py<0    then w.py=0 end
				if w.py>maxy then w.py=maxy end
				
--				w.px=w.pxd-p.pxd
--				w.py=w.pyd-p.pyd
				
				if w.parent.snap then
					w.parent:snap()
				end
				
				w:call_hook("slide")
				
				w:set_dirty()
				
				w:layout()
				w:build_m4()


			end

			if act== 1 and (keyname=="left" or keyname=="right") then
				master.press=true
			end
			if act==-1 and (keyname=="left" or keyname=="right") then
				master.press=false
				master.active=nil
			end
			
--mark as dirty
			if master.active~=old_active then
				if master.active then master.active:set_dirty() end
				if old_active then old_active:set_dirty() end
			end
			if master.over~=old_over then
				if master.over then master.over:set_dirty() end
				if old_over then old_over:set_dirty() end
				if master.over then master.over:call_hook("over") end
			end
			
--		end
	end
--

	function master.clean_all(m)
		meta.clean_all(m)
		master.over=nil
		master.active=nil
		master.focus=nil
		master.edit=nil
		master.go_back_id=nil
		master.go_forward_id=nil
		master.ids={}
		master.timehooks={}
	end
	
	function master.dragging()

		if master.active and (master.active.class=="drag") and master.press then
			return true
		end
		
		return false
	end

--
-- Select this widget by id, so we can have a simple default action on each screen if the user hammers buttons
--
	function master.activate_by_id(id)
		master:call_descendents(function(w)
			if w.id==id then
				master.over=w
				if w.class=="textedit" then
					master.edit=w
				end
				if master.over then master.over:call_hook("over") end
			end
		end)
		
	end
	
end

return wmaster
end
