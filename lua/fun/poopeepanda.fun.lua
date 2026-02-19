--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
-- ok ok 

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wstr=require("wetgenes.string")


oven.opts.fun="" -- back to menu on reset				

sysopts={
	update=function() update() end, -- call global update
	draw=function() draw() end, -- call global draw

	mode="swordstone", -- select basic text setup on a 256x128 screen using the swanky32 palette.
	lox=256,loy=128, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
	layers=4,
	hardware={ -- modify hardware so we have sprites and move the text layer
		{
			component="copper",
			name="copper",
			autosize="lohi",
			layer=1,
		},
		{
			component="tilemap",
			name="map",
			tiles="tiles",
			tile_size={8,8},
			layer=2,
			autosize="lohi",
		},
		{
			component="sprites",
			name="sprites",
			tiles="tiles",
			layer=3,
		},
		{
			component="tilemap",
			name="text", -- will replace the old text
			tiles="tiles",
			tile_size={4,8},
			layer=4,
			autosize="lohi",
		},
	},
}

hardware,main=system.configurator(sysopts)

all=require("wetgenes.gamecake.zone.flat.all"):import()

all.create_scene=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or require("wetgenes.gamecake.zone.scene").create()
	scene.create_scene=all.create_scene
	scene.oven=oven

	-- do level after players so we can more easily focus
	scene.sortby.level=-1001
	scene:sortby_update()
	
	scene.require_search={
		"wetgenes.gamecake.zone.flat.",
		"",
	}
	for _,it in pairs({ -- static data and functions for each system
		all,
		require("wetgenes.gamecake.zone.flat.kinetic"):import(),
		levels,
		cameras,
		huds,
		players,
		gibs,
		faunas,
		fauna_pandas,
	}) do
		scene.infos[it.caste]=it
	end

	scene.do_clean=function(scene)
		scene:systems_cocall("clean")
		scene:call("destroy")
		scene:reset()
--		scene.infos.all.scene.initialize(scene)
	end
	
	scene.do_setup=function(scene)
		scene:systems_cocall("setup")

		local boots={
			{"kinetic"},
			{"level",idx=1},

			{"hud",idx=1,depends={player=5}},
			{"camera",idx=1,depends={player=5}},
			{"player",idx=1,pos={32,32,0},depends={camera=4,hud=3}},

			{"hud",idx=2,depends={player=8}},
			{"camera",idx=2,depends={player=8}},
			{"player",idx=2,pos={64,32,0},depends={camera=7,hud=6}},

--			{"fauna",sname="fauna_slime",pos={96,32,0}},
			{"fauna_panda",sname="fauna_panda",pos={192,32,0}},
		}
		scene:creates(boots)
	end

	scene.infos.all.scene.initialize(scene)

	return scene

end

local main_setup=function()

	-- reset tiles
    local ctiles=system.components.tiles
	ctiles.reset_tiles()
	ctiles.upload_default_font_4x8()
	ctiles.upload_default_font_8x8()
	ctiles.upload_default_font_8x16()

	-- create global scene
	global.scene=all.create_scene()
	scene:do_setup()

end
setup=main_setup

update=function()
	if setup then setup=setup() end -- call setup once
	scene:do_update()
end

draw=function()

	scene:do_draw()
end



--#base_draw.lua
--------------------------------------------------------------------------------
--
-- Drawing utility funcs

draws={}
draws.sprite=function(sname,pos,side,idx)
	local map=system.components.map
	local px=map.window_px-map.px
	local py=map.window_py-map.py
	local spr=system.components.tiles.names[sname]
	if idx then
		spr=spr.cuts[idx]
	end
	system.components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=pos[1]+px , py=pos[2]+py , pz=pos[3] ,
		sx=side,
		rz=0,
	})
end

--#player.lua
--------------------------------------------------------------------------------
--
-- a player

players={}
-- methods added to system
players.system={}
-- methods added to each item
players.item={}

players.caste="player"

-- players are hard linked to these other entities by uid
-- note slot 1 is reserved for a parent which we do not have so it is empty
-- which also means these other entities will back link to us via slot 1
players.uidmap={
	camera=2,
	hud=3,
	length=3,
}

players.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 0,0,0 ),
	ang=V3( 0,0,0 ),
	acc=V3( 0,200,0 ),
	idx=1,
	side=1,
	foot=8,
	onfloor=0,
	jump=0,
	flap=0,
	walk=0,
	score=0,
}

players.types={
	pos="tween",
	rot="tween",
	vel="get",
	ang="get",
	acc="get",
	idx="get",
	side="get",
	foot="get",
	onfloor="get",
	jump="get",
	flap="get",
	walk="get",
	score="get",
}


players.graphics={

{nil,"ply1",[[
. . . . . . . . . . . . . . . . 
. . . . . . r r r R . . . . . . 
. . . . r r r r r r R R . . . . 
. . . r 7 7 r 7 7 r r R R . . . 
. . r 7 0 0 7 0 0 7 r r R R . . 
. . r 7 0 0 7 0 0 7 r r R R . . 
. r r r 7 7 r 7 7 r r r R R R . 
. r r r r r r r r r r R R R R . 
. r r 7 r r r 7 r r r R R R R . 
. R r r 7 7 7 r r r R R R R R . 
. . R r r r r r r R R R R R . . 
. . R R r r r r R R R R R R . . 
. . . R R R R R R R R R R . . . 
. . . . R R R R R R R R . . . . 
. . . . . . R R R R . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply1_hand",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . r O r r R 
. . . . . . . . . . . r O r r R 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply1_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . O O . . O O . . . . . 
. . . . r r r . r r r . . . . . 
]]},

{nil,"ply1_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . R R . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . R R . . . . . . . . . . . R R R R . . . . . . 
. . . . . R R . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . R R . . . . . . . . . . . R R R R . . . . . . 
. . . . . O O . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . O O . . . . . . . . . . . R R R R . . . . . . 
. . . . r r r . . O O . . . . . . . . . O O . . . . O O . . . . . . . . . O O . r r r . . . . . . . . . . . O O O O . . . . . . 
. . . . . . . . r r r . . . . . . . . r r r . . . r r r . . . . . . . . r r r . . . . . . . . . . . . . . r r r r r . . . . . . 
]],4},

