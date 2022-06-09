--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local wwin=require("wetgenes.win") -- system independent helpers
local wstr=require("wetgenes.string")
local wsbox=require("wetgenes.sandbox")
local snames=require("wetgenes.gamecake.spew.names")
local lfs ; pcall( function() lfs=require("lfs") end ) -- may not have a filesystem

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,settings)

	settings=settings or {}


	settings.filename=wwin.files_prefix.."settings.lua"
	
	local cake=oven.cake
	local canvas=cake.canvas

--	local sgui=oven.rebake("wetgenes.gamecake.spew.gui")
	
	local sounds=oven.rebake("wetgenes.gamecake.sounds")
	
	local p
	local ss={}
	
-- check profile data is valid
	function settings.check()
	end

-- initialise profile data
	function settings.init()
		ss={}
		ss.vol_music=10/11
		ss.vol_sfx=10/11
	end
	
-- load all profile data
	function settings.load()
		if lfs then
log("oven","Loading "..settings.filename)
			local fp=io.open(settings.filename,"r")
			if fp then
				local s=fp:read("*all")
				ss=wsbox.lson(s) -- safeish
				fp:close()
				settings.check()
				for name,value in pairs(ss) do
					settings.apply(name,value)
				end
				return true
			end
		end
		return false
	end
	
-- save all profile data
	function settings.save()
		if lfs then
log("oven","Saving "..settings.filename)
			local fp=io.open(settings.filename,"w")
			if fp then
				fp:write(wstr.serialize(ss))
				fp:close()
			end
		end
	end

-- set a value in the current profile
	function settings.set(name,value)
		if ss[name]~=value then
			ss[name]=value
			settings.apply(name,value)
			settings.save()
		end
	end

-- apply this setting to the engine
	function settings.apply(name,value)
			if     name=="vol_music" then
				sounds.vol_stream=value
			elseif name=="vol_sfx"   then
				sounds.vol_beep=value
			end
	end
	
-- get a value from the current profile
	function settings.get(name,value)
		return ss[name] or value
	end

-- set a score in the current profile
	function settings.set_score(name,score)
		local scores=p[scores] or {}
		p[scores]=scores
		scores[#scores+1]=score
		settings.save()
	end

--make sure we have a dir to load/save settings into
if lfs then
	lfs.mkdir(wwin.files_prefix:sub(1,-2)) -- skip trailing slash
end

-- try autoload
if not settings.load() then
-- or create and save a default file
	settings.init()
	settings.save()
end

	return settings
end
