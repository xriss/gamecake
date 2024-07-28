--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local wzips=require("wetgenes.zips")
local wcsv=require("wetgenes.csv")

local wwin=require("wetgenes.win")
local tardis=require("wetgenes.tardis")
local bit=require("bit")

-- widget class master
-- the master widget

local wstr=require("wetgenes.string")
local dprint=function(a) print(wstr.dump(a)) end

local wjson=require("wetgenes.json")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,wmaster)
wmaster=wmaster or {}

local gl=oven.gl
local cake=oven.cake
local canvas=oven.canvas

local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

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
	master.new_data=function(dat) return master.datas.new_data(dat) end

	function master.set_theme(master,def)
		if type(def)=="string" then
			def=master.actions[def].json or {}
		end

	-- built in color themes, 

		master.color_theme_bright={ { 0.10, 0.10, 0.10 },{ 0.70, 0.70, 0.70 },{ 1.00, 1.00, 1.00 }, text=0, scale=1, alpha=1, grid_size=40, text_size=24, name="bright", }
		master.color_theme_dark  ={ { 0.00, 0.00, 0.00 },{ 0.30, 0.30, 0.30 },{ 1.00, 1.00, 1.00 }, text=2, scale=1, alpha=1, grid_size=40, text_size=24, name="dark",   }

	-- global GUI color theme
		master.theme={}
		if def.theme=="bright" then
			for n,v in pairs(master.color_theme_bright) do master.theme[n]=v end
		else
			for n,v in pairs(master.color_theme_dark) do master.theme[n]=v end
		end
		if def.text_size then master.theme.text_size=def.text_size end master.text_size=master.theme.text_size
		if def.grid_size then master.theme.grid_size=def.grid_size end master.grid_size=master.theme.grid_size
--print(master.grid_size,debug.traceback())
	end
	master:set_theme(def)

-- get a color from a theme and optionally apply a tint
	function master.get_color(val,tint)
	
		local t=master.theme

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
		
		c[1]=c[1]*t.alpha
		c[2]=c[2]*t.alpha
		c[3]=c[3]*t.alpha
		c[4]=t.alpha -- full alpha only
	
		return c
	end


-- the master gets some special overloaded functions to do a few more things
	function master.update(widget,resize)
--print("update",widget)
		
		local up=oven.ups.up(1)
		if up then
			
			if not master.no_keymove then

				if not master.press then -- do not move when button is held down
					local vx=0
					local vy=0
					if up.button("left_set")  then vx=-1 end
					if up.button("right_set") then vx= 1 end
					if up.button("up_set")    then vy=-1 end
					if up.button("down_set")  then vy= 1 end
					master.keymove(vx,vy)
				end

			end

			if up.button("fire_set")  then

				master.press=true

				if master.over then

					if master.active~=master.over then
					
						if master.active then
							master.active:call_hook_later("inactive") -- this widget is no longer active
						end

						master.active=master.over

						master.active=master.over
						local p=master.last_mouse_position
						if p then
							local rx,ry=master.over.parent:mousexy(p[1],p[2])
							master.active_xy={rx-master.over.px,ry-master.over.py,mx=p[1],my=p[2]}
						end
						
						master.active:call_hook_later("active") -- an active widget is about to click (button down)
					end

					if master.active and master.active.can_focus then
						master.set_focus(master.active)
					else
						master.set_focus(nil)
					end

					master.over:set_dirty()

				end

			end
			
			if up.button("fire_clr")  then

				master.press=false

				if master.over and master.over~=master and master.active==master.over then -- no click if we drag away from button
				
					if not master.over.never_set_focus_edit then
--print("active",master.active,master.active.class)
--print("over",master.over,master.over.class)
--						master.set_focus_edit(master.over)
--						master.set_focus(master.over)

						if master.active and master.active.can_focus then
							master.set_focus(master.active)
						end

					end
					if up.button("mouse_left_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_left"}) -- its a left click
					elseif up.button("mouse_right_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_right"}) -- its a right click
					elseif up.button("mouse_middle_clr")  then
						master.over:call_hook_later("click",{keyname="mouse_middle"}) -- its a middle click
					else
						master.over:call_hook_later("click") -- probably not a mouse click
					end
					
					master.over:set_dirty()
				
				elseif master.over and master.over~=master then -- mouse up but not mouse down ( ie we dragged and released )

					if up.button("mouse_left_clr")  then
						master.over:call_hook_later("release",{keyname="mouse_left"}) -- its a left click
					elseif up.button("mouse_right_clr")  then
						master.over:call_hook_later("release",{keyname="mouse_right"}) -- its a right click
					elseif up.button("mouse_middle_clr")  then
						master.over:call_hook_later("release",{keyname="mouse_middle"}) -- its a middle click
					else
						master.over:call_hook_later("release") -- probably not a mouse click
					end
					
					master.over:set_dirty()

				end
				
				if master.active then
					master.active:call_hook_later("inactive") -- this widget is no longer active
				end

				master.active=nil
				
			end

		end