{nil,"ply2",[[
. . . . . . . . . . . . . . . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G 7 0 G 7 0 G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 G 7 G 7 G g g . . . . 
. . . G G 7 G 7 G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g g g g g g g g . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply2_hand",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . g G d G G . 
. . . . . . . . . . g G d G . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply2_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . g g . . g g . . . . . 
. . . . . g g . . g g . . . . . 
. . . . . G G . . G G . . . . . 
. . . . . d d . . d d . . . . . 
. . . . G G G . G G G . . . . . 
]]},

{nil,"ply2_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . g g . . g g . . . . . . . . . g g . . . . g g . . . . . . . . . g g . . g g . . . . . . . . . . . g g g g . . . . . . 
. . . . . G G . . g g . . . . . . . . . g g . . . . g g . . . . . . . . . g g . . G G . . . . . . . . . . . g g g g . . . . . . 
. . . . . d d . . G G . . . . . . . . . G G . . . . G G . . . . . . . . . G G . . d d . . . . . . . . . . . G G G G . . . . . . 
. . . . G G G . . d d . . . . . . . . . d d . . . . d d . . . . . . . . . d d . G G G . . . . . . . . . . . d d d d . . . . . . 
. . . . . . . . G G G . . . . . . . . G G G . . . G G G . . . . . . . . G G G . . . . . . . . . . . . . . G G G G G . . . . . . 
]],4},

}

players.item.get_values=function(player)

	player:get_auto_values()

end

players.item.set_values=function(player)

	player:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
players.system.setup=function(sys)

	 system.components.tiles.upload_tiles( players.graphics )

end

players.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
players.item.setup=function(player)
	player:get_values()

	player:setup_kinetic()
	player:set_values()
end

players.item.setup_kinetic=function(player)
	if player.body then return end -- only create once
	local space=player:get_singular("kinetic").space
	player.body=space:body(1,1)
	player.shape=player.body:shape("circle",4,0,0)
	player.shape:friction(0.5)
	player.shape:elasticity(0.5)
	player.shape:filter(player.uid,0x00000100,0x00ffffff)
	player.shape.uid=player.uid
	player:set_body() -- set positon etc
end

players.item.clean_kinetic=function(player)
	if not player.body then return end -- auto clean
	local space=player:get_singular("kinetic").space		
	space:remove(player.shape)
	space:remove(player.body)
	player.body=nil
	player.shape=nil
end

players.item.clean=function(player)
	player:clean_kinetic()
end

players.item.update=function(player)
	if player:get("deleted") then return end
	player:get_values()
	player:setup_kinetic() -- might need to recreate body
	
	local level=player:get_singular("level") -- only one level is active at a time

	local up=player.scene.ups[player.idx] or player.sys.oven.ups.empty

	local lx=( up:axis("lx") ) or 0
	local ly=( up:axis("ly") ) or 0
	local ba_now=( up:get("a") or up:get("a_set") ) or false
	local ba_set=( up:get("a_set") ) or false

	local grav=level:get_gravity(player.pos)
	if ba_now then
		grav=grav*0.5 -- reduce gravity while flapping arms
		player.flap=(player.flap+1)%4
	else
		player.flap=2
	end

	player.acc=V3( 0, 0 ,0) -- reset force
	local va -- velocity we want to achieve
	if player.onfloor>0 or player.jump>0 then -- when on floor
		va=lx*512
	else -- when in air
		va=lx*256
	end
	if va then -- apply left/right movement
		if va<0 and player.vel[1]>0 then player.vel[1]=0 end -- quick turn
		if va>0 and player.vel[1]<0 then player.vel[1]=0 end -- quick turn
		local vb=va-player.vel[1] -- diff from current velocity
		player.acc[1]=player.acc[1]+(vb) -- apply force to make us move at requested speed
	end

	if lx*lx > 1/16 then
		player.walk=player.walk+1
		if player.walk>4 then player.walk=1 end
	else
		player.walk=0
	end
	
	player.vel[1]=player.vel[1]*12/16 --  dampen horizontal velocity
	player.vel[2]=player.vel[2]*14/16 --  dampen vertical velocity
	
	if lx<0 then player.side= 1 end
	if lx>0 then player.side=-1 end

--	player.pos=player.pos+player.vel

	local footspeed=0.25

	local space=player:get_singular("kinetic").space
	local hit=space:query_segment_first(player.pos[1],player.pos[2],player.pos[1],player.pos[2]+16,2,player.uid,0x00000100,0x00ffffff)
	if hit and hit.alpha and hit.alpha<0.75 then

		local d=(hit.alpha*16)+2 -- distance + radius
		local o=player.vel[2] -- original velocity
		local v=((d-9)) -- distance to where we want to be

		if ba_now then -- crouch while button held down
			v=v+3
		end

		local a=v*32 -- force to adjust velocity by

		player.foot=d
		if player.foot<7  then player.foot=7  end
		if player.foot>11 then player.foot=11 end
		player.acc[2]=player.acc[2]+a --  hover

		player.onfloor=4

if hit.shape.uid then
	local it=scene:find_uid(hit.shape.uid)
	if it and it.caste=="fauna" then
		if it:mark_deleted() then
			player.score=player.score+100
