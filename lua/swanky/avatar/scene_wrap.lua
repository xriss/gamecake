--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local deepcopy=require("wetgenes"):export("deepcopy")

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local djon=require("djon")

local zscene=require("wetgenes.gamecake.zone.scene")
--local chunks=require("unzone.system.chunk")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.create=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or zscene.create()
	
	scene.wrap_name="swanky.avatar.scene_wrap"
	
	scene.require_search={
		"wetgenes.gamecake.zone.system.",
		"unzone.system.",
		"",
	}
	for _,it in pairs({ -- static data and functions for each system
		scene:require("all"),
		scene:require("kinetic"),
--		scene:require("zone"),
		scene:require("render"),
		scene:require("camera"),
		scene:require("sky"),
		scene:require("floor"),
		scene:require("water"),
		scene:require("player"),
--		scene:require("solid"),
--		scene:require("chunk"),
--		scene:require("chunk_floor"),
--		scene:require("chunk_flora"),
	}) do
		scene.infos[it.caste]=it
	end
	
	
	scene.full_clean=function(scene)
		scene:systems_cocall("clean")
		scene:call("destroy")
		scene:reset()
	end
	
	
	scene.full_setup=function(scene)

		scene:systems_cocall("setup")

		local boots={
--			{"zone",zid=0,zname="test",depends={kinetic=2}},
			{"kinetic"},
		}
		-- batch alloc singular systems
		local zones=scene:creates(boots)

		local boots={
			{"camera",depends={focus=2,render=3,water=4,sky=5}},
			{"player",cam_focus={0,0,0},depends={camera=1}},
			{"render",depends={camera=1}},
			{"water",depends={camera=1}},
			{"sky",depends={camera=1}},
			{"floor",},
		}
		-- batch alloc player items
		local players=scene:creates(boots)
		
		-- we are in all_values but need to reset all_tweens as awell
		scene:do_memo({
			task="all_tweens",
--				id=false,
			cmd="tick",
			tick=scene.values:get("tick"),
		})
		scene.oven.upnet.reset_tick( scene.values:get("tick") ) -- reset tick

		scene.pause_housekeeping=4

	end

	return scene

end
