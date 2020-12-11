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



bullet.test=function()

	local world=core.world_create( {} )
	
	core.world_gravity(world,0,-10,0)

--	core.test( world )


	local shapes={}
	local bodys={}
	
	shapes[1]=core.shape_create("box",100,50,100)
	shapes[2]=core.shape_create("sphere",10)

	bodys[1]=core.body_create("rigid",shapes[1],0,0,-50,0)
	bodys[2]=core.body_create("rigid",shapes[2],1,0,20,0)

	core.world_add_body(world,bodys[1])
	core.world_add_body(world,bodys[2])

	for i = 1,150 do
		core.world_step(world,1/60,10)
		print(1," : ",core.body_transform(bodys[1]))
		print(2," : ",core.body_transform(bodys[2]))
	end

	print("done")
	
	for i,v in ipairs(shapes) do core.shape_destroy( v ) end
	for i,v in ipairs(bodys)  do core.body_destroy( v )  end
	core.world_destroy( world )

end



return bullet