--			scene.systems.gibs:add_burst(it.pos,player.vel,4,V3(0,-200,0),4)
			for i=1,16 do
				local v=V3( player.vel[1]*-2+(100*((player.sys:get_rnd()-0.5)*2)) , player.vel[2]*-2+(-100*player.sys:get_rnd()) , 0 )
				local boots={
					{"gib",sname="gib_green",life=0,size=4,pos=it.pos,vel=v},
				}
				scene:creates(boots)
			end
		end
	end
--	print("player stomp",hit.shape.uid)
end

	else
		if player.foot>11 then player.foot=player.foot-footspeed end
		if player.foot<11 then player.foot=player.foot+footspeed end

		player.acc:add(grav) -- gravity
	end

	if player.onfloor>0 and player.jump<=0 then -- meep meep jump	
		if ba_set then
			player.onfloor=0
			player.jump=4
			player.acc[2]=0
			player.vel[2]=-120
		end
	end
	if player.onfloor>0 then player.onfloor=player.onfloor-1 end

	if player.jump>0 then -- jump higher while button is held down
		player.onfloor=0 -- no foot grab while jumping
		player.jump=player.jump-1 -- continue jump
	end

--PRINT( ba_now , player.onfloor , player.jump )
--PRINT( player.vel )

	player:set_body() -- then we call update_kinetic which will set_values before draw
end

players.item.update_kinetic=function(player)
	if player:get("deleted") then return end
	player:get_body()
	player:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
players.item.draw=function(player)
	if player:get("deleted") then return end

	player:get_values()

	local p=V3( player.pos[1] , player.pos[2], player.pos[1]+player.pos[2] )
	local f=math.abs(player.flap-2)
	draws.sprite( "ply"..player.idx          , p , player.side )
	draws.sprite( "ply"..player.idx.."_hand" , p+V3(0,f,-1) , player.side )

	if player.walk==0 then
		draws.sprite( "ply"..player.idx.."_feet" , p+V3(0,player.foot-8,-1) , player.side )
	else
		draws.sprite( "ply"..player.idx.."_walk" , p+V3(0,player.foot-8,-1) , player.side , player.walk)
	end
end


--#fauna.lua
--------------------------------------------------------------------------------
--
-- a basic fauna

faunas={}
-- methods added to system
faunas.system={}
-- methods added to each item
faunas.item={}

faunas.caste="fauna"

faunas.uidmap={
	length=0,
}

faunas.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 0,0,0 ),
	ang=V3( 0,0,0 ),
	acc=V3( 0,200,0 ),
	idx=1,
	side=1,
	foot=8,
	onfloor=0,
	jump=0,
	flap=0,
	sname="",
	thunk=0,
}

faunas.types={
	pos="tween",
	rot="tween",
	vel="get",
	ang="get",
	acc="get",
	idx="get",
	side="get",
	foot="get",
	onfloor="get",
	jump="get",
	flap="get",
	sname="get",
	thunk="get",
}


faunas.graphics={

{nil,"fauna_slime",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . d d d G . . . . . . 
. . . . d d d d d d G G . . . . 
. . . d d d d d d d d G G . . . 
. . d d 7 0 d 7 0 d d d G G . . 
. . d d 0 0 d 0 0 d d d G G . . 
. d d d d d d d d d d d G G G . 
. d d d d d d d d d d G G G G . 
. G G d d d d d d d G G G G G . 
. . G G G G G G G G G G G G . . 
]]},

{nil,"fauna_slime_splat",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . d d d d d d G G . . . . . . . . 
. . . . . . . d d d d d d d d G G . . . . . . . 
. . . . . . d d 7 0 d 7 0 d d d G G . . . . . . 
. . . . G G d d 0 0 d 0 0 d d d G G G G . . . . 
G G G G G G G G G G G G G G G G G G G G G G G G 
]]},

{nil,"fauna_slime_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . . G G G G G G G G G G . . . 
. . . . . G G G G G G G . . . . 
]]},

}

faunas.item.get_values=function(fauna)

	fauna:get_auto_values()

end

faunas.item.set_values=function(fauna)

	fauna:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
faunas.system.setup=function(sys)

	 system.components.tiles.upload_tiles( faunas.graphics )

end

faunas.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
faunas.item.setup=function(fauna)
	fauna:get_values()

	fauna:setup_kinetic()
	fauna:set_values()
end

faunas.item.setup_kinetic=function(fauna)
	if fauna.body then return end -- already done
	local space=fauna:get_singular("kinetic").space
	fauna.body=space:body(1,1)
	fauna.shape=fauna.body:shape("circle",4,0,0)
	fauna.shape:friction(0.5)
	fauna.shape:elasticity(0.5)
	fauna.shape:filter(fauna.uid,0x00010000,0x00ffffff)
	fauna.shape.uid=fauna.uid
	fauna:set_body()
end

faunas.item.clean_kinetic=function(fauna)
	if not fauna.body then return end -- already done
	local space=fauna:get_singular("kinetic").space		
	space:remove(fauna.shape)
	space:remove(fauna.body)
	fauna.body=nil
	fauna.shape=nil
end

faunas.item.clean=function(fauna)
	fauna:clean_kinetic()
end

faunas.item.update_brain=function(fauna,brain)

	fauna.thunk=fauna.thunk-1
	if fauna.thunk<=0 then
		fauna.thunk=fauna.sys:get_rnd(8,32)
		brain.jump=V3(fauna.sys:get_rnd(-180,180),fauna.sys:get_rnd(-10,-160),0)
		brain.move[1]=brain.jump[1]<0 and -1 or 1
	end
end

faunas.item.update=function(fauna)
	if fauna:get("deleted") then return end
	fauna:get_values()
	fauna:setup_kinetic() -- might need to recreate body
	
