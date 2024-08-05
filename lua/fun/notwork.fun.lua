
--[[

build something "simple" and see if we can network it as a  multiplayer 
game

]]

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local chatdown=require("wetgenes.gamecake.fun.chatdown")
local wstr=require("wetgenes.string")
function ls(s) print(wstr.dump(s))end

local best_hx=320
local best_hy=200
do
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

end

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
	sys.metatable={__index=sys.methods}
	sys:create({})
end

scenery.item.create=function(sys,boot)
	local item={}
	setmetatable(item,sys.metatable)
	scene.add( item , sys.caste , boot )
	return item:setup(boot)
end


scenery.item.methods={}

scenery.item.methods.setup=function(item)
	item.rz=0
end
	
scenery.item.methods.update=function(item)
	item.rz=item.rz+0.1
end
	
scenery.item.methods.draw=function(item)
	local spr=names.test_ship1
	components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=128 , py=128 , pz=0 ,
		rz=math.floor((item.rz/15)+0.5)*15,
	})
end





--
-- FINALY
--
-- preload all the images after loading this file
for name,sys in pairs(scenery) do
	if sys.loads then sys.loads() end
end
