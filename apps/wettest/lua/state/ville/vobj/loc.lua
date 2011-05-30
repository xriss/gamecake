
--
-- This is the relocation part of a vobj, not all vobjs need to move so not all vobjs need this
--

local math=math

local table=table
local ipairs=ipairs
local coroutine=coroutine
local error=error
local require=require
local print=print

local gl=require("gl")
local tardis=require("wetgenes.tardis")
local comm=require("spew.client.comm")

module(...)
local ville=require("state.ville")
local vobj=require("state.ville.vobj")



function setup(it,tab)

	local new_loc=function()
		local loc={}
			loc.xyz=tardis.v3.new{0,0,0}
			loc.siz=tardis.v3.new{1,1,1}
			loc.rot=tardis.v3.new{0,0,0}
		return loc
	end

	it.loc=new_loc() -- where we are at this precise time, draw using these values
	it.dst=new_loc() -- where we want to be at some point in the future
	it.vel=new_loc() -- velocity
	
	it.vel.siz:set{0,0,0}
	it.vel.d=0
	
-- external values

	local p=tab.props.xyz
	if p then
	
		it.loc.xyz[1]=p[1]
		it.loc.xyz[2]=p[2]
		it.loc.xyz[3]=p[3]
		
		it.dst.xyz[1]=p[1]
		it.dst.xyz[2]=p[2]
		it.dst.xyz[3]=p[3]
		
	end
	
		
	return it
end


function clean(it)
end


function change(it,tab)
	if tab.props.xyz then
	
		local p=tab.props.xyz
			
		if p[4] then -- special?

			it.dst.xyz[1]=p[1]
			it.dst.xyz[2]=p[2]
			it.dst.xyz[3]=p[3]
		
		else -- just walk
			
			it.dst.xyz[1]=p[1]
			it.dst.xyz[2]=p[2]
			it.dst.xyz[3]=p[3]
			
		end

		local l=it.loc.xyz
		local d=it.dst.xyz
		local v=it.vel.xyz
		
		local t={}
		t[1]=d[1]-l[1]
		t[2]=d[2]-l[2]
		t[3]=d[3]-l[3]
		local dd = (t[1]*t[1]) + (t[2]*t[2]) + (t[3]*t[3])
		local d=math.sqrt(dd)
	
		if d<1 then d=1 end
		d=d/1.5
		it.vel.d=d
		v[1]=t[1]/(d)
		v[2]=t[2]/(d)
		v[3]=t[3]/(d)
	end
end


function update(it)

	
	if it.vel.d>0 then
	
		it.vel.d=it.vel.d-1
		
		local l=it.loc.xyz
		local d=it.dst.xyz
		local v=it.vel.xyz
		
		if it.vel.d>0 then
			l[1]=l[1]+v[1]
			l[2]=l[2]+v[2]
			l[3]=l[3]+v[3]
			
		else
			l[1]=d[1]
			l[2]=d[2]
			l[3]=d[3]
			
			if it.plan=="try" then -- we got here and have a plan to try again
				it.plan=nil
				local v=ville.vobjs[it.props.try]
				if v.use then v:use("try") end
			end
		end
	
	end
	
end

function set_translate(it)

	local p=it.loc.xyz
	local v=it.vel.xyz
	
	gl.Translate(p[1],p[2],p[3])
	
	local yrot=180
	if it.vel.d>0 then
		yrot=0+(180*math.atan2(-v[1], -v[3])/math.pi)
	end
	return yrot
end