--	local up=fauna.scene.ups[fauna.idx] or fauna.sys.oven.ups.empty

	local level=fauna:get_singular("level") -- only one level is active at a time

	local brain={}
	brain.move=V3(0,0,0)
	brain.jump=nil
	fauna:update_brain(brain)

	local grav=level:get_gravity(fauna.pos)

	fauna.acc=V3( 0, 0 ,0) -- reset force
	local va -- velocity we want to achieve
	if fauna.onfloor>0 or fauna.jump>0 then -- when on floor
		va=brain.move[1]*512
	else -- when in air
		va=brain.move[1]*256
	end
	if va then -- apply left/right movement
		if va<0 and fauna.vel[1]>0 then fauna.vel[1]=0 end -- quick turn
		if va>0 and fauna.vel[1]<0 then fauna.vel[1]=0 end -- quick turn
		local vb=va-fauna.vel[1] -- diff from current velocity
		fauna.acc[1]=fauna.acc[1]+(vb) -- apply force to make us move at requested speed
	end
	
	fauna.vel[1]=fauna.vel[1]*12/16 --  dampen horizontal velocity
	fauna.vel[2]=fauna.vel[2]*14/16 --  dampen vertical velocity
	
	if brain.move[1]<0 then fauna.side= 1 end
	if brain.move[1]>0 then fauna.side=-1 end

--	fauna.pos=fauna.pos+fauna.vel

	local footspeed=0.25
	local footbase=3
	local space=fauna:get_singular("kinetic").space
	local hit=space:query_segment_first(fauna.pos[1],fauna.pos[2],fauna.pos[1],fauna.pos[2]+16,2,fauna.uid,0x00010000,0x00ffffff)
	if hit and hit.alpha and hit.alpha<((footbase+4)/16) then

		local d=(hit.alpha*16)+2 -- distance + radius
		local o=fauna.vel[2] -- original velocity
		local v=((d-(footbase+2))) -- distance to where we want to be
		local a=v*32 -- force to adjust velocity by

		fauna.foot=d-(footbase+2)
		if fauna.foot<0  then fauna.foot=0  end
		if fauna.foot>3 then fauna.foot=3 end
		fauna.acc[2]=fauna.acc[2]+a --  hover

		fauna.onfloor=4

if hit.shape.uid then
--	print("monster stomp",hit.shape.uid)
end

	else
		if fauna.foot>3 then fauna.foot=3 end
		if fauna.foot<3 then fauna.foot=fauna.foot+footspeed end

		fauna.acc:add(grav) -- gravity
	end

	if fauna.onfloor>0 and fauna.jump<=0 then -- meep meep jump	
		if brain.jump then
			fauna.onfloor=0
			fauna.jump=4
			fauna.acc[2]=0
			fauna.vel:add(brain.jump) --[2]=-120
		end
	end
	if fauna.onfloor>0 then fauna.onfloor=fauna.onfloor-1 end

	if fauna.jump>0 then -- jump higher while button is held down
		fauna.onfloor=0 -- no foot grab while jumping
		fauna.jump=fauna.jump-1 -- continue jump
	end

--PRINT( ba_now , fauna.onfloor , fauna.jump )
--PRINT( fauna.vel )

	fauna:set_body() -- then we call update_kinetic which will set_values before draw
end

faunas.item.update_kinetic=function(fauna)
	if fauna:get("deleted") then return end
	fauna:get_body()
	fauna:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
faunas.item.draw=function(fauna)
	if fauna:get("deleted") then return end

	fauna:get_values()

	local p=V3( fauna.pos[1] , fauna.pos[2]-3, fauna.pos[1]+fauna.pos[2] )
	local f=math.abs(fauna.flap-2)
	draws.sprite( fauna.sname          , p , fauna.side )
	draws.sprite( fauna.sname.."_feet" , p+V3(0,fauna.foot,8) , fauna.side )

end


--#gib.lua
--------------------------------------------------------------------------------
--
-- a basic gib

gibs={}
-- methods added to system
gibs.system={}
-- methods added to each item
gibs.item={}

gibs.caste="gib"

gibs.uidmap={
	length=0,
}

gibs.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 0,0,0 ),
	ang=V3( 0,0,0 ),
	acc=V3( 0,200,0 ),
	sname="gib_green",
	life=16,
	size=4,
}

gibs.types={
	pos="tween",
	rot="tween",
	vel="get",
	ang="get",
	acc="get",
	snane="get",
	life="get",
	size="get",
}


gibs.graphics={


{nil,"gib_green",[[
. . g g g g . . . . . . . . . . . . . . . . . . . . . . . . . . 
. g d d G g g . . . g g g g . . . . . . . . . . . . . . . . . . 
g d d d d G g g . g g d G g g . . . . g g . . . . . . . . . . . 
g d d d d G G g . g d d d G g . . . g d G g . . . . . d G . . . 
g G d d G G G g . g G d G G g . . . g G G g . . . . . G g . . . 
g G G G G G G g . g g G G g g . . . . g g . . . . . . . . . . . 
. g g G G G g . . . g g g g . . . . . . . . . . . . . . . . . . 
. . g g g g . . . . . . . . . . . . . . . . . . . . . . . . . . 
]],4},

}

gibs.collision_name=0x00010001	-- unique ID
gibs.collision_bits=0x01000000	-- assigned bitmask
gibs.collision_mask=0x000000ff	-- interact bitmask
gibs.collision_handlers={
	[{gibs.collision_name}]={
		postsolve=function(it)
			local gib=it.shape_a.in_body -- should be a gib
			local points=it:points()
			local n=V3( points.normal_x , points.normal_y , 0 )
			-- remember what we hit for next update
			gib.hit={
				body=it.shape_a.in_body,
				normal=n,
			}
			return true
		end
	}
}

gibs.item.get_values=function(gib)

	gib:get_auto_values()

end

gibs.item.set_values=function(gib)

	gib:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
gibs.system.setup=function(sys)

	 system.components.tiles.upload_tiles( gibs.graphics )

end

gibs.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
gibs.item.setup=function(gib)
	gib:get_values()
	
	gib:setup_kinetic()
	gib:set_values()