-- loop over and call all later function
-- later functions can add more functions *as* they are called
-- but these will be called next time around
		if true then
			local later=master.later
			master.later={}
			local call_later=function(c,...) return c(...) end
			for i,v in ipairs(later) do
				if v then
					if type(v[1])~="function" then log("widgets",tostring(v[1])) end
					call_later(unpack(v))
				end
			end
		end
		
		local tim=wwin.time()
		for w,t in pairs(master.timehooks) do
			if t<=tim then
				w:call_hook("timedelay",t)
				master.timehooks[w]=nil
			end
		end
	
		if ( resize and (widget.hx~=resize.hx or widget.hy~=resize.hy) ) or master.request_layout or master.request_refresh then
			master.request_layout=false
			widget.hx=resize and resize.hx or widget.hx
			widget.hy=resize and resize.hy or widget.hy
			widget:resize_and_layout()
		end
		if master.request_refresh then -- redraw and layout
			master.request_refresh=false
			widget:set_dirty()
			widget:call_descendents(function(w) w:set_dirty() end) -- force a redraw
		end
		
		local throb=(widget.throb>=128)
		
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
			local hx=math.ceil(widget.hx)
			local hy=math.ceil(widget.hy)
			if widget.fbo.w~=hx or widget.fbo.h~=hy then -- resize so we need a new fbo
				widget.fbo:resize(hx,hy,widget.hz or 0)
				widget:set_dirty() -- flag redraw
			end				
		end
		for i,v in ipairs(widget) do
			mark_dirty_fbos(v)
		end
	end


	find_dirty_fbos=function(widget)
		if widget.fbo and ( widget.dirty or widget.fbo.dirty ) then
			widget.dirty=true
			dirty_fbos[ #dirty_fbos+1 ]=widget
		end
		for i,v in ipairs(widget) do
			find_dirty_fbos(v)
		end
	end

	function master.draw(widget)

		gl.ActiveTexture( gl.TEXTURE0 )

		dirty_fbos={}
		mark_dirty_fbos(widget)
		find_dirty_fbos(widget)

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

-- handle shift key states and sending of action msgs triggered by keys
		master.keystate_msg(m)
	
		local fo=master.focus and master.focus.msg and master.focus
		if fo and fo~=master then
			fo:msg(m) -- this will catch mouse ups as we lose focus
		end
	
		if m.class=="text" then
			if master.focus or m.softkey then -- fake keyboard only
				widget:key(m.text)
			end
		elseif m.class=="key" then
			if master.focus or m.softkey then -- fake keyboard only
				widget:key(nil,m.keyname,m.action)
			end
		elseif m.class=="mouse" then
			widget:mouse(m.action,m.x,m.y,m.keyname)
		end

		local oo=master.over and master.over.msg and master.over
		if oo and fo~=oo and oo~=master then
			oo:msg(m) -- this will catch most messages and not double send if we are also the focus
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
			wwin.StartTextInput()
		else
			wwin.StopTextInput()
		end
	end
	
	function master.set_focus(focus)
--print("focus",tostring(focus),focus and focus.class)
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
--print("edit focus",tostring(focus),focus and focus.class)
				else
					master.set_focus_edit(nil)
				end
			end
		else
			master.set_focus_edit(nil)
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
--print(widget,act,x,y,keyname)

-- keep mouse state in master	
		if act==1 and keyname then
			master["mouse_"..keyname]=true
		elseif act==-1 and keyname then
			master["mouse_"..keyname]=false
		end
		
		if act==1 then
			if master.last_mouse_click then
				if master.last_mouse_click[1]+0.4 > wwin.time() then -- double click
					if keyname == master.last_mouse_click[2] then -- same key
						local dx=master.last_mouse_click[3] - x
						local dy=master.last_mouse_click[4] - y
						local dd=( dx*dx + dy*dy )
						if dd < 32*32 then -- same place
							act=master.last_mouse_click[5]+1 -- act 2 is double click 3 is triple etc...
						end
					end
				end
			end
			master.last_mouse_click={wwin.time(),keyname,x,y,act}
		end
		master.last_mouse_position={x,y}

		master.old_active=master.active
		master.old_over=master.over

		
		if master.dragging() then -- handle mouse drag logic
			master.active:drag(x,y)
			if master.active.mouse then
				master.active.mouse(master.active,act,x,y,keyname) -- cascade down into all widgets
			end
		else
			master.over=nil
			meta.mouse(widget,act,x,y,keyname) -- cascade down into all widgets
		end
		
--mark as dirty
		if master.active~=master.old_active then
			if master.active     then master.active:set_dirty() end
			if master.old_active then master.old_active:set_dirty() end
		end
		if master.over~=master.old_over then
			if master.over     then master.over:set_dirty() end
			if master.old_over then master.old_over:set_dirty() end
			if master.over     then master.over:call_hook_later("over") end
			if master.edit     then master.edit:call_hook_later("notover") end
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
				master.activate(w)
			end
		end)
		
	end
	function master.activate(w)
		master.over=w
		if w.class=="textedit" then
			master.edit=w
		end
		if master.over then master.over:call_hook_later("over") end
	end
	
