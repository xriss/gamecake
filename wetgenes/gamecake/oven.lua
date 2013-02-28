-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")

-- handle a simple oven for win programs,
-- all it does is call other ovens/mods functions.

local function print(...) return _G.print(...) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(opts)

	local oven={}

		oven.opts=opts
		
		oven.baked={}
		oven.mods={}

--
-- preheat a normal oven
-- you may perform this yourself if you want more oven control
--
		function oven.preheat()

			oven.frame_rate=1/opts.fps -- how fast we want to run
			oven.frame_time=0

			local inf={width=opts.width,height=opts.height,title=opts.title}
			local screen=wwin.screen()

			inf.x=(screen.width-inf.width)/2
			inf.y=(screen.height-inf.height)/2

			if wwin.flavour=="raspi" then -- do fullscreen on raspi
				inf.x=0
				inf.y=0
				inf.width=screen.width
				inf.height=screen.height
				inf.dest_width=screen.width
				inf.dest_height=screen.height
				if inf.height>=480*2 then -- ie a 1080 monitor, double the pixel size
					inf.width=inf.width/2
					inf.height=inf.height/2
				end
			end

			oven.win=wwin.create(inf)
			oven.win:context({})

			if opts.show then oven.win:show(opts.show) end
--			oven.win:show("full")
--			oven.win:show("max")

			oven.rebake("wetgenes.gamecake.cake") -- bake the cake in the oven,

			-- the order these are added is important for priority, top of list is lowest priority, bottom is highest.
			oven.rebake_mod("wetgenes.gamecake.mods.escmenu") -- escmenu gives us a doom style escape menu
			oven.rebake_mod("wetgenes.gamecake.mods.console") -- console gives us a quake style tilda console
			oven.rebake_mod("wetgenes.gamecake.mods.keys") -- touchscreen keys and posix keymaping
			oven.rebake_mod("wetgenes.gamecake.mods.mouse") -- auto fake mouse on non windows builds
			oven.rebake_mod("wetgenes.gamecake.mods.layout") -- screen layout options

			if opts.start then
				oven.next=oven.rebake(opts.start)
			end
			
			
			return oven
		end
		


-- require and bake oven.baked[modules] in such a way that it can have simple circular dependencies

		function oven.rebake(name)

			local ret=oven.baked[name]
			
			if not ret then
		
				if type(name)=="function" then -- allow bake function instead of a name
					return name(oven,{})
				end
			
				ret={modname=name}
				oven.baked[name]=ret
				ret=assert(require(name)).bake(oven,ret)
				
			end

			return ret
		end


-- this performs a rebake and adds the baked module into every update/draw function
-- so we may insert extra functionality without having to modify the running app
-- eg a console or an onscreen keyboard
		function oven.rebake_mod(name)
		
			if oven.mods[name] then return oven.mods[name] end -- already setup, nothing else to do
			local m=oven.rebake(name) -- rebake mod into this oven

			oven.mods[name]=m			-- store baked version by its name
			table.insert(oven.mods,m)		-- and put it at the end of the list for easy iteration
			
			m.setup() -- and call setup since it will always be running from now on until it is removed
			
			return m
		end
		

		if opts.times then
			oven.times={}
			function oven.times.create()
				local t={}
				t.time=0
				t.time_live=0
				
				t.hash=0
				t.hash_live=0
				
				t.started=0
				
				function t.start()
					t.started=oven.win and oven.win.time() or 0
				end
				
				function t.stop()
					local ended=oven.win and oven.win.time() or 0
					
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
			oven.times.update=oven.times.create()
			oven.times.draw=oven.times.create()
		end

		function oven.change()

		-- handle oven changes

			if oven.next then
			
				oven.clean()
				
				if type(oven.next)=="string" then	 -- change by required name
				
					oven.next=oven.rebake(oven.next)
					
				elseif type(oven.next)=="boolean" then -- special exit oven
				
					if wwin.hardcore.task_to_back then -- on android there is no quit, only back
						wwin.hardcore.task_to_back()
						oven.next=nil
					else
						return true
					end
					
				end

				if oven.next then
					oven.last=oven.now
					oven.now=oven.next
					oven.next=nil
					
					oven.setup()
				end
				
			end
			
		end

		function oven.setup()	
			if oven.now and oven.now.setup then
				oven.now.setup() -- this will probably load data and call the preloader
			end
			oven.preloader_enabled=false -- disabled preloader after first setup completes
		end

		function oven.start()	
			oven.win:start()
			oven.cake.start()
			oven.cake.canvas.start()
			if oven.now and oven.now.start then
				oven.now.start()
			end
		end

		function oven.stop()
			oven.win:stop()
			oven.cake.stop()
			oven.cake.canvas.stop()
			if oven.now and oven.now.stop then
				oven.now.stop()
			end
		end

		function oven.clean()
			if oven.now and oven.now.clean then
				oven.now.clean()
			end
		end

		function oven.update()