end

gibs.item.setup_kinetic_reshape=function(gib)
	local space=gib:get_singular("kinetic").space
	if gib.shape then
		space:remove(gib.shape)
		gib.shape=nil
	end
	gib.shape=gib.body:shape("circle",gib.size,0,0)
--	gib.shape=gib.body:shape("box",-3,-3,3,3,0)
	gib.shape:friction(1.0)
	gib.shape:elasticity(0.5)
	gib.shape:filter(gib.uid,gibs.collision_bits,gibs.collision_mask)
	gib.shape:collision_type(gibs.collision_name)
	gib.shape.uid=gib.uid
end
gibs.item.setup_kinetic=function(gib)
	if gib.body then return end -- already done
	local space=gib:get_singular("kinetic").space
	gib.body=space:body(1,1)
	gib:setup_kinetic_reshape()
	gib:set_body()
end

gibs.item.clean_kinetic=function(gib)
	if not gib.body then return end -- already done
	local space=gib:get_singular("kinetic").space		
	space:remove(gib.shape)
	space:remove(gib.body)
	gib.body=nil
	gib.shape=nil
end

gibs.item.clean=function(gib)
	gib:clean_kinetic()
end


gibs.item.update=function(gib)
	if gib:get("deleted") then return end
	gib:get_values()
	gib:setup_kinetic() -- might need to recreate body
	
	if gib.body.hit then
		if gib.life>0 then
			gib.life=gib.life-1
			gib.body.hit=nil -- ignore hits until we run out of life
		else
			if gib.size>1 then
				gib.size=gib.size-1
				gib:setup_kinetic_reshape()
				gib.body.hit=nil
			else
				gib:mark_deleted()
			end
		end
	end

	gib:set_body() -- then we call update_kinetic which will set_values before draw
end

gibs.item.update_kinetic=function(gib)
	if gib:get("deleted") then return end
	gib:get_body()
	gib:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
gibs.item.draw=function(gib)
	if gib:get("deleted") then return end

	gib:get_values()

	local p=V3( gib.pos[1] , gib.pos[2], gib.pos[1]+gib.pos[2] )
	draws.sprite( gib.sname          , p , 1 , 5-gib.size)

end

--#fauna_panda.lua
--------------------------------------------------------------------------------
--
-- a basic fauna_panda

fauna_pandas={}
-- methods added to system
fauna_pandas.system={}
-- methods added to each item
fauna_pandas.item={}

fauna_pandas.caste="fauna_panda"

fauna_pandas.uidmap={
	length=0,
}

fauna_pandas.values={
	pos=V3( 0,0,0 ),
	sname="",
	thunk=0,
}

fauna_pandas.types={
	pos="tween",
	sname="get",
	thunk="get",
}


fauna_pandas.graphics={

{nil,"fauna_panda",[[
. . 0 0 0 0 . . . . . . . . . . . 0 0 0 0 . . . 
. 0 0 1 1 0 0 . . 6 6 6 6 6 . . 0 0 1 1 0 0 . . 
. 0 1 0 0 0 0 6 6 6 6 6 6 6 6 6 0 0 0 0 1 0 . . 
. 0 1 0 0 0 6 6 6 6 6 6 6 6 6 6 6 0 0 0 1 0 . . 
. 0 0 0 0 6 6 0 0 0 6 6 6 0 0 0 6 6 0 0 0 0 . . 
. . 0 0 6 6 0 0 6 0 0 6 0 0 6 0 0 6 6 0 0 . . . 
. . . . 6 6 0 0 0 0 6 6 6 0 0 0 0 6 6 . . . . . 
. . . 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 6 . . . . 
. . . 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 6 . . . . 
. . . 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 6 . . . . 
. . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . 
. . . . 6 6 6 0 6 6 0 0 0 6 6 0 6 6 6 . . . . . 
. . . . . 6 6 6 0 0 6 6 6 0 0 6 6 6 . . . . . . 
. . . . . . 0 6 6 6 6 6 6 6 6 6 0 . . . . . . . 
. . . . . 0 0 0 0 6 6 6 6 6 0 0 0 0 . . . . . . 
. . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . 
. . . 0 0 0 0 0 0 0 6 6 6 0 0 0 0 0 0 0 . . . . 
. . 0 1 0 0 0 0 6 6 6 6 6 6 6 0 0 0 0 1 0 . . . 
. . 0 1 1 0 . 6 6 6 0 6 0 6 6 6 . 0 1 1 0 . . . 
. . . 0 0 . . 6 6 6 6 0 6 6 6 6 . . 0 0 . . . . 
. . . . . . . 0 0 0 6 0 6 0 0 0 . . . . . . . . 
. . . . . . 0 1 0 1 0 . 0 1 0 1 0 . . . . . . . 
. . . . . . 0 0 1 0 0 . 0 0 1 0 0 . . . . . . . 
. . . . . . . 0 0 0 . . . 0 0 0 . . . . . . . . 
]]},

}

fauna_pandas.item.get_values=function(fauna_panda)

	fauna_panda:get_auto_values()

end

fauna_pandas.item.set_values=function(fauna_panda)

	fauna_panda:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fauna_pandas.system.setup=function(sys)

	 system.components.tiles.upload_tiles( fauna_pandas.graphics )

end

fauna_pandas.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fauna_pandas.item.setup=function(fauna_panda)
	fauna_panda:get_values()

	fauna_panda:set_values()
end

fauna_pandas.item.update_brain=function(fauna_panda,brain)

	fauna_panda.thunk=fauna_panda.thunk-1
	if fauna_panda.thunk<=0 then
		fauna_panda.thunk=fauna_panda.sys:get_rnd(16*8,16*16)
		scene:creates({
			{"fauna",sname="fauna_slime",pos={fauna_panda.pos[1],fauna_panda.pos[2],0}},
		})
	end


