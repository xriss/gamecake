--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,buffers)
		
	local gl=oven.gl
	local cake=oven.cake
	
	local funcs={}
	local metatable={__index=funcs}

	buffers.data=setmetatable({}, { __mode = 'vk' })

	buffers.create = function(tab)

		local it={}
		setmetatable(it,metatable)
		for nam,val in pairs(tab) do -- copy
			it[nam]=val
		end
		
		it[0]=gl.GenBuffer()
		it.__gc=pack.alloc(1,{__gc=function() it:clean() end})
		
		if it.start then
			it:start(buffers) -- and call start now
		end
		
		buffers.data[it]=it

		
		return it
	end

	buffers.bind=function(it)
		gl.BindBuffer(gl.ARRAY_BUFFER,it[0])
	end

	buffers.start = function()
		for v,n in pairs(buffers.data) do
			if not v[0] then
				v[0]=gl.GenBuffer()
				if v.start then
					v:start(buffers)
				end
			end
		end
	end

	buffers.stop = function()
		for v,n in pairs(buffers.data) do
			if v[0] then gl.DeleteBuffer(v[0]) end
			v[0]=nil
			if v.stop then
				v:stop(buffers)
			end
		end

	end

	buffers.clean = function(it)
		buffers.data[it]=nil
		if it[0] then gl.DeleteBuffer(it[0]) end
		it[0]=nil
		if it.stop then it:stop(buffers) end
	end

-- set some functions into the metatable 
	for i,n in ipairs({
		"bind",
		"clean",
		}) do
		funcs[n]=buffers[n]
	end

-- create a simple number sequence in a fmt="u16" buffer
-- the buffer is filled in using a standard lua for loop with the provided range
-- this is intended to produce a buffer of unique IDs for procedural
-- vertex code ruining in a vertex shader
	buffers.create_sequence=function(fmt,min,max,stp)
		stp=stp or 1
		local vb=buffers.create({
			start=function(vb)
				vb:bind()
				local d={}
				for i=min,max,stp do d[#d+1]=i end
				local s=pack.save_array(d,fmt)
				gl.BufferData(gl.ARRAY_BUFFER,#s,s,gl.STATIC_DRAW)
			end,
		})
		return vb
	end
	
	return buffers
end




