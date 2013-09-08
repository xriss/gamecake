
--
-- This is the relocation part of a vobj, not all vobjs need to move so not all vobjs need this
--

local win=win

local math=math

local table=table
local ipairs=ipairs
local coroutine=coroutine
local error=error
local require=require
local print=print

local gl=require("gl")
local tardis=require("wetgenes.tardis")

module(...)
local ville=require("state.ville")
local vobj=require("state.ville.vobj")



function setup(it,tab)

	it.area={}
	local a=it.area
	
-- these are screen locations, ignore the z part

	a.p=tardis.v3.new() -- the zero point of this object in screen space
	
	a.x=tardis.v3.new() -- unit vectors pointing along the objects x,y,z axis
	a.y=tardis.v3.new()
	a.z=tardis.v3.new()

	a.min=tardis.v3.new()
	a.max=tardis.v3.new()

		
	return it
end


function clean(it)
end


function update(it,xyz)

	local a=it.area

-- setup in 3d space
	
	a.p:set(xyz)
	
	a.x:set(a.p)
	a.x[1]=a.x[1]+1
	
	a.y:set(a.p)
	a.y[2]=a.y[2]+1
	
	a.z:set(a.p)
	a.z[3]=a.z[3]+1
	
-- convert to 2d space

	ville.mtx_3d_to_2d:product(a.p)
	ville.mtx_3d_to_2d:product(a.x)
	ville.mtx_3d_to_2d:product(a.y)
	ville.mtx_3d_to_2d:product(a.z)
	
-- scale x,y into pixels

	local w=win.width/2
	local h=win.height/2
	
	a.p[1]=(a.p[1]+1)*w
	a.x[1]=(a.x[1]+1)*w
	a.y[1]=(a.y[1]+1)*w
	a.z[1]=(a.z[1]+1)*w

	a.p[2]=(1-a.p[2])*h
	a.x[2]=(1-a.x[2])*h
	a.y[2]=(1-a.y[2])*h
	a.z[2]=(1-a.z[2])*h

-- subtract p from x,y,z to turn them into 2d unit vectors (the unit is in 3d space)

	a.x:sub(a.p)
	a.y:sub(a.p)
	a.z:sub(a.p)
	
-- finally work out the average size in 2d space

	a.s=math.sqrt( ( (a.x[1]*a.x[1]) + (a.x[2]*a.x[2]) + (a.y[1]*a.y[1]) + (a.y[2]*a.y[2]) + (a.z[1]*a.z[1]) + (a.z[2]*a.z[2]) )  / 3 )

-- we can now multiply lengths by s or use the x,y,z normals offset from p to get 2d points for mouse collisions
	
	a.min:set(a.p):sub({a.s*20,a.s*20,0})
	a.max:set(a.p):add({a.s*20,a.s*20,0})
	
end