end

fauna_pandas.item.update=function(fauna_panda)
	fauna_panda:get_values()
	
	local brain={}
	fauna_panda:update_brain(brain)
	
	fauna_panda:set_values()
end

fauna_pandas.item.update_kinetic=function(fauna_panda)
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
fauna_pandas.item.draw=function(fauna_panda)

	fauna_panda:get_values()

	local p=V3( fauna_panda.pos[1] , fauna_panda.pos[2]-3, fauna_panda.pos[1]+fauna_panda.pos[2] )
	draws.sprite( fauna_panda.sname          , p , 1 )

end

--#level.lua
--------------------------------------------------------------------------------
--
-- a level

levels={}
-- methods added to system
levels.system={}
-- methods added to each item
levels.item={}

levels.caste="level"

levels.uidmap={
	length=0,
}

levels.values={
	pos=V3( 0,0,0 ),
	focus=V3( 0,0,0 ),
	idx=1,
	time=0,
}

levels.types={
	pos="tween",
	focus="tween",
	time="tween",
	idx="get",
}


levels.graphics={

{nil,"char_empty",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},

{nil,"char_black",[[
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
]]},

{nil,"char_solid",[[
r r r r r r r R 
r r r r r r r R 
r r r r r r r R 
R R R R R R R R 
r r r R r r r r 
r r r R r r r r 
r r r R r r r r 
R R R R R R R R 
]]},

{nil,"char_sign",[[
. . . F f . . . 
s s s s s s s s 
s 0 1 0 1 0 1 s 
s 1 0 1 0 1 0 s 
s s s s s s s s 
. . . F f . . . 
. . . F f . . . 
. . . F f . . . 
]]},


}

levels.combine_legends=function(...)
	local legend={}
	for _,t in ipairs{...} do -- merge all
		for n,v in pairs(t) do -- shallow copy, right side values overwrite left
			legend[n]=v
		end
	end
	return legend
end

levels.legend={
	[0]={ name="char_empty",	},

	[". "]={ name="char_empty",				},
	["< "]={ name="char_empty",				dir_left=1,  },
	["> "]={ name="char_empty",				dir_right=1, },
	["^ "]={ name="char_empty",				dir_up=1,    },
	["v "]={ name="char_empty",				dir_down=1,  },
	["00"]={ name="char_black",				solid=1, dense=1, },		-- black border
	["0 "]={ name="char_solid",				solid=1, dense=1, },		-- empty border
	["T1"]={ name="char_sign",				text="Welcome to the Dungeon, we got fun and games." },
	["T2"]={ name="char_sign",				text="We got everything you want, honey, we got the Memes." },
	["T3"]={ name="char_sign",				text="Stomp ten Slims." },
	["P1"]={ spawn="player", idx=1,  },
	["P2"]={ spawn="player", idx=2,  },
}

levels.infos={}

levels.infos[1]={
legend=levels.combine_legends(levels.legend,{
}),
title="Test.",
map=[[
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . T1. . . . T2. . . T3. . . . . . . . . . . . 0 
0 . . . 0 0 0 0 . 0 0 0 0 . 0 0 . 0 0 0 . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . 0 0 . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . 0 . . . . . . . . . . . . . 0 
0 . . . 0 0 0 0 . . . . . . . . 0 0 . . . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . 0 0 0 . . . . . . . . . . . . 0 
0 0 0 0 . . . . . . . . . . . . . . . . . . 0 0 0 . . . 0 0 0 0 
0 . . . . . . . . 0 0 0 . . . . . . . . . . . . . . . . . . . 0 
0 P1. . . . . . . . . . . . . . . . . . . . . . . . . . . . P20 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
]],
}

levels.item.get_values=function(level)

	level:get_auto_values()

end

levels.item.set_values=function(level)

	level:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
levels.system.setup=function(sys)

	 system.components.tiles.upload_tiles( levels.graphics )

end

levels.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
levels.item.setup=function(level)
	level:get_values()

	local names=system.components.tiles.names
	local space=level:get_singular("kinetic").space
	
	local info=levels.infos[level.idx]

	local tilemap={}
	for n,v in pairs( info.legend ) do -- build tilemap from legend
		if v.name then -- convert name to tile
			tilemap[n]=names[v.name]
		end
	end
	local hx,hy=bitdown.pix_size(info.map)
--print("mapsize",hx,hy)
	system.components.map.autosize=nil -- turn off autosize
	system.components.map.screen_resize(hx*8,hy*8)
	system.components.map.tilemap_grd:clear(0)

	bitdown.tile_grd( info.map, tilemap, system.components.map.tilemap_grd  ) -- draw into the screen (tiles)

	local map=bitdown.pix_tiles(  info.map,  info.legend )
	level.map=map
	local get_tile=function(x,y,name)
		local t=map[y] and map[y][x]
		if name then return t and t[name] else return t end
	end
	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			local shape
			if tile.solid then -- merge outside edges of solid cells to create smoother collisions
				if (not get_tile(x,y-1,"solid") or not get_tile(x,y+1,"solid") ) then -- try to drag across
					local otile=get_tile(x-1,y)
					if otile and otile.shape and otile.solid==tile.solid then -- drag across
						if otile.shape[5]-otile.shape[3] == 8 then -- check size
							shape=otile.shape
							shape[4]=shape[4]+8
						end
					end
				elseif (not get_tile(x-1,y,"solid") or not get_tile(x+1,y,"solid") ) then -- try to drag down
					local otile=get_tile(x,y-1)
					if otile and otile.shape and otile.solid==tile.solid then -- drag across
						if otile.shape[4]-otile.shape[2] == 8 then -- check size
							shape=otile.shape
							shape[5]=shape[5]+8
						end
					end
				end
				if not shape then -- start new shape
					shape={"box",x*8,y*8,(x+1)*8,(y+1)*8,0}
				end
				tile.shape=shape
			end
		end
	end
	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			local shape=tile.shape
			tile.shape=nil
			if shape and shape[2]==x*8 and shape[3]==y*8 then -- shape must start at this cell
