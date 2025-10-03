--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local deepcopy=require("wetgenes"):export("deepcopy")

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local wzips=require("wetgenes.zips")
local geom_gltf=require("wetgenes.gamecake.spew.geom_gltf")

local scene_wrap=require("swanky.avatar.scene_wrap")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,main_zone)
	main_zone=main_zone or {}
	main_zone.modname=M.modname

	local gl=oven.gl

	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
	local geoms=oven.rebake("wetgenes.gamecake.spew.geoms")

	local gui=oven.rebake(oven.modname..".gui")
	local shadow=oven.rebake("wetgenes.gamecake.zone.shadow")
	local screen=oven.rebake("wetgenes.gamecake.zone.screen")
	local avatar=oven.rebake(oven.modname..".avatar")


	oven.ups.keymap(1,"full") -- 1up has basic keyboard mappings
	oven.upnet=oven.rebake("wetgenes.gamecake.upnet")

main_zone.loads=function()

end

main_zone.setup=function()

-- reset matrices shaders might need to predraw textures
	gl.uniforms.camera=function(u)
		gl.UniformMatrix4f( u , M4() ) -- so we can undo the camera from the view
	end

	gl.uniforms.incamera=function(u)
		gl.UniformMatrix4f( u , M4() ) -- apply this one in a shader
	end

	main_zone.loads()

	screen.setup()
	shadow.setup()
--	shadow.maparea=64

	oven.upnet.setup()

	-- a scene is a bunch of systems and items
	local scene=scene_wrap.create()

	main_zone.scene=scene
	gui.scene=scene

	scene.oven=oven
	
	scene.infos.all.scene.initialize(scene)

--	scene:full_setup()
	scene:systems_cocall("setup")
	scene:start_all_tasks()


end

main_zone.clean=function()

	main_zone.scene:full_clean()

end


main_zone.msg=function(m)

--	main_zone.scene:systems_call("msg",m)

end

main_zone.update=function()

	shadow.maparea=16

	main_zone.scene:do_update()

end

main_zone.draw=function()

	main_zone.scene:do_draw()

end

	return main_zone
end

