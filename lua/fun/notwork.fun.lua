
--[[

build something "simple" and see if we can network it as a  multiplayer 
game

]]

local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local chatdown=require("wetgenes.gamecake.fun.chatdown")
local wstr=require("wetgenes.string")
function ls(s) print(wstr.dump(s))end

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

	boot=boot or {}
	
	boot.pos=boot.pos or {0,0,0}
	boot.vel=boot.vel or {0,0,0}

	boot.rot=boot.rot or {0,0,0}
	boot.ang=boot.ang or {0,0,0}

	return boot

end

scenery.all.methods={}

scenery.all.methods.setup_body=function(it,boot)

	it.pos=V3( boot.pos )
	it.vel=V3( boot.vel )

	it.rot=V3( boot.rot )
	it.ang=V3( boot.ang )

end

scenery.all.methods.update_body=function(it)

	it.pos[1]=it.pos[1]+it.vel[1]
	it.pos[2]=it.pos[2]+it.vel[2]
	
	if it.pos[1]<0                    then it.pos[1]=it.pos[1]+components.screen.hx end
	if it.pos[1]>components.screen.hx then it.pos[1]=it.pos[1]-components.screen.hx end
	if it.pos[2]<0                    then it.pos[2]=it.pos[2]+components.screen.hy end
	if it.pos[2]>components.screen.hy then it.pos[2]=it.pos[2]-components.screen.hy end

	it.rot[3]=it.rot[3]+it.ang[3]

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
	local item={}
	setmetatable(item,sys.metatable)
	sys:gene(boot)
	scene.add( item , sys.caste , boot )
	item:setup(boot)
	return item
end

-- generate any missing boot data
scenery.item.gene=function(sys,boot)
	boot=sys:gene_body(boot)

	return boot
end


scenery.item.methods={}

scenery.item.methods.setup=function(it,boot)

	it:setup_body(it,boot)

end
	
scenery.item.methods.update=function(it)

	local up=oven.ups.manifest(1)

	local lx=up:axis("lx") or 0
	local ly=up:axis("ly") or 0
	local rx=up:axis("rx") or 0
	local ry=up:axis("ry") or 0
	
	
	local fa=1/16
	if ( it.vel[1]*it.vel[1] + it.vel[2]*it.vel[2] ) < (lx*lx + ly*ly) then
		fa=1/2
	end
	it.vel[1] = it.vel[1]*(1-fa) + lx*2*(fa)
	it.vel[2] = it.vel[2]*(1-fa) + ly*2*(fa)


	it.ang[3]=0
	local dorot=function(dx,dy)
		if ( dx*dx + dy*dy ) > 0.01 then
			local d=((math.atan2(dy,dx)*180/math.pi)+90+360*360)%360
			local dd=(((d-it.rot[3])+360*360)%360)
			if dd > 180 then dd=dd-360 end
			local s=5
			if dd<0 then it.ang[3]=-s else it.ang[3]=s end
		end
	end
	dorot(it.vel[1],it.vel[2])
	dorot(rx,ry)

	it:update_body(it,boot)

end
	
scenery.item.methods.draw=function(it)
	local spr=names.test_ship1
	components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=it.pos[1] , py=it.pos[2] , pz=it.pos[3] ,
		rz=math.floor((it.rot[3]/15)+0.5)*15,
	})
end





--
-- FINALY
--
-- preload all the images
for name,sys in pairs(scenery) do
	if sys.loads then sys.loads() end
end
