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

	oven.is={}

	oven.opts=opts or {}
	
	oven.baked={}

--
-- preheat a normal oven
-- you may perform this yourself if you want more oven control
--
	function oven.preheat()

		if opts.start then
			if type(opts.start)=="string" then
				oven.next=oven.rebake(opts.start)
			else
				oven.next=opts.start
			end
			oven.main=oven.next
		end
		
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

	function oven.change()
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

	function oven.setup()	
		if oven.now and oven.now.setup then
			oven.now.setup()
		end
	end

	function oven.clean()
		if oven.now and oven.now.clean then
			oven.now.clean()
		end
	end

	oven.ticks=0
	function oven.update()
		oven.ticks=(oven.ticks+1)%0x100000000	-- 32bit update tick counter

		if oven.update_co then -- resume until it dies
			if coroutine.status(oven.update_co)~="dead" then
				assert_resume(oven.update_co) -- run it, may need more than one resume before it finishes
				return
			else
				oven.update_co=nil
			end
		end

		if not oven.update_co then -- create a new co and run it once
			local f=function()
				oven.change()
				if oven.now and oven.now.update then
					oven.now.update()
				end
			end
			oven.update_co=coroutine.create(f)
			assert_resume(oven.update_co)
		end
	end

	

-- no draw?
	function oven.draw() 
		if oven.now and oven.now.draw then
			oven.now.draw()
		end
	end

-- no msgs?
	function oven.msgs()
		if oven.now and oven.now.msgs then
			oven.now.msgs()
		end
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