--[[
collectgarbage()
local gci=gcinfo()		
local gb=oven.gl.counts.buffers				
print(string.format("mem=%6.0fk gb=%4d",math.floor(gci),gb))
]]

			local f
			f=function()
			
				if oven.frame_rate and oven.frame_time then --  framerate limiter enabled
					if oven.frame_time<(oven.win:time()-0.5) then oven.frame_time=oven.win:time() end -- prevent race condition
					while (oven.frame_time)>oven.win:time() do oven.win:sleep(0.001) end -- simple frame limit
					oven.frame_time=oven.frame_time+oven.frame_rate -- step frame forward one tick
				end

--print( "UPDATE",math.floor(10000000+(oven.win:time()*1000)%1000000) )

				if oven.times then oven.times.update.start() end
				
				if oven.now and oven.now.update then
					oven.now.update()
				end
				for i,v in ipairs(oven.mods) do
					if v.update then
						v.update()
					end
				end

				if oven.times then oven.times.update.stop() end
				
				if oven.frame_rate and oven.frame_time then --  framerate limiter enabled
					if (oven.frame_time-0.001)<=oven.win:time() then -- repeat until we are ahead of real time
						return f() -- tailcall
					end
				end
				
			end
			
			if not oven.update_co then -- create a new one
				oven.update_co=coroutine.create(f)
			end
			if coroutine.status(oven.update_co)~="dead" then
				assert(coroutine.resume(oven.update_co)) -- run it, may need more than one resume before it finishes
			end
			
		end

		oven.preloader_enabled=true
		function oven.preloader()
			if not oven.preloader_enabled then return end
			if oven.win then
				oven.msgs()
				oven.cake.canvas.draw()
				local p=oven.rebake(opts.preloader or "wetgenes.gamecake.spew.preloader")
				p.setup() -- warning, this is called repeatedly
				p.update()
				p.draw()
				oven.win:swap()
			end
		end

		function oven.draw()
		
			if oven.update_co then
				if coroutine.status(oven.update_co)~="dead" then return end -- draw nothing until it is finished
				oven.update_co=nil -- create a new one next update
			end
		
			oven.cake.canvas.draw()
			
--print( "DRAW",math.floor(10000000+(oven.win:time()*1000)%1000000) )

			if oven.times then oven.times.draw.start() end -- between calls to draw
			
			if oven.now and oven.now.draw then
				oven.now.draw()
			end
			
			for i,v in ipairs(oven.mods) do
				if v.draw then
					v.draw()
				end
			end
						
			if oven.times then oven.times.draw.stop() end -- draw is squify so just use it as the total time

			if oven.win then
				oven.win:swap()
			end
			
		end

		function oven.msgs() -- read and process any msgs we have from win:msg
			if oven.win then
				for m in oven.win:msgs() do

					if m.class=="close" then -- window has been closed so do a shutdown
						oven.next=true
					end

					if m.class=="mouse" and m.x and m.y then	-- need to fix x,y numbers
						m.xraw,m.yraw=m.x,m.y					-- remember original
					end
					
					if m.class=="app" then -- androidy
--print("caught : ",m.class,m.cmd)
						if		m.cmd=="init_window" then
							oven.start()
							oven.paused=false
						elseif	m.cmd=="lost_focus"  then
							oven.paused=true
						elseif	m.cmd=="gained_focus"  then
							oven.paused=false
						elseif	m.cmd=="term_window"  then
							oven.paused=true
							oven.stop()
						end
					end

					for i=#oven.mods,1,-1 do -- run it through the mods backwards, so the topmost layer gets first crack at the msgs
						local v=oven.mods[i]
						if m and v and v.msg then
							m=v.msg(m) -- mods can choose to eat the msgs, they must return it for it to bubble down
						end
					end
					if m and oven.now and oven.now.msg then
						oven.now.msg(m)
					end
				end
			end
		end

-- a busy blocking loop, or not, if we are running on the wrong sort
-- of system it just returns and expects the other functions
-- eg oven.serv_pulse to be called when necesary.
		function oven.serv(oven)
		
			if oven.win.noblock then
				return oven
			end
			
			local finished
			repeat
				finished=oven.serv_pulse(oven)
			until finished
		end
		function oven.serv_pulse(oven)
				if oven.change() then return true end
				oven.msgs()
				
				oven.cake.update()
				if not oven.paused then
					oven.update()
					oven.draw()
				else
					oven.win:sleep(1/10)
				end
				
--				return oven.change()
		end
		

	return oven

end
