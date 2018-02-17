--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--local wwin=require("wetgenes.win")
local tardis=require("wetgenes.tardis")
local bit=require("bit")

-- widget class master
-- the master widget

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmaster)
wmaster=wmaster or {}

local gl=oven.gl
local cake=oven.cake
local canvas=oven.canvas

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local skeys=oven.rebake("wetgenes.gamecake.spew.keys")
	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")

local mkeys=oven.rebake("wetgenes.gamecake.mods.keys")


local wdatas=oven.rebake("wetgenes.gamecake.widgets.datas")



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

	master.active_xy={0,0}

-- create or reuse datas interface
	master.datas=master.datas or wdatas.new_datas({master=master})



-- built in color themes, 

	master.color_theme_bright={ { 0.00, 0.00, 0.00 },{ 0.60, 0.60, 0.60 },{ 1.00, 1.00, 1.00 }, text=0, scale=1, }
	master.color_theme_dark  ={ { 0.00, 0.00, 0.00 },{ 0.30, 0.30, 0.30 },{ 1.00, 1.00, 1.00 }, text=2, scale=1, }

-- global GUI color theme

	master.color_theme=master.color_theme_bright
	master.color_theme=master.color_theme_dark

-- get a color from a theme and optionally apply a tint
	function master.get_color(val,tint)
	
		local t=master.color_theme

		if not val then val=t.text end -- text color
	
		local c={}

		val=((val-1)*t.scale)+1 -- theme scale intensity

		if val<0 then val=0 end -- clamp
		if val>2 then val=2 end

		if val<1 then
			for i=1,3 do c[i]=t[1][i]*(1-val) + t[2][i]*(val) end -- blend down
		elseif val>=1 then
			for i=1,3 do c[i]=t[2][i]*(2-val) + t[3][i]*(val-1) end -- blend up
		end
		
--		if c[4]<=0 then c[4]=1 end -- fix possible divide by zero?
--		for i=1,3 do c[i]=c[i]/c[4] end -- normalise
		
		if tint then
			if type(tint)=="number" then -- convert from 0xAARRGGBB
				local r,g,b,a	
				a=bit.band(bit.rshift(tint,24),0xff)
				r=bit.band(bit.rshift(tint,16),0xff)
				g=bit.band(bit.rshift(tint, 8),0xff)
				b=bit.band(tint,0xff)
				tint={r/0xff,g/0xff,b/0xff,a/0xff}
			end
			local v=tint[4]
			if v<0 then v=0 end -- clamp
			if v>1 then v=1 end
			for i=1,3 do c[i]=c[i]*(1-v) + tint[i]*(v) end -- skip alpha
		end

		for i=1,3 do if c[i]<0 then c[i]=0 end if c[i]>1 then c[i]=1 end end -- clamp result
		
		c[4]=1 -- full alpha only
	
		return c
	end


-- the master gets some special overloaded functions to do a few more things
	function master.update(widget,resize)

		
		local ups=srecaps.ups()
		if ups then -- use skeys / srecaps code 


			if master.focus then
				skeys.set_opts("typing",true)
			else
				skeys.set_opts("typing",false)
			end
			
			if not master.press then -- do not move when button is held down
				local vx=0
				local vy=0
				if ups.button("left_set")  then vx=-1 end
				if ups.button("right_set") then vx= 1 end
				if ups.button("up_set")    then vy=-1 end
				if ups.button("down_set")  then vy= 1 end
				master.keymove(vx,vy)
			end

			if ups.button("fire_set")  then

				master.press=true

				if master.over then

					if master.active~=master.over then
					
						if master.active then
							master.active:call_hook_later("inactive") -- this widget is no longer active
						end

						master.active=master.over
						local axis=ups.axis()
						local rx,ry=master.over.parent:mousexy(axis.mx,axis.my)
						master.active_xy={rx-master.over.px,ry-master.over.py,mx=axis.mx,my=axis.my}
						
						master.active:call_hook_later("active") -- an active widget is about to click (button down)
					end

					if master.active and master.active.can_focus then
						master.set_focus(master.active)
					end

					master.over:set_dirty()

				end

			end
			
			if ups.button("fire_clr")  then

				master.press=false

				if master.over --[[ and master.active==master.over ]] then -- no click if we drag away from button
				
					if not master.over.never_set_focus_edit then
--						print("click",master.edit,master.focus)
						master.set_focus_edit(master.over)
						master.set_focus(master.over)
					end
					if ups.button("mouse_left_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_left"}) -- its a left click
					elseif ups.button("mouse_right_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_right"}) -- its a right click
					elseif ups.button("mouse_middle_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_middle"}) -- its a middle click
					else
						master.over:call_hook_later("click") -- probably not a mouse click
					end
					
					master.over:set_dirty()
					
				end
				
				if master.active then
					master.active:call_hook_later("inactive") -- this widget is no longer active
				end

				master.active=nil
				
			end

		end

-- loop over and call all later function then empty the table
-- later functions can add more functions as they are called
		if true then
			local call_later=function(c,...) return c(...) end
			repeat
				local v=table.remove(master.later,1)
				if v then
					if type(v[1])~="function" then dprint(tostring(v[1])) end
					call_later(unpack(v))
				end
			until not v
		end
--		master.later={}
		
		local tim=os.time()
		for w,t in pairs(master.timehooks) do
			if t<=tim then
				w:call_hook("timedelay",t)
				master.timehooks[w]=nil
			end
		end
	
		if resize then
--print("resize",wstr.dump(resize))
			if widget.hx==resize.hx and widget.hy==resize.hy then
			else
				widget.hx=resize.hx or widget.hx
				widget.hy=resize.hy or widget.hy
				widget:set_dirty()
				widget:resize_and_layout()
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
		
