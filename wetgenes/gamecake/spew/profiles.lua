-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs=require("lfs")
local wwin=require("wetgenes.win") -- system independent helpers
local wstr=require("wetgenes.string")
local wsbox=require("wetgenes.sandbox")
local snames=require("wetgenes.gamecake.spew.names")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,profiles)

	profiles=profiles or {}

	profiles.filename=wwin.files_prefix.."profiles.lua"
	
	local cake=oven.cake
	local canvas=cake.canvas
	
	local p
	local ps={}
	
-- check profile data is valid
	function profiles.check()
		for i,p in pairs(ps) do
		end
	end

-- initialise profile data
	function profiles.init()
		ps={}
		for i=1,5 do
			p={}
			p.name=snames.random()
			ps[i]=p
		end
		profiles.select(1)
	end
	
-- load all profile data
	function profiles.load()
print("Loading "..profiles.filename)
		local fp=io.open(profiles.filename,"r")
		if fp then
			local s=fp:read("*all")
			ps=wsbox.lson(s) -- safeish
			fp:close()
			profiles.select(1)
			profiles.check()
			return true
		end
		
		return false
	end
	
-- save all profile data
	function profiles.save()
print("Saving "..profiles.filename)
		local fp=io.open(profiles.filename,"w")
		fp:write(wstr.serialize(ps))
		fp:close()
	end

-- make this profile current
	function profiles.select(pid)
		profiles.pid=pid
		p=ps[pid] or ps[1]
	end

-- set a value in the current profile
	function profiles.set(name,value)
		if p[name]~=value then
			p[name]=value
			profiles.save()
		end
	end
	
-- get a value from the current profile
	function profiles.get(name)
		return p[name]
	end

-- set a score in the current profile
	function profiles.set_score(name,score)
		local scores=p[scores] or {}
		p[scores]=scores
		scores[#scores+1]=score
		profiles.save()
	end

-- simple iteration of all profiles, iterates the profile raw tab (ipairs)
	function profiles.ipairs()
		return ipairs(ps)
	end
	

--make sure we have a dir to load/save profiles into
lfs.mkdir(wwin.files_prefix:sub(1,-2)) -- skip trailing slash

-- try autoload
if not profiles.load() then
-- or create and save a default file
	profiles.init()
	profiles.save()
end

	return profiles
end
