--
-- Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
--

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
wire.thread_metatable={__index=threads.thread_functions}

wire.fifo_functions={is="fifo"}
wire.fifo_metatable={__index=threads.fifo_functions}


wire.thread=function(opts)
--[[
	local t=type(thread)
	local thread_name
	local thread_idx
	if t=="number" then
		thread_idx=thread
		thread=nil
	elseif t=="string" then
		thread_name=thread
		thread=nil
	end
]]
	
	local thread = {}
	setmetatable(thread,threads.thread_metatable)

	return thread
end

wire.fifo=function(opts)
	local fifo={}
	setmetatable(fifo,threads.fifo_metatable)

	return fifo
end