--print("shape",unpack(shape))
				tile.shape=space.static:shape(unpack(shape))
				tile.shape:friction(tile.solid)
				tile.shape:elasticity(tile.solid)
				tile.shape:filter(0,0x00000001,0xffffffff)
			end
			if tile.text then
				tile.text_lines=wstr.smart_wrap(tile.text,(256-16)/8)
			end
		end
	end

-- set background color
	local it=system.components.copper
	it.shader_name="fun_copper_back_y5"
	it.shader_uniforms.cy0={ 0.25 , 0    , 0.25 , 1   }
	it.shader_uniforms.cy1={ 0.125, 0    , 0.25 , 1   }
	it.shader_uniforms.cy2={ 0.125, 0.125, 0.25 , 1   }
	it.shader_uniforms.cy3={ 0    , 0.125, 0.25 , 1   }
	it.shader_uniforms.cy4={ 0    , 0.25 , 0.25 , 1   }

	it.shader_uniforms.cy0={ 1/2 , 0   , 1/2 , 1   }
	it.shader_uniforms.cy1={ 1/4 , 0   , 1/2 , 1   }
	it.shader_uniforms.cy2={ 1/4 , 1/4 , 1/2 , 1   }
	it.shader_uniforms.cy3={ 0   , 1/4 , 1/2 , 1   }
	it.shader_uniforms.cy4={ 0   , 1/2 , 1/2 , 1   }

	level.time=0

	level:set_values()
end

levels.item.update=function(level)
	level:get_values()

	level.time=level.time+(1/16)
	
	level:set_values()
end

levels.item.get_gravity=function(level,pos)
	return V3(0,400,0)
--[[
	local n=pos-V3(128,96,0)
	local d=n:len()
	n:normalize()
	d=d-100
	if d>0 then
		d=0
	else
		d=math.abs(d*d)
		if d>800 then d=800 end
	end
	return n*-d
]]
end

levels.item.get_tile_by_idx=function(level,idx,name)
	local x=idx%0x100
	local y=math.floor(idx/0x100)%0x100
	return level:get_tile_by_xy(x,y,name)
end

levels.item.get_tile_by_xy=function(level,x,y,name)
	local t=level.map[y] and level.map[y][x]
	if name then return t and t[name] else return t end
end


levels.item.get_tile_by_pos=function(level,pos)
	local x=math.floor((pos[1])/8)
	local y=math.floor((pos[2])/8)
	return level:get_tile_by_xy(x,y)
end

levels.item.each_tile_near=function(level,pos,near)

--	local done={}
	local list={}
	local push=function(t)
		if not t then return end