--		if widget.dirty then widget:resize_and_layout() print("dirty master") end -- a dirty master forces a layout

		meta.update(widget)
		
		if master.over and master.over~=master then
			master.cursor=master.over.cursor
		else
			master.cursor=nil
		end
		
	end
	
	function master.resize_and_layout(widget)
--print("master layout")
		meta.resize(widget)
		meta.layout(widget)
		meta.build_m4(widget)
		master.remouse(widget)
--exit()
	end
	master.layout=master.resize_and_layout

	local dirty_fbos={}
	local mark_dirty_fbos -- to recurse is defined...
	local find_dirty_fbos -- to recurse is defined...

	mark_dirty_fbos=function(widget)
		if widget.fbo then
			if widget.fbo.w~=widget.hx or widget.fbo.h~=widget.hy then -- resize so we need a new fbo
--print("resize fbo",widget.hx,widget.hy)
				widget.fbo:resize(widget.hx,widget.hy,widget.hz or 0)
				widget:set_dirty() -- flag redraw
			end				
		end
		for i,v in ipairs(widget) do
			mark_dirty_fbos(v)
		end
	end


	find_dirty_fbos=function(widget)
		if widget.fbo and ( widget.dirty or widget.fbo.dirty ) then
			widget.fbo.dirty=nil
			widget.dirty=true
			dirty_fbos[ #dirty_fbos+1 ]=widget
		end
		for i,v in ipairs(widget) do
			find_dirty_fbos(v)
		end
	end

	function master.draw(widget)

		dirty_fbos={}
		mark_dirty_fbos(widget)
		find_dirty_fbos(widget)

		gl.Disable(gl.CULL_FACE)
		gl.Disable(gl.DEPTH_TEST)

		gl.PushMatrix()
		
		
		if #dirty_fbos>0 then
			for i=#dirty_fbos,1,-1 do -- call in reverse so sub fbos can use their child fbo data
				dirty_fbos[i]:draw() -- dirty, so this only draws into the fbo
			end
		end

		meta.draw(widget)
		
		gl.PopMatrix()
		
	end
	
	function master.msg(widget,m)
	
		if m.class=="text" then
			if skeys.opts.typing or m.softkey then -- fake keyboard only
				widget:key(m.text)
			end
		elseif m.class=="key" then
			if skeys.opts.typing or m.softkey then -- fake keyboard only
				widget:key(nil,m.keyname,m.action)
			end
		elseif m.class=="mouse" then
			widget:mouse(m.action,m.x,m.y,m.keyname)
		end

	end

	function master.set_focus_edit(edit)
		if master.edit==edit then return end -- no change

		if master.edit then
			master.edit:call_hook_later("unfocus_edit")
			master.edit=nil
		end

		if edit then -- can set to nil
			if edit.class=="textedit" then -- also set edit focus
				widget.edit=edit
				master.edit:call_hook_later("focus_edit")
			end
		end
	end
	
	function master.set_focus(focus)
--print("focus",tostring(focus))
		if master.focus==focus then return end -- no change
	
		if master.focus then
			master.focus:call_hook_later("unfocus")
			master.focus=nil
		end

		if focus then -- can set to nil
			if focus.can_focus then
				master.focus=focus
				master.focus:call_hook_later("focus")
				if focus.class=="textedit" then -- also set edit focus
					master.set_focus_edit(focus)
				end
			end
		end
	
	end
--
-- handle key input
--
	function master.key(widget,ascii,key,act)

		if master.focus then -- key focus, steals all the key presses until we press enter again
		
			if master.focus.key then
				master.focus:key(ascii,key,act)
			end
			
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
		
		end

	end


	function master.keymove(vx,vy)
		if vx~=0 or vy~=0 then -- move hover selection

-- print(vx,vy)
		
			if master.over and master.over.hx and master.over.hy then
				local over=master.over
				local best={}

				local ox,oy=over:get_master_xy(over.hx/2,over.hy/2)
				
				master:call_descendents(function(w)
					if w.solid and w.hooks and not w.hidden then
						local wx,wy=w:get_master_xy(w.hx/2,w.hy/2)
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
					if master.over then master.over:call_hook_later("over") end
				end
			end

			if not master.over then
				master:call_descendents(function(v)
					if not master.over then
						if v.solid and v.hooks and not v.hidden then
							master.over=v
							v:set_dirty()
							master.over:call_hook_later("over")
						end
					end
				end)
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

		master.last_mouse_position={x,y}

		local old_active=master.active
		local old_over=master.over

		meta.mouse(widget,act,x,y,keyname) -- cascade down into all widgets
		
		if master.dragging() then -- handle mouse drag logic
			master.active:drag(x,y)

		end
		
--mark as dirty
		if master.active~=old_active then
			if master.active then master.active:set_dirty() end
			if old_active then old_active:set_dirty() end
		end
		if master.over~=old_over then
			if master.over then master.over:set_dirty() end
			if old_over then old_over:set_dirty() end
			if master.over then master.over:call_hook_later("over") end
		end
		
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
		master.later={}
		master.later_append=function(f,...) assert(type(f)=="function") master.later[#master.later+1]={f,...} end
	end
	
	function master.dragging()

		if master.active and master.active.drag and master.press then
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
				if master.over then master.over:call_hook_later("over") end
			end
		end)
		
	end
	
--
-- mark all widgets that reference this data as dirty
--
	function master.dirty_by_data(data)
		master:call_descendents(function(w)
			if w.data==data then
				w:set_dirty()
			end
		end)		
	end

end

return wmaster
end
