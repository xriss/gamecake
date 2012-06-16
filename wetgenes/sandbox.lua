--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--

local string=string
local table=table

local type=type
local pairs=pairs
local ipairs=ipairs
local tostring=tostring
local setmetatable=setmetatable

--
-- Simple sandboxing of lua functions
--


-- make a table to be used as a reasonably "safe" environment
-- code can still lock up in loops or allocate too much memory
-- but it doesnt get to jump out of its sandbox

local function local_make_env_safe()
local env={
	assert=assert,
	error=error,
	ipairs=ipairs,
	pairs=pairs,
	next=next,
	pcall=pcall,
	select=select,
	tonumber=tonumber,
	tostring=tostring,
	type=type,
	unpack=unpack,
	xpcall=xpcall,
	coroutine={
		create=coroutine and coroutine.create,
		resume=coroutine and coroutine.resume,
		running=coroutine and coroutine.running,
		status=coroutine and coroutine.status,
		wrap=coroutine and coroutine.wrap,
		yield=coroutine and coroutine.yield,
	},
	table={
		concat=table and table.concat,
		insert=table and table.insert,
		maxn=table and table.maxn,
		remove=table and table.remove,
		sort=table and table.sort,
	},
	string={
		byte=string and string.byte,
		char=string and string.char,
		find=string and string.find,
		format=string and string.format,
		gmatch=string and string.gmatch,
		gsub=string and string.gsub,
		len=string and string.len,
		lower=string and string.lower,
		match=string and string.match,
		rep=string and string.rep,
		reverse=string and string.reverse,
		sub=string and string.sub,
		upper=string and string.upper,
	},
	math={
		abs=math and math.abs,
		acos=math and math.acos,
		asin=math and math.asin,
		atan=math and math.atan,
		atan2=math and math.atan2,
		ceil=math and math.ceil,
		cos=math and math.cos,
		cosh=math and math.cosh,
		deg=math and math.deg,
		exp=math and math.exp,
		floor=math and math.floor,
		fmod=math and math.fmod,
		frexp=math and math.frexp,
		huge=math and math.huge,
		ldexp=math and math.ldexp,
		log=math and math.log,
		log10=math and math.log10,
		max=math and math.max,
		min=math and math.min,
		modf=math and math.modf,
		pi=math and math.pi,
		pow=math and math.pow,
		rad=math and math.rad,
		random=math and math.random, -- should replace with sandboxed versions
		randomseed=math and math.randomseed, -- should replace with sandboxed versions
		sin=math and math.sin,
		sinh=math and math.sinh,
		sqrt=math and math.sqrt,
		tan=math and math.tan,
		tanh=math and math.tanh,
	},
	os={
		clock=os and os.clock,
		date=os and os.date, -- this can go boom in some situations?
		difftime=os and os.difftime,
		time=os and os.time,
	},
}

-- a modified loadstring that can set its function environment
-- setfenv is probably quite dangerous to expose, too much opportunity for
-- mischief on any function the sandbox code is given access to
-- it is however safe in this use since its your function that was just
-- loadstringed
	env.loadstring=function(s,newenv)
		local f,e=loadstring(s)
		if f then setfenv(f,newenv or env) end
		return f,e
	end

	return env
end



module("wetgenes.sandbox")

--
-- get a functional environment full of useful but "safe" functions
--
function make_env(opts)

local env=local_make_env_safe()

	return env
end

