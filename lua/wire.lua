--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

-- need to preloadlibs on new threads
local preloadlibs=require("wetgenes.gamecake.core").preloadlibs



--[[#lua.wire

	local wire=require("wire")

We use wire as the local name of this library.

Multiple long lived lua states managed via standard c11 threads and 
mutexs with simple fifo message handling. A small compatibility library 
is needed for building for windows using mingw. No problems with 
linux/android/wasm

We are not doing anything clever.
 
Only data is passed and that data must fit in messagepack, eg json like but 
binary strings are allowed.

No lua tables that are both arrays and objects, messagepack does not 
understand them.

No userdata, no functions.

When in doubt, send a string, remember they can be binary strings, then 
unpack it at the other end.

Finally any lua libs required in a thread *must* be thread safe.

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local wire=M

local core=require("wire.core")

-- meta methods bound to the various objects

wire.thread_functions={is="thread"}
wire.thread_metatable={__index=wire.thread_functions}

wire.fifo_functions={is="fifo"}
wire.fifo_metatable={__index=wire.fifo_functions}

-- keep cache of threads and fifos by handle
wire.cache_threads={}
wire.cache_fifos={}

-- cache names to handles ( house thread syncs this data )
wire.cache_names={} 

--[[#lua.wire.thread

	thread = wire.thread(opts)

Create or wrap a thread handle.

	opts.handle=nil

The handle to wrap or use in creation. If a valid handle we will simply 
wrap it with a thread table and return it. No thread will be created.

	opts.start="..."

Create a new thread and run this code.

	opts.data={}

Optional data for the new thread. Will become part of the start string 
inside a local called start_data

]]
wire.thread=function(opts)
	
	local thread = {}
	setmetatable(thread,wire.thread_metatable)
	
	thread.handle=opts.handle

	if not core.handle( thread.handle ) then -- we must create a new thread

		local start=wire.prepare_start(opts)
--		print(start)
		
		thread.handle=core.thread_create( thread.handle or 0 ,
			start , preloadlibs )
		
	end
	
	if not core.handle( thread.handle ) then
		error("wire thread creation failed")
	end

	return thread
end

--[[#lua.wire.fifo

]]
wire.fifo=function(opts)
	local fifo={}
	setmetatable(fifo,wire.fifo_metatable)

	fifo.handle=opts.handle

	if not core.handle( fifo.handle ) then -- we must create a new fifo

	end
	
	if not core.handle( fifo.handle ) then
		error("wire fifo creation failed")
	end

	return fifo
end


--[[#lua.wire.serialize

	luastr = wire.serialize(tab)
	luastr = wire.serialize(tab,{ indent="\t" , newline="\n" , errors=true  })

Turn a table into a valid lua string that will recreate the table.

Optionally pass in an opts table that we will use during serialization 
so must be unique.

Possible options are.

	indent=" "
	
Indention per level. set to "" for compact output.

	newline="\n"
	
Line ending, set to "" for overly compact output.

	errors=nil
	
Set this to true and we will raise errors rather than simply adding 
comments to the output. EG unknown types or recursion.

]]
wire.serialize = function(o,opts)
	opts=opts or {}

	-- set defaults
	local opts_indent=opts.indent or " "
	local newline=opts.newline or "\n"
	local errors=opts.errors

	local dedupe={}
	local ret={}
	local fout=function(...)
		for i,v in ipairs({...}) do ret[#ret+1]=tostring(v) end
	end

	-- recursive
	local serialize ; serialize=function(o,indent)
		local t=type(o)
		if t == "number" then
		
			return fout(o)
			
		elseif t == "boolean" then
		
			if o then return fout("true") else return fout("false") end
			
		elseif t == "string" then
		
			return fout(string.format("%q", o))
			
		elseif t == "table" then
		
			local indent2=indent..opts_indent -- next indent level
			
			if dedupe[o] then
				if errors then
					error("recursive table "..tostring(o))
				else
					fout("{ --[[ ",tostring(o)," ]] }")
					return
				end
			else
				dedupe[o]=true
			
				fout("{",newline)
							
				local maxi=0
				
				for k,v in ipairs(o) do -- dump number keys in order
					fout(indent2)
					serialize(v,indent2)
					fout(",",newline)
					maxi=k -- remember top
				end
				
				for k,v in pairs(o) do
					 -- skip what we already dumped
					if (type(k)~="number") or (k<1) or (k>maxi) or (math.floor(k)~=k) then
						fout(indent2,"[")
						serialize(k,indent2)
						fout("]=")
						serialize(v,indent2)
						fout(",",newline)
					end
				end
				fout(indent,"}")
				dedupe[o]=false
				return
			end
		elseif t == "nil" then	
			return fout("nil")
		else
			if errors then
				error("cannot serialize a " .. type(o))
			else
				return fout("nil --[[",tostring(o),"]]")
			end
		end
	end

	serialize(o,"")
	fout(newline)
	return table.concat(ret)
end


--[[#lua.wire.prepare_start

	start = wire.prepare_start( { start="..." , data={} } )

]]
wire.prepare_start=function( opts )

	local ret={}
	local fout=function(...)
		for i,v in ipairs({...}) do ret[#ret+1]=v end
	end
	
	fout("local head_data = ", wire.serialize({}) ) -- base data
	fout(wire.starters.header) -- always do this

	fout("local start_data = ", wire.serialize(opts.data) ) -- custom data
	fout(opts.start) -- your actual start code

	return table.concat(ret)
end

wire.do_start_header=function()

	print("hello i am",wire.thread_handle)

end

wire.do_start_house=function(data)

	print("i am house code",data.poop)

end

-- generic thread starting lua strings, you can do anything here but
-- the idea is we send state data here then require and run functions.

wire.starters={}
wire.starters.header=[[
	local wire=require("wire")
	wire.do_start_header(head_data)
]]
wire.starters.house=[[
	wire.do_start_house(start_data)
]]


-- thread -1 will always exist and thread -2 will be auto created here

wire.thread_handle = assert( core.thread() )-- get our thread handle 

wire.thread( { handle=-1 } ) -- Wrap the main thread

if core.handle( -2 ) then -- check if house thread exists

	wire.thread( { handle=-2 } ) -- Wrap the house thread

else -- Create the house thread ( we are the main thread and no other threads exist yet )

	assert( wire.thread_handle == -1 ) -- main thread check
	wire.thread( { handle=-2 , start=wire.starters.house , data={poop="yes"} , } )

end

if wire.thread_handle<-2 then -- we are not main or house so also wrap our thread handle

	wire.thread( { handle=wire.thread_handle } ) -- just wrap

end
