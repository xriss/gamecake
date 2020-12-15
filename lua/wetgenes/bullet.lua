--[[--------------------------------------------------------------------

(C) Kriss@XIXs.com 2020 and released under the MIT license.

See https://github.com/xriss/gamecake for full notice.

----------------------------------------------------------------------]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.bullet

	local bullet=require("wetgenes.bullet")

We use bullet as the local name of this library.

A lua binding to the [Bullet 
Physics](https://github.com/bulletphysics/bullet3) library

]]

local bullet={}

local core=require("wetgenes.bullet.core")
bullet.core=core

-- meta methods bound to the various objects

bullet.world_functions={is="world"}
bullet.world_metatable={__index=bullet.world_functions}

bullet.shape_functions={is="shape"}
bullet.shape_metatable={__index=bullet.shape_functions}

bullet.body_functions={is="body"}
bullet.body_metatable={__index=bullet.body_functions}

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world

	world=bullet.world()

Create the world you will be simulating physics in.

]]
bullet.world=function(...)
	local world={}
	world.bodies={}
	world.shapes={}
	setmetatable(world,bullet.world_metatable)
	world[0]=core.world_create(...)
	
--	core.world_register(world[0],world)
	
	return world
end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.destroy

	world:destroy()

Destroy the world and all associated data.

]]
bullet.world_functions.destroy=function(world)

	for i,body  in pairs(world.bodys)   do body:destroy() end
	for i,shape in pairs(world.shapes)  do shape:destroy() end

	core.world_destroy( world[0] )

end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.gravity

	world:gravity(x,y,z)
	
	x,y,z = world:gravity()

Set or get world gravity vector. Recommended gravity is 0,-10,0

]]
bullet.world_functions.gravity=function(world,vx,vy,vz)

	local rx,ry,rz = core.world_gravity( world[0] , vx , vy , vz )

	return rx,ry,rz

end


------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape

	shape = world:shape()

Create a shape.

]]
bullet.world_functions.shape=function(world,...)
	local shape={}
	setmetatable(shape,bullet.shape_metatable)
	shape[0]=core.shape_create(...)
	shape.world=world
	
	world.shapes[ shape[0] ]=shape

	return shape
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.shape.destroy

	shape:destroy()

Destroy shape.

]]
bullet.shape_functions.destroy=function(shape)
	local world=shape.world
	core.shape_destroy( shape[0] )
	world.shapes[ shape[0] ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body

	body = world:body()

Create a body.

]]
bullet.world_functions.shape=function(world,...)
	local body={}
	setmetatable(body,bullet.body_metatable)
	body[0]=core.body_create(...)
	body.world=world
	
	world.bodies[ body[0] ]=body

	return body
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.world.body.destroy

	body:destroy()

Destroy body.

]]
bullet.body_functions.destroy=function(body)
	local world=body.world
	core.body_destroy( body[0] )
	world.bodies[ body[0] ]=nil
end

------------------------------------------------------------------------
--[[#lua.wetgenes.bullet.test

	bullet.test()

Create a simple world and simulate some physics printing positions out 
to the console. Hopefully without crashing :)

]]
bullet.test=function()

	local world=core.world_create( )
	
	core.world_gravity(world,0,-10,0)

--	core.test( world )


	local shapes={}
	local bodys={}
	
	shapes[1]=core.shape_create("box",100,50,100)
	shapes[2]=core.shape_create("sphere",10)

	bodys[1]=core.body_create("rigid",shapes[1],0,0,-50,0)
	bodys[2]=core.body_create("rigid",shapes[2],1,0,20,0)
	bodys[3]=core.body_create("rigid",shapes[2],1,0,40,0)

	for i,v in ipairs(bodys) do
		core.world_add_body(world,v)
	end

	for i = 1,150 do
		core.world_step(world,1/60,10)
		for i,v in ipairs(bodys) do
			print(i," : ",core.body_transform(v))
		end
	end

	print("done")
	
	for i,v in ipairs(bodys)  do print("B",i) core.world_remove_body(world,v) core.body_destroy( v )  end
	for i,v in ipairs(shapes) do print("S",i) core.shape_destroy( v ) end
	print("W",1)
	core.world_destroy( world )

end



return bullet