--
-- mark all widgets that reference this data as dirty
--
	function master.dirty_by_data(data)
		master:call_descendents(function(w)
			if w.data==data or w.daty==data or w.datx==data then
				w:set_dirty()
			end
		end)		
	end

	function master.get_action(id,user)
		local action=master.actions[id]
		if user and action then action=action[user] end
		return action
	end

-- add a simple message to queue that will trigger action as if its key had been pressed
	function master.push_action_msg(id,user)
		oven.win:push_msg({
			class="action",
			action=1,
			time=os.time(),
			id=id,
			user=user,
		})
	end

	function master.reset_actions()
		master.keys={}
		master.actions={}
	end

	function master.load_actions(new_actions)

		if not new_actions then -- load default if no actions provided
			local filename="lua/wetgenes/gamecake/widgets/actions.csv"
			local text=assert(wzips.readfile(filename),"file not found: "..filename)
			new_actions=wcsv.map(wcsv.parse(text))
		end
		
		for i,v in ipairs(new_actions) do
			if tostring(v.id):sub(1,1)=="#" then -- ignore # comment at start of line assuming first column is id
			else
				if v.id then -- allow quick lookup by id and possibly user
					if v.user then
						master.actions[v.id]=master.actions[v.id] or {}
						master.actions[v.id][v.user]=v
					else
						master.actions[v.id]=v
					end
				end
				if v.key then -- map keys
					for m in v.key:gmatch("[^ ]+") do
						local ks=nil
						for k in m:gmatch("[^+]+") do
							k=k:lower()
							if     k=="alt" then    ks="alt"
							elseif k=="ctrl" then   ks=ks and ks.."_ctrl"  or "ctrl"
							elseif k=="shift" then  ks=ks and ks.."_shift" or "shift"
							elseif k~="" then
								ks=ks or "none"
								master.keys[ks]=master.keys[ks] or {}
								master.keys[ks][k]=v
							end
						end
					end
				end
				if v.json then -- parse json data chunk
--					print(v.json)
					v.json=wjson.decode(v.json)
--					dump(v)
				end
			end
		end

	end

	master.keystate_reset=function()
		master.keystate_alt=false
		master.keystate_ctrl=false
		master.keystate_shift=false
		master.keystate="none"
	end

	master.keystate_update=function()
		local ks=nil
		if master.keystate_alt   then  ks="alt"                          end
		if master.keystate_ctrl  then  ks=ks and ks.."_ctrl"  or "ctrl"  end
		if master.keystate_shift then  ks=ks and ks.."_shift" or "shift" end
		master.keystate=ks or "none"
	end

	master.keystate_msg=function(m)

--[[
		if m.class=="action" then
			local action=master.get_action(m.id,m.user)
			dump(m)
			dump(action)
		end
]]

		if m.class=="key" then
			if     ( m.keyname=="shift" or m.keyname=="shift_l" or m.keyname=="shift_r" ) then
				if     m.action== 1 then master.keystate_shift=true
				elseif m.action==-1 then master.keystate_shift=false end
				master.keystate_update()
			elseif ( m.keyname=="control" or m.keyname=="control_l" or m.keyname=="control_r" ) then
				if     m.action== 1 then master.keystate_ctrl=true
				elseif m.action==-1 then master.keystate_ctrl=false end
				master.keystate_update()
			elseif ( m.keyname=="alt" or m.keyname=="alt_l" or m.keyname=="alt_r" ) then
				if     m.action== 1 then master.keystate_alt=true
				elseif m.action==-1 then master.keystate_alt=false end
				master.keystate_update()
			end
			local name=(m.keyname or ""):lower()
			local action=(master.keys[master.keystate] or {})[name]
			if action then -- add an action message
				oven.win:push_msg({
					class="action",
					action=m.action,
					time=m.time,
					id=action.id,
					user=action.user,
				})
			end
		end

	end

-- auto load default actions, call master.reset_actions() to remove these default actions
	do
		master.reset_actions()
		master.load_actions()
		master.keystate_reset()
	end


end

return wmaster
end
