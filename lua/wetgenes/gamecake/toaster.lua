--
-- (C) 2023 Kriss@XIXs.com
--

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require



local log,dump=require("wetgenes.logs"):export("log","dump")


--[[#lua.wetgenes.gamecake.toaster

	oven=require("wetgenes.gamecake.toaster").bake(opts)

A cut down oven without opengl or even file access intended to be used 
in a sub process or task.

]]

local function assert_resume(co)
	local aa={ coroutine.resume(co) }
	if aa[1] then return unpack(aa) end -- no error
	error( tostring(aa[2]).."\nin coroutine\n"..debug.traceback(co) ) -- error
end


local M={ modname=(...) } ; package.loaded[M.modname]=M

--[[#lua.wetgenes.gamecake.toaster.newticks

	ticks=require("wetgenes.gamecake.toaster").newticks(rate,jump)

create a ticks time controller for updates.

rate is time between steps , jump is how far behind we should get 
before giving up ( eg callback takes longer than rate allows ) these 
values are in seconds and default to 1/60 and 1 respectively.

Then we perform one or more update steps like so

	ticks.step( callback ) -- callback may be called zero or more times

After that we should wait a bit (maybe draw a frame) and then call 
again.

]]
M.newticks=function(rate,jump)
	-- get time now in seconds
	local getnow=require("lanes").now_secs

	local ticks={}
	
	ticks.jump=jump or 1 -- if we get this far behind then jump forwards
	ticks.rate=rate or 1/60 -- update rate
	ticks.time=getnow()
	ticks.count=0

	-- return true if we callback at least once
	ticks.step=function(callback)
		local now=getnow()
		local done=false
		while now-ticks.time >= ticks.rate do -- step forward
			if now-ticks.time >= ticks.jump then -- jump forward skipping updates
				ticks.time=now
			else
				ticks.time=ticks.time+ticks.rate
			end
			ticks.count=ticks.count+1
			done=true
			callback() -- run callback
		end
		return done
	end

	return ticks
end

--[[#lua.wetgenes.gamecake.toaster.bake

	oven=wetgenes.gamecake.toaster.bake(opts)

Bake creates an instance of a lua module bound to an oven state. Here 
we are creating the main state that other modules will then bind to.

Modules are then bound together using rebake...

	b=oven.rebake(nameb)
	c=oven.rebake(namec)

All of these will be connected by the same oven and circular 
dependencies should work with the caveat that just because you have the 
table for a baked module does not mean that it has all been filled in 
yet.

]]
function M.bake(opts)

	local oven={}
	
	oven.newticks=M.newticks

	oven.is={}

	oven.opts=opts or {}
	
	oven.baked={}

	oven.tasks=require("wetgenes.tasks").create({linda=opts.linda})

--
-- preheat a normal oven
-- you may perform this yourself if you want more oven control
--
	function oven.preheat()
		oven.next=opts.start
		return oven
	end

-- require and bake oven.baked[modules] in such a way that it can have simple circular dependencies

	function oven.rebake(name)

		local ret=oven.baked[name]
		
		if not ret then
			ret={modname=name}
			oven.baked[name]=ret -- need to create and remember here so we can always rebake even if the result is not filled in yet
			ret=assert(require(name)).bake(oven,ret)
		end

		return ret
	end

	function oven.change(oven_next)
		if oven_next then oven.next=oven_next end
		if oven.next then
		
			oven.clean()
			
			if type(oven.next)=="string" then	 -- change by required name
			
				oven.next=oven.rebake(oven.next)
				
			elseif type(oven.next)=="boolean" then -- special exit oven
			
				oven.last=oven.now
				oven.next=nil
				oven.now=nil
				
			end

			if oven.next then
				oven.last=oven.now
				oven.now=oven.next
				oven.next=nil
				
				oven.setup()
			end
			
		end		
	end


-- simply call into now if the function exists

	function oven.setup()	
		if oven.now and oven.now.setup then return oven.now.setup() end
	end

	function oven.clean()
		if oven.now and oven.now.clean then return oven.now.clean() end
	end

	function oven.update()
		if oven.now and oven.now.update then return oven.now.update() end
	end

	function oven.draw() 
		if oven.now and oven.now.draw then return oven.now.draw() end
	end

	function oven.msgs()
		if oven.now and oven.now.msgs then return oven.now.msgs() end
	end




-- a busy blocking loop
	function oven.serv(oven)
		local finished
		repeat
			finished=oven.serv_pulse(oven)
		until finished
	end

-- a single step
	function oven.serv_pulse(oven)
		oven.msgs()
		oven.update()
		oven.draw()
	end
		
	return oven
end
