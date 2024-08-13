
--[[

build something "simple" and see if we can network it as a  multiplayer 
game

]]

local tardis=require("wetgenes.tardis")
local V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V1","V2","V3","V4","M2","M3","M4","Q4")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local chatdown=require("wetgenes.gamecake.fun.chatdown")
local wstr=require("wetgenes.string")
function ls(s) print(wstr.dump(s))end

-- provides base networking and synced inputs
local upnet=require("wetgenes.gamecake.upnet").bake(oven)

-- request full keymap
oven.ups.keymap(1,"full") -- 1up has basic keyboard mappings


do
	local best_hx=320
	local best_hy=200

	local fsx,fsy=system.fullscreen_width  or 1920,system.fullscreen_height or 1080
	local shy=math.floor((fsy)/best_hy) -- require at least 200 pixels high
	if shy<1 then shy=1 end -- sanity

	local shx=math.floor((fsx)/best_hx) -- require at least 320 pixels wide
	if shx<1 then shx=1 end -- sanity
	
	if shx<shy then shy=shx end -- pick the smallest divider

-- these may be slightly higher than the inputs, to fully cover available screen aspect ratio
	best_hx=math.floor(fsx/shy)
	best_hy=math.floor(fsy/shy)

-- for 1920x1080 we would get 384x216
	print( "myscreensize" , best_hx , best_hy , "x"..shy )

oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() -- called at a steady 60fps
		if setup then setup=setup() end -- call setup once
		upnet.update()
		scene.call("update")
	end,
	draw=function() -- called at actual display frame rate
		scene.call("draw")
	end,
	hx=best_hx,hy=best_hy, -- autoscale, with at least 320x200
})

end


setup=function()
print("setup")

	upnet.setup()

	-- cache system tables that we use a lot in globals
	components=system.components
	names=components.tiles.names

	-- use zone scene ( same as fun entities ) and set scene as global
	scene=require("wetgenes.gamecake.zone.scene").create({
		sortby={
			"item",
		},
	})

	-- create scene systems
	for name,sys in pairs(scenery) do
		sys.caste=name
		scene.systems_insert( sys )
	end

	-- setup all the systems
	scene.systems_call("setup")

end

-- custom scene code and data
scenery={}

scenery.all={}

scenery.all.setup_metatable=function(sys)

	-- make sure we have unique sub tables
	sys.methods=sys.methods or {}
	sys.metatable=sys.metatable or {}

	-- and methods are available to items
	sys.metatable.__index=sys.methods

	-- merge all functions into each system
	for n,v in pairs( scenery.all ) do
		if not sys[n] then
			sys[n]=v
		end
	end

	-- merge all methods into each item
	for n,v in pairs( scenery.all.methods ) do
		if not sys.methods[n] then
			sys.methods[n]=v
		end
	end
	
end

-- generate any missing boot data
-- do not call with : use . and pass in boot only
scenery.all.gene_body=function(sys,boot)
	
	boot.pos=boot.pos or {0,0,0}
	boot.vel=boot.vel or {0,0,0}

	boot.rot=boot.rot or {0,0,0}
	boot.ang=boot.ang or {0,0,0}

end

scenery.all.methods={}

-- test history diffs
scenery.all.methods.advance_values=function(it)

	it:push_values()
	while it.values_length>16 do -- max history
		it:pull_values()
	end

end

scenery.all.methods.setup_values=function(it)

	it.values={ {} }
	it.values_length=#it.values

end

scenery.all.methods.push_values=function(it)

	table.insert(it.values,1,{})
	it.values_length=#it.values

end

scenery.all.methods.pop_values=function(it)

	assert( it.values_length>1 ) -- must always leave some values

	return table.remove(it.values,1) -- go back in time one tick

end

-- remove base values and merge that data with new base
scenery.all.methods.pull_values=function(it)

	assert( it.values_length>1 ) -- must always leave some values

	local v=table.remove(it.values,it.values_length) -- base
	it.values_length=#it.values
	local w=it.values[it.values_length] -- new base
	
	for n,v in pairs(v) do -- copy into any nils
		if type(w[n])=="nil" then w[n]=v end
	end
	
	return v -- return old value object
end

scenery.all.methods.get=function(it,name)
	
	-- search backwards through time for this value
	for i=1,it.values_length do
		local v=it.values[i][name]
		if type(v)~="nil" then return v end
	end

end

-- get values for previous frame so we can tween with now
scenery.all.methods.get_prev=function(it,name)
	
	-- search backwards through time for this value
	for i=2,it.values_length do
		local v=it.values[i][name]
		if type(v)~="nil" then return v end
	end

