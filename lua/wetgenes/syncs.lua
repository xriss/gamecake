-- (C) 2025 Kriss@XIXs.com


--[[#lua.wetgenes.syncs

	local await=require("wetgenes.syncs").wait

Simple coop await style coroutines, probably all you need is the global await function.

This global function will wrap another function in a coroutine and resume it 
repeatedly along with other global coroutines until it returns.

This is really only useful if you have other coroutines or tasks doing 
something useful that you have a way of communicating with. IE use it 
with tasks as a simpler more generic way to wrap a function that may 
yield.

]]


-- M=module
local M={ modname = (...) } ; package.loaded[M.modname] = M


M.syncs={}
M.syncs_metatable={__index=M.syncs}


M.create=function(syncs)
	syncs=syncs or {}
	
	setmetatable(syncs,M.syncs_metatable)
	
	return syncs:setup()
end


M.syncs.setup=function(syncs)

	syncs.list={}
	syncs.results={}
	setmetatable(syncs.results,{__mode = "k"}) -- weak keys

	return syncs
end

-- run all waiting coroutines in list removing the finished ones
M.syncs.run=function(syncs)

	for idx=#syncs.list,-1,1 do
		
		local c=syncs.list[idx]
		
		syncs:resume(c)

		if coroutine.status( c )=="dead" then -- remove from list when dead
			table.remove( syncs.list , idx )
		end

	end

end

-- resume or start and return t finished
M.syncs.resume=function(syncs,c,...)
	if coroutine.status( c )~="dead" then
		local r = { coroutine.resume( c , ... ) }
		assert( r[1] , debug.traceback( c , r[2] ) )
		if coroutine.status( c )=="dead" then -- last call as an error would have asserted
			table.remove(r,1) -- remove ok boolean
			syncs.results[c]=r
		end
	end
	return ( coroutine.status( c )=="dead" )
end

-- get and delete result so should only be called once as another call will always be nil
M.syncs.result=function(syncs,c)
	local r=syncs.results[c]
	syncs.results[c]=nil
	return unpack(r)
end

-- start a new function as a coroutine  and call it once with the suplied args
-- return the coroutine which we can call resume on until it gives us a result
-- note that the coroutine will be resumed once before we return so may already be complete
M.syncs.start=function(syncs,f,...)

	local c=coroutine.create(f)
	
	syncs.list[#syncs.list+1]=c
	
	syncs:resume(c,...) -- first call
	
	return c
end

-- wrap a function in a coroutine and resume it repeatedly until it finishes
-- other functions in this syncs list will also be resumed whilst we do this
-- this is similar to doing an await in js but the function is just a function with possible yields
M.syncs.wait=function(syncs,f,...)

	local c=syncs:start(f,...)
	while not syncs:resume(c) do -- until finished
		syncs:run() -- auto run other tasks
		syncs:sleep() -- try and sleep but probably wont
	end
	
	return syncs:result(c)
end

-- yield if we are in a coroutine, do nothing if not
M.syncs.yield=function(syncs)
	if coroutine.running() then
		coroutine.yield()
	end
end

-- create global syncs and expose methods to module
M.global=M.create()
M.yield  =function()      return M.global:yield()       end
M.wait   =function(f,...) return M.global:wait(f,...)   end
M.start  =function(f,...) return M.global:start(f,...)  end
M.result =function(c)     return M.global:result(c)     end
M.resume =function(c,...) return M.global:resume(c,...) end
M.run    =function(c,...) return M.global:run()         end
