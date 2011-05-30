package.cpath=wetlua.cpath -- and set paths so we can find things
package.path=wetlua.path

local wetquire=require("wetquire")
wetquire.overload() -- replace require and module

local debug=debug
local xpcall=xpcall
local assert=assert

--package.path =package.path.. ";./lua/?.lua;./lua/?/init.lua"
--package.cpath=package.cpath..";./lua/?.so;./lua/init.dll"


if not main_next or main_next=="" then main_next="menu" end

local work=require('work')

local bit=require('bit')
local gl=require('gl')

local widget = require("fenestra.widget")
local np_check_msg=oldmain and oldmain.np_check_msg

function goto(s) main_next=s end -- request a main state change with a goto, oh my.
local goto=goto

-- win is global and contains all state data
win=require('fenestra.wrap').win(wet_setup_hwnd)

local menu

local win=win
local _G=_G
local gcinfo=gcinfo
local string=string
local math=math

local function print(...) _G.print(...) end

local times={}

local function state_change(force)

-- handle state changes

	if main_next or force then
	
		if main and main.clean then
			times.clean.start()
			main:clean()
			times.clean.stop()
		end
		
		if type(main_next)=="string" then
		
			main_name=main_next
			main_next=require("state."..main_next)
		end

		main_last=main
		main=main_next
		main_next=nil
		
		if main and main.setup then
			times.setup.start()
			main:setup()
			times.setup.stop()
		end
		
	end
	
end


module("state.main")


function setup()

	work.lanes_setup() -- create some worker threads

menu_active=false
end_now=false

	win.setup(_G) -- create and associate with this global table, eg _G.print gets replaced


	local last=win.time()
	local frame_last=last
	local frame_count=0
	local fps=0


	local function times_setup()
		local t={}
		t.time=0
		t.time_live=0
		
		t.hash=0
		t.hash_live=0
		
		t.started=0
		
		function t.start()
			t.started=win.time()
		end
		
		function t.stop()
			local ended=win.time()
			
			t.time_live=t.time_live + ended-t.started
			t.hash_live=t.hash_live + 1
		end
		
		function t.done()
			t.time=t.time_live
			t.hash=t.hash_live
			t.time_live=0
			t.hash_live=0
			
		end
		
		return t
	end



	times.update=times_setup()
	times.draw=times_setup()
	times.swap=times_setup()
	times.setup=times_setup()
	times.clean=times_setup()

	win.update=function()

		win.width=win.get("width")
		win.height=win.get("height")

		state_change()	
		
		local t=win.time()
		local d=t-last
		local d_orig=d

	-- count frames	
		if t-frame_last >= 1 then
		
			fps=frame_count
			frame_count=0
			frame_last=t
		
			times.update.done()
			times.draw.done()
			times.swap.done()
		end
		
	-- update

		local do_draw=false
		while d >= 0.020 do
		
			times.update.start()
			
			win.console.update()
				
			if menu_active then
			
				menu:update()
				
			else

				win.widget:update()
				
				if _G.main and _G.main.update then
					_G.main:update()
				end
				
			end
			
			times.update.stop()
			
			if d>1 then -- reset when very out of sync
				last=t
				d=0
			else
				last=last+0.020
				d=d-0.020
			end
			
			do_draw=true
		end

	-- draw


		if do_draw then

			times.draw.start()
			
			if menu_active then
			
				gl.ClearColor(0,0,0.25,0)
				win.begin()
				
				win.clip2d(0,0,0,0)
				win.project23d(640/480,1,1024)
				gl.MatrixMode("MODELVIEW")
				gl.LoadIdentity()
			
				gl.PushMatrix()
				gl.Translate(-320,-240, -240*1.0)
				menu:draw()
				gl.PopMatrix()
				
			else
				local skipwidge
				if _G.main and _G.main.draw then
					skipwidge=_G.main:draw()
				else
					gl.ClearColor(0,0,0.25,0)
					win.begin()
					
					win.clip2d(0,0,0,0)
					win.project23d(640/480,1,1024)
					gl.MatrixMode("MODELVIEW")
					gl.LoadIdentity()
				end
				
				if not skipwidge then
					win.clip2d(0,0,0,0)
					win.project23d(640/480,1,1024) -- undo any changes
					gl.MatrixMode("MODELVIEW")
					gl.PushMatrix()
					gl.Translate(-320,-240, -240*1.0)
					win.widget:draw()
					gl.PopMatrix()
				end
			end
			
			win.console.draw()
		
			win.swap()
			times.draw.stop()
			
			frame_count=frame_count+1
			
			local gci=gcinfo()
			win.console.display(string.format("fps=%02.0f t=%03.0f u=%03.0f d=%03.0f gc=%0.0fk",fps,math.floor(0.5+(10000/fps)),math.floor(0.5+times.update.time*10000),math.floor(0.5+times.draw.time*10000/times.draw.hash),math.floor(gci) ))
			
		end

	end