end

scenery.all.methods.set=function(it,name,value)

	if it:get(name)~=value then -- only write if changed
		it.values[1][name]=value -- to current values object only
	end

end

scenery.all.methods.setup_body=function(it,boot)

	it:set( "pos" , V3( boot.pos ) )
	it:set( "vel" , V3( boot.vel ) )

	it:set( "rot" , V3( boot.rot ) )
	it:set( "ang" , V3( boot.ang ) )

end

scenery.all.methods.update_body=function(it,pos,vel,rot,ang)

	-- best to pass in these values if you have already got and modified them
	pos=pos or V3( it:get("pos") )
	vel=vel or V3( it:get("vel") )
	rot=rot or V3( it:get("rot") )
	ang=ang or V3( it:get("ang") )
	it:advance_values()

	pos=pos+vel
	
	if pos[1]<0                    then pos[1]=pos[1]+components.screen.hx end
	if pos[1]>components.screen.hx then pos[1]=pos[1]-components.screen.hx end
	if pos[2]<0                    then pos[2]=pos[2]+components.screen.hy end
	if pos[2]>components.screen.hy then pos[2]=pos[2]-components.screen.hy end

	rot=rot+ang

	it:set("pos",pos:quantize(1/128))
	it:set("vel",vel:quantize(1/128))
	it:set("rot",rot:quantize(1/128))
	it:set("ang",ang:quantize(1/128))

end


scenery.item={}

scenery.item.loads=function()

	hardware.graphics.loads{

{nil,"test_ship1",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . d . . . . . . . . 
. . . . . . G d d . . . . . . . 
. . . . . . G Y d . . . . . . . 
. . . . . . G d d . . . . . . . 
. . . . . d G d d G . . . . . . 
. . . . . d G d d G . . . . . . 
. . . . d d G d d G G . . . . . 
. . . d d d G d d G G G . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

	}

end
	
scenery.item.setup=function(sys)
	scenery.all.setup_metatable(sys)

	sys:create({pos={128,128,0}})
end

scenery.item.create=function(sys,boot)
	boot=boot or {}
	local it={}
	setmetatable(it,sys.metatable)
	sys:gene(boot)
	scene.add( it , sys.caste , boot )
	it:setup(boot)
	return it
end

-- generate any missing boot data
scenery.item.gene=function(sys,boot)

	sys:gene_body(boot)

end


scenery.item.methods={}

scenery.item.methods.setup=function(it,boot)

	it:setup_values(it)
	it:setup_body(it,boot)

end
	
scenery.item.methods.update=function(it)

	local pos=V3( it:get("pos") )
	local vel=V3( it:get("vel") )
	local rot=V3( it:get("rot") )
	local ang=V3( it:get("ang") )

	local up=oven.ups.manifest(1)

	local lx=up:axis("lx") or 0
	local ly=up:axis("ly") or 0
	local rx=up:axis("rx") or 0
	local ry=up:axis("ry") or 0
	
	
	local fa=1/16
	if ( vel[1]*vel[1] + vel[2]*vel[2] ) < (lx*lx + ly*ly) then
		fa=1/2
	end
	vel[1] = vel[1]*(1-fa) + lx*2*(fa)
	vel[2] = vel[2]*(1-fa) + ly*2*(fa)


	ang[3]=0
	local dorot=function(dx,dy)
		if ( dx*dx + dy*dy ) > 0.01 then
			local d=((math.atan2(dy,dx)*180/math.pi)+90+360*360)%360
			local dd=(((d-rot[3])+360*360)%360)
			if dd > 180 then dd=dd-360 end
			local s=5
			if dd<0 then ang[3]=-s else ang[3]=s end
		end
	end
	dorot(vel[1],vel[2])
	dorot(rx,ry)

	it:update_body(pos,vel,rot,ang)

end
	
scenery.item.methods.draw=function(it)

	local pos=V3( it:get("pos") )
	local rot=V3( it:get("rot") )

	local spr=names.test_ship1
	components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=pos[1] , py=pos[2] , pz=pos[3] ,
		rz=math.floor((rot[3]/15)+0.5)*15,
	})
	
--[[
	local s=""
	for i=1,it.values_length do
		s=s..tostring( it.values[i].pos ).." "
	end
	print(s)
]]

end





--
-- FINALY
--
-- preload all the images
for name,sys in pairs(scenery) do
	if sys.loads then sys.loads() end
end
