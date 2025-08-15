--
-- (C) 2025 Kriss@XIXs.com
--

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local all=require("wetgenes.gamecake.zone.system.all") -- put everything in here
-- sub tasks for running in other threads
all.code=all.code or {}
-- methods added to manifest, we do not require a scene or systems to manifest boot data
all.manifest=all.manifest or {}
-- methods added to scene
all.scene=all.scene or {}
-- methods added to systems, shared resources can be kept in a system but not state data
all.system=all.system or {}
-- methods added to each item
all.item=all.item or {}

M.system=all.system -- our only export is also all.system


local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local Ox=function(n) return string.format("%012x",n or 0) end

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

local bit = require("bit")


local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

local json_diff=require("wetgenes.json_diff")
local hashish=require("wetgenes.json_diff").hashish


-- get a system by name
all.system.get_system=function(sys,name)
	return sys.scene.systems[name]
end

-- get a system.singular by name
all.system.get_singular=function(sys,name)
	local sys=sys.scene.systems[name]
	if sys then return sys.singular end
end

all.system.get_rnd_raw=function(sys)
	local r=sys:get("rnd")
	if not r then r=0 for i=1,#sys.caste do r=r+sys.caste:byte(i) end r=r%65536 end
	r=(((r+1)*75)%65537)-1
	sys:set("rnd",r) -- note rnd is in the range 0 to 65535 inclusive
	return r
end
-- get a basic pseudo random number between 0 and 1 inclusive ( zxspectrum sequence )
-- a changes request to an integer between 1 and a inclusive (a can not be more than 65536)
-- b changes request to be a number between a and b inclusive (floating point)
all.system.get_rnd=function(sys,a,b)
	local r=sys:get_rnd_raw()
	if b then
		return a+((b-a)*(r/65535))
	elseif a then
		return 1+(r%a)
	end
	return (r/65535) -- so this may produce a 0 or 1
end

-- load glsl with same name as module if it exists
all.system.load_glsl=function(sys)

	local wzips=require("wetgenes.zips")
	local gl=sys.gl

	local nameslash=sys.info.modname:gsub("%.","/")
	local filename="lua/"..nameslash..".glsl"
	local src=wzips.readfile(filename)
	if src and gl then
		gl.shader_sources( src , filename )
--		print("GLSL LOAD",sys.info.modname,filename)
	end

end

-- initalize metatables links and load extra data ( call once as system is added )
all.system.initialize=function(sys)

	local merge=function(into,from)
		if not into then return end
		for n,v in pairs( from or {} ) do
			if type(into[n])=="nil" then -- never overwrite
				into[n]=v
			end
		end
	end

	-- make sure we have unique sub tables
	sys.methods=sys.methods or {}
	sys.metatable=sys.metatable or {}
	sys.zips=sys.zips or {}

	sys.info.types=sys.info.types or {}
	sys.info.twraps=sys.info.twraps or {}

	-- and methods will be available to items via metatable
	sys.metatable.__index=sys.methods

	-- keep shortcuts to oven bound data in each system
	sys.oven=sys.scene.oven
--	sys.upnet=sys.oven.upnet
	sys.gl=sys.oven.gl

	-- keep shortcuts to oven bound data in each items metatable
	sys.methods.sys=sys
	sys.methods.scene=sys.scene
	sys.methods.oven=sys.oven
--	sys.methods.upnet=sys.upnet
	sys.methods.gl=sys.gl

	sys.uidmap=sys.info.uidmap
	sys.methods.uidmap=sys.info.uidmap

	sys.methods.caste=sys.caste

	-- get list of all sub castes seperated by _
	local castes={}
	for caste in all.inherits(sys.caste,true) do castes[#castes+1]=caste end
	castes[#castes+1]="all" -- and include all as a generic base class

	-- merge all castes
	for _,caste in ipairs(castes) do
		local info=sys.scene.infos[caste]
		if info then
			merge( sys             , info.system or {} ) -- merge system functions
			merge( sys.methods     , info.item   or {} ) -- and item functions
			merge( sys.info.values , info.values or {} ) -- and default values
			merge( sys.info.types  , info.types  or {} ) -- and types of values
			merge( sys.info.twraps , info.twraps or {} ) -- and twraps of values
		end
	end

	for n,v in pairs( sys.info.values ) do -- find zip values which wil be non tardis tables and auto flag tweens
		local t=type(v)
		if t=="table" then -- a table
			if not v.new then -- not a tardis value, assume ziped data
				sys.zips[n]=""
				sys.info.types[n]="ignore" -- do not get/set these
			end
		end
		if not sys.info.types[n] then	-- any other values are set to auto
			sys.info.types[n]="get"
		end
	end
	sys.types={}
	for n,v in pairs( sys.info.types ) do
		if v~="ignore" then -- remove ignores
			sys.types[n]=v
		end
	end
	sys.twraps={}
	for n,v in pairs( sys.info.twraps ) do
		sys.twraps[n]=v
	end

	sys.info.values_order={}
	for n in pairs(sys.info.values) do sys.info.values_order[#sys.info.values_order+1]=n end
	table.sort(sys.info.values_order)

	-- global system values
	sys.values=sys.scene.create_values()
	sys.tweens=sys.scene.create_values() -- ( drawing cache of values )
	sys:get_rnd() -- seed with caste name

	-- load glsl code if it exists
	sys:load_glsl()

end


-- core creation actions
all.system.create_core=function(sys,boot)
	local it={}
	local boot=boot or {}
	boot.caste=sys.caste -- make sure caste is correct
	it.boot=sys:gene(boot) -- apply basic boot values
	if it.boot.uid then it.uid = it.boot.uid end
	if it.boot.id  then it.id  = it.boot.id  end
	setmetatable(it,sys.metatable)
	sys.scene:add(it)
	it:setup_values(boot)
	it:setup() -- system specific setup
	return it
end

all.system.create=function(sys,boot)
	return all.system.create_core(sys,boot)
end