-- create a simple master menu

	

	menu=widget.setup(win,{font=win.font_debug})

-- call update once before setting the mouse or key functions

	win.update()

local hooks={}
	function hooks.click(widget)
print(widget.id)
		if widget.id then
			if widget.id=="continue" then
				menu_active=false
			elseif widget.id=="menu" then
				menu_active=false
				goto("menu")
			elseif widget.id=="reload" then
				wetquire.set_reload_time() -- ask for reload
				goto(_G["main_name"])
				menu_active=false
			elseif widget.id=="quit" then
				end_now=true
				menu_active=false
			end
		end
	end
	local top=menu:add({hx=640,hy=480,mx=1,class="hx",ax=0,ay=0})
	top:add({sy=4,sx=1})
	top:add({text="Continue",color=0x8800ff00,id="continue",hooks=hooks})
	top:add({text="Main Menu",color=0x88ffff00,id="menu",hooks=hooks})
	top:add({text="Reload",color=0x88ff8800,id="reload",hooks=hooks})
	top:add({text="Quit",color=0x88ff0000,id="quit",hooks=hooks})
	top:add({sy=4,sx=1})
	
	menu:layout()
	
	menu.state="ready"


-- setup mouse and key handlers
	
	function win.keypress(ascii,key,act)
	
		key=string.lower(key)
	
		if act=="up" and (key=="esc" or key=="escape") then
			menu_active=not menu_active
		end
	
			
		if not win.console.keypress(ascii,key,act) then -- console didnt want it
		
			if _G.main and _G.main.keypress then
			
				_G.main.keypress(ascii,key,act)
			
			end
		end
		
	end


-- transform mouse data to widget view and tell the widgets
	function win.mouse(act,x,y,key)

		local hx=win.width/2
		local hy=win.height/2

		local tx,ty
		
		if win.height/(win.width or 1) > (3/4) then -- deal with new smart viewport sizeing
		
			tx=(4/3)*(x-hx)/hx
			ty=(4/3)*(hy-y)/hx
			
		else
		
			tx=(x-hx)/hy
			ty=(hy-y)/hy

		end
		
		if menu_active then
			
			menu:mouse(act,320+tx*240,240+ty*240,key)
		else
			win.widget:mouse(act,320+tx*240,240+ty*240,key)
			
			
			if _G.main and _G.main.mouse then
			
				_G.main.mouse(act,x,win.height-y,key)
			
			end
		end
		
	end
	
	
end


function update()

	while win.msg("wait") do

		if np_check_msg then
			local si,sv=np_check_msg()
			if si and sv then
				if si=="state" then
					if sv=="quit" then
						end_now=true
					else
						goto(sv)
					end
				end
			end
		end
		
--		print("lanes_update")
		work.lanes_update()
		win.update()
		
		if end_now then break end
	end
	
end


function clean()

	menu:remove_all()
	
	-- cleanup any state
	state_change(true)


	-- display final setup/clean timers
	times.setup.done()
	times.clean.done()
	print("setup:"..times.setup.time.." / "..times.setup.hash.." = "..(times.setup.time/times.setup.hash))
	print("clean:"..times.clean.time.." / "..times.clean.hash.." = "..(times.clean.time/times.clean.hash))

	win.clean()

	work.lanes_clean()

end





					
					

-- start? only if main_next is already set

if _G.main_next then

	local function f()
		setup()
		update()
		clean()
	end
	
	assert( xpcall(f,debug.traceback) )
	
end