--		if done[t] then return end
--		done[t]=true
		list[#list+1]=t
	end

	local bx=math.floor((pos[1])/8)
	local by=math.floor((pos[2])/8)

	push( level:get_tile_by_xy(bx,by) )

	for n=1,near do
		for x=-n,n,1 do
			push( level:get_tile_by_xy(bx+x,by-n) )
			push( level:get_tile_by_xy(bx+x,by+n) )
		end
		for y=-n+1,n-1,1 do
			push( level:get_tile_by_xy(bx-n,by+y) )
			push( level:get_tile_by_xy(bx+n,by+y) )
		end
	end

	return ipairs(list)
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
levels.item.draw=function(level)
	
	local text=system.components.text

	local add_char16=function(s,x,y)
		local sx,sy=text.text_tile8x16(s)
		system.components.sprites.list_add({
			t=sy*256+sx ,
			hx=8 , hy=16 ,
			ox=0 , oy=0 ,
			px=x+1 , py=y+1 , pz=0 ,
			color=0xff000000,
		})
		system.components.sprites.list_add({
			t=sy*256+sx ,
			hx=8 , hy=16 ,
			ox=0 , oy=0 ,
			px=x , py=y , pz=-1 ,
			color=0xffffffff,
		})
	end
	local add_string16=function(s,x,y)
		local l=#s
		for i=1,l do
			local c=s:sub(i,i)
			add_char16(c,x+((i-1)*8),y)
		end
	end

	local add_char4=function(s,x,y)
		local sx,sy=text.text_tile4x8(s)
		system.components.sprites.list_add({
			t=sy*256+(sx/2) ,
			hx=4 , hy=8 ,
			ox=0 , oy=0 ,
			px=x+1 , py=y+1 , pz=0 ,
			color=0xff000000,
		})
		system.components.sprites.list_add({
			t=sy*256+(sx/2) ,
			hx=4 , hy=8 ,
			ox=0 , oy=0 ,
			px=x , py=y , pz=-1 ,
			color=0xffffffff,
		})
	end
	local add_string4=function(s,x,y)
		local l=#s
		for i=1,l do
			local c=s:sub(i,i)
			add_char4(c,x+((i-1)*4),y)
		end
	end

	local tf=math.floor(level.time*100)%100
	local ts=math.floor(level.time)%60
	local tm=math.floor(level.time/60)
	local s=tm..":"..("0"..ts):sub(-2)
	add_string16(s,128-(#s*4),0)

	local players=scene:caste("player")
	for i,p in ipairs(players) do
		if p.idx==1 then
			local s=p.score..""
			add_string4(s,2,1)
		elseif p.idx==2 then
			local s=p.score..""
			add_string4(s,254-(#s*4),1)
		end
	end
end

--#camera.lua
--------------------------------------------------------------------------------
--
-- a camera

cameras={}
-- methods added to system
cameras.system={}
-- methods added to each item
cameras.item={}

cameras.caste="camera"

cameras.uidmap={
	player=1,
	length=1,
}

cameras.values={
	pos=V3( 0,0,0 ),
	focus=V3( 0,0,0 ),
	idx=1,
	tile=0x0000,
}

cameras.types={
	pos="tween",
	focus="tween",
	idx="get",
}


cameras.graphics={

}

cameras.item.get_values=function(camera)

	camera:get_auto_values()

end

cameras.item.set_values=function(camera)

	camera:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
cameras.system.setup=function(sys)

	 system.components.tiles.upload_tiles( cameras.graphics )

end

cameras.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
cameras.item.setup=function(camera)
	camera:get_values()

	camera:set_values()
end

cameras.item.update=function(camera)
	if camera.idx~=1 then return end -- only 1 hud

	camera:get_values()
	

	local player=camera:depend("player")
	local hud=player:depend("hud")

	camera.focus:mix(player.pos+player.vel,1/16) -- smooth ?
	
	local map=system.components.map
	local screen=system.components.screen

	if map.window_hx<=screen.hx then -- center
		camera.pos[1]=-(screen.hx-map.window_hx)/2
	else
		camera.pos[1]=camera.focus[1]-( screen.hx/2 )
		if camera.pos[1]<0 then camera.pos[1]=0 end
		local m=map.window_hx-screen.hx
		if camera.pos[1]>m then camera.pos[1]=m end
	end

	local shy=(screen.hy-hud.pos[2])
	if map.window_hy<=shy then -- center
		camera.pos[2]=-(shy-map.window_hy)/2
	else
		camera.pos[2]=camera.focus[2]-( shy/2 )
		if camera.pos[2]<0 then camera.pos[2]=0 end
		local m=map.window_hy-(shy-hud.pos[1])
		if camera.pos[2]>m then camera.pos[2]=m end
	end



	camera:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
cameras.item.draw=function(camera)
	if camera.idx~=1 then return end -- only 1 hud

	local map=system.components.map
	local screen=system.components.screen

	camera:get_values()

	if camera.pos[1]<0 then
		map.window_px=math.floor(-camera.pos[1])
		map.px=0
	else
		map.window_px=0
		map.px=math.floor(camera.pos[1])
	end
	
	if camera.pos[2]<0 then
		map.window_py=math.floor(-camera.pos[2])
		map.py=0
	else
		map.window_py=0
		map.py=math.floor(camera.pos[2])
	end

end

--#hud.lua
--------------------------------------------------------------------------------
--
-- a hud

huds={}
-- methods added to system
huds.system={}
-- methods added to each item
huds.item={}

huds.caste="hud"

huds.uidmap={
	player=1,
	length=1,
}

huds.values={
	pos=V3( 0,0,0 ),
	focus=V3( 0,0,0 ),
	idx=1,
	dst=V3( 0,0,0 ),
	tile_idx=0,
	tile_time=0,
}

huds.types={
	pos="tween",
	dst="tween",
	focus="tween",
	idx="get",
	tile_idx="get",
}


huds.graphics={

{nil,"char_life",[[
. . . . . . . . 
. . . 1 1 . . . 
. . . 1 1 . . . 
. . . . . . . . 
. . 1 . . 1 . . 
. . . 1 1 . . . 
. . . 1 1 . . . 
. . 1 . . 1 . . 
]]},


}

huds.item.get_values=function(hud)

	hud:get_auto_values()

end

huds.item.set_values=function(hud)

	hud:set_auto_values()

end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
huds.system.setup=function(sys)

	 system.components.tiles.upload_tiles( huds.graphics )

end

huds.system.clean=function(sys)
end

huds.system.draw=function(sys)

    local ctext=system.components.text
    local bg=24
	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color

end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
huds.item.setup=function(hud)
	hud:get_values()

	hud:set_values()
end

huds.item.update=function(hud)
	if hud.idx~=1 then return end -- only 1 hud

	hud:get_values()
	
	local player=hud:depend("player")
	
	local level=hud:get_singular("level") -- only one level is active at a time
	
	local show_tile
	for _,t in level:each_tile_near( player.pos , 1 ) do
		if t.name=="char_sign" then
			show_tile=t
			break
		end
	end
	if show_tile and hud.tile_idx~=show_tile.idx then -- new text
		hud.tile_idx=show_tile.idx
		hud.tile_time=0
	end
	
	if show_tile then
		hud.dst=V3(0,#show_tile.text_lines*16+16,0)
		hud.tile_time=hud.tile_time+1
	else
		hud.dst=V3(0,0,0)
		hud.tile_time=hud.tile_time-1
		if hud.tile_time<0 then hud.tile_time=0 end
	end


	hud.pos=(hud.dst+hud.pos*7)/8

	hud:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
huds.item.draw=function(hud)
	if hud.idx~=1 then return end -- only 1 hud


	local text=system.components.text
	local screen=system.components.screen
	local level=hud:get_singular("level") -- only one level is active at a time

	hud:get_values()

	text.window_py=screen.hy-hud.pos[2]
	text.py=0

    local bg=24
--	text.text_clear(0x01000000*bg) -- clear text forcing a background color

--	text.text_print("This is the hud, 4 lives and 10 secs",0,0,31,24) -- (text,x,y,color,background)
--	text.text_print2("This is the hud, 4 lives and 10 secs",0,1,31,24) -- (text,x,y,color,background)

	if hud.tile_idx>=0 then
		local maxchar=math.floor(hud.tile_time/1)
		
		local tile=level:get_tile_by_idx(hud.tile_idx)
		if tile and tile.text_lines then
			for i,line in ipairs(tile.text_lines) do
				if maxchar>0 then
					local s=line:sub(1,maxchar)
					maxchar=maxchar-#s
					text.text_print4(s,2,i*2-1,31,24) -- (text,x,y,color,background)
				end
			end
		end
	end
end

--#
--------------------------------------------------------------------------------

-- lock globals to help catch future accidents
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
