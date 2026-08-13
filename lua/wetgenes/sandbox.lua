--
-- (C) 2013 Kriss@XIXs.com
--
--local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--
-- Simple sandboxing of lua functions
--

local M={ modname=(...) } ; package.loaded[M.modname]=M
local sandbox=M


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
	_VERSION=_VERSION,
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


--
-- get a functional environment full of useful but "safe" functions
-- probably not safe
--
sandbox.make_env=function (opts)

	local env=local_make_env_safe()

	return env
end

--
-- a duplicate env including unsafe function
-- definitely not safe
--
sandbox.make_unsafe_env=function (opts)

	local env=local_make_env_safe()

-- dangerous global tables
	env.package=package
	env.io=io
	env.debug=debug
	env.jit=jit

-- dangerous globals
	env.dofile=dofile
	env.getfenv=getfenv
	env.getmetatable=getmetatable
	env.load=Gload
	env.loadfile=loadfile
	env.print=print
	env.rawequal=rawequal
	env.rawget=rawget
	env.rawset=rawset
	env.setfenv=setfenv
	env.setmetatable=setmetatable
	env.module=module
	env.require=require

-- safeish globals
	env.assert=assert
	env.error=error
	env.ipairs=ipairs
	env.pairs=pairs
	env.next=next
	env.pcall=pcall
	env.select=select
	env.tonumber=tonumber
	env.tostring=tostring
	env.type=type
	env.unpack=unpack
	env.xpcall=xpcall
	env._VERSION=_VERSION

-- we have our own loadstring with builtin setfenv
--	env.loadstring=loadstring

	return env
end

--
-- this performs the opposite of a sandbox.save_ini
-- turns a string containing lua code into a table containing the globals it sets
-- IE read an ini file, run it through this
--
sandbox.ini=function (s,import)

	local env=sandbox.make_env()
	for n,v in pairs(import or {}) do env[n]=v end
	local tab={}
	local meta={__index=env}
	env._G=tab
	setmetatable(tab, meta)

	local f=assert(loadstring(s))
	setfenv(f,tab)
	assert(pcall(f))

	return tab
end
sandbox.load_ini=sandbox.ini

-- this performs the opposite of a sandbox.save_lson
sandbox.lson=function (s,import)

	local env=sandbox.make_env()
	for n,v in pairs(import or {}) do env[n]=v end
	local tab={}
	local meta={__index=env}
	env._G=tab
	setmetatable(tab, meta)

	local f=assert(loadstring("return "..s))
	setfenv(f,tab)
	local _,ret=assert(pcall(f))
	return ret
end
sandbox.load_lson=sandbox.lson

local sortedpairs = function( tab )
	local order={}
	for n,v in pairs(tab) do order[#order+1]=n end
	table.sort(order)
	local idx=0
	return function()
		idx=idx+1
		return order[idx] , tab[ order[idx] ]
	end
end

-- create lson formated text ( lua code )
sandbox.save_lson = function(o,opts)
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
				
				for k,v in sortedpairs(o) do
					 -- skip numbers we already dumped
					local tk=type(k)
					if (tk~="number") or (k<1) or (k>maxi) or (math.floor(k)~=k) then
						if  ( type(tk)=="string"                    ) -- must be string
						and ( not string.match ( k , "[^%a_%d]" )   ) -- must only contain alphanumeric or underscore
						and ( string.match ( k:sub(1,1) , "[%a_]" ) ) -- must start with letter or underscore
						then -- nekkid
							fout(indent2,k,"=")
						else
								fout(indent2,"[")
								serialize(k,indent2)
								fout("]=")
						end
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

-- create ini formated text ( lua code that writes to the environment )
-- similar to lson but the top level must be string keys with no funny characters
-- and these are written as globals
sandbox.save_ini = function(tab)
	local ret={}
	local fout=function(...)
		for i,v in ipairs({...}) do ret[#ret+1]=tostring(v) end
	end

	for n,v in sortedpairs(tab) do
		fout( n.."="..sandbox.save_lson(v))
	end

	return table.concat(ret)
end
