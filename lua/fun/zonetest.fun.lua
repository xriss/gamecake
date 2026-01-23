--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")


oven.opts.fun="" -- back to menu on reset

sysopts={
	update=function() update() end, -- call global update
	draw=function() draw() end, -- call global draw

	mode="swordstone", -- select basic text setup on a 256x128 screen using the swanky32 palette.
	lox=256,loy=128, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
	layers=3,
	hardware={ -- modify hardware so we have sprites and move the text layer
		{
			component="tilemap",
			name="map",
			tiles="tiles",
			tile_size={8,8},
			layer=1,
			autosize="lohi",
		},
		{
			component="sprites",
			name="sprites",
			tiles="tiles",
			layer=2,
		},
		{
			component="tilemap",
			name="text", -- will replace the old text
			tiles="tiles",
			tile_size={4,8}, -- use half width tiles for font
			layer=3,
			autosize="lohi",
		},
	},
}

hardware,main=system.configurator(sysopts)

all=require("wetgenes.gamecake.zone.flat.all"):import()

all.create_scene=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or require("wetgenes.gamecake.zone.scene").create()
	scene.create=create_scene
	scene.oven=oven
	
	scene.require_search={
		"wetgenes.gamecake.zone.flat.",
		"",
	}
	for _,it in pairs({ -- static data and functions for each system
		all,
		textbounces,
		players,
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
			{"textbounce",text="Hello World!",pos={1,1,0},vel={2/16,1/16,0},},
			{"textbounce",text="POOP!",pos={7,11,0},vel={2/16,1/16,0},},
			{"textbounce",text="Bounce!",pos={1,3,0},vel={2/16,1/16,0},},
			{"player",idx=1,pos={32,32,0}},
			{"player",idx=2,pos={64,32,0}},
		}
		scene:creates(boots)
	end

	scene.infos.all.scene.initialize(scene)

	return scene

end

setup=function()

	-- reset tiles
    local ctiles=system.components.tiles
	ctiles.reset_tiles()
	ctiles.upload_default_font_4x8()

	-- create global scene
	global.scene=all.create_scene()
	scene:do_setup()

end

update=function()
	if setup then setup=setup() end -- call setup once
	scene:do_update()
end

draw=function()

    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=0
	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
	-- all text is expected to be redrawn in scene
	scene:do_draw()
end






--------------------------------------------------------------------------------
--
-- bouncing text

textbounces={}
-- methods added to system
textbounces.system={}
-- methods added to each item
textbounces.item={}

textbounces.caste="textbounce"

textbounces.uidmap={
	length=0,
}

textbounces.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 1,1,0 ),
	ang=V3( 0,0,0 ),
	text="",
}

textbounces.types={
	pos="tween",
	rot="get",
	vel="get",
	ang="get",
	text="get",
}


textbounces.item.get_values=function(textbounce)

	textbounce:get_auto_values()
--	textbounce:get_body_values()

end

textbounces.item.set_values=function(textbounce)

	textbounce:set_auto_values()
--	textbounce:set_body_values()

end


textbounces.system.setup=function(sys)
end

textbounces.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
textbounces.item.setup=function(textbounce)
	textbounce:get_values()

	textbounce:set_values()
end

textbounces.item.update=function(textbounce)
	textbounce:get_values()

    local cscreen=system.components.screen

	local tx=math.floor(cscreen.hx/4)
	local ty=math.floor(cscreen.hy/8)
	local s=textbounce.text
	local sx=#s
	
	textbounce.pos[1]=textbounce.pos[1]+textbounce.vel[1]
	textbounce.pos[2]=textbounce.pos[2]+textbounce.vel[2]
	
	if textbounce.pos[1]<=0     then textbounce.pos[1]=0     ; textbounce.vel[1]= math.abs(textbounce.vel[1]) end
	if textbounce.pos[2]<=0     then textbounce.pos[2]=0     ; textbounce.vel[2]= math.abs(textbounce.vel[2]) end
	if textbounce.pos[1]>=tx-sx then textbounce.pos[1]=tx-sx ; textbounce.vel[1]=-math.abs(textbounce.vel[1]) end
	if textbounce.pos[2]>=ty-1  then textbounce.pos[2]=ty-1  ; textbounce.vel[2]=-math.abs(textbounce.vel[2]) end

	textbounce:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
textbounces.item.draw=function(textbounce)

	textbounce:get_values()

    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=0
    local fg=system.ticks%32 -- cycle the foreground color

	local px=math.floor(0.5+textbounce.pos[1])
	local py=math.floor(0.5+textbounce.pos[2])

	ctext.text_print(textbounce.text,px,py,fg,bg) -- (text,x,y,color,background)

end

--------------------------------------------------------------------------------
--
-- a player

players={}
-- methods added to system
players.system={}
-- methods added to each item
players.item={}

players.caste="player"

players.uidmap={
	length=0,
}

players.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 0,0,0 ),
	ang=V3( 0,0,0 ),
	idx=1,
}

players.types={
	pos="tween",
	rot="tween",
	vel="get",
	ang="get",
	idx="get",
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
. r r 7 r r r 7 r r r r O r r R 
. R r r 7 7 7 r r r R r O r r R 
. . R r r r r r r R R R R R . . 
. . R R r r r r R R R R R R . . 
. . . R R R R R R R R R R . . . 
. . . . R R R R R R R R . . . . 
. . . . . O O R R O O . . . . . 
. . . . r r r . r r r . . . . . 
]]},

{nil,"ply2",[[
. . . . . . . . . . . . . . . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G 7 0 G 7 0 G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 G 7 G 7 G g G d G G . 
. . . G G 7 G 7 G G g G d G . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g G G g g G G g . . . . 
. . . . . d d . . d d . . . . . 
. . . . G G G . G G G . . . . . 
]]},

}

players.sprite=function(sname,pos)
	local spr=system.components.tiles.names[sname]
	system.components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=pos[1] , py=pos[2] , pz=0 ,
		rz=0,
	})
end

players.item.get_values=function(player)

	player:get_auto_values()
--	player:get_body_values()

end

players.item.set_values=function(player)

	player:set_auto_values()
--	player:set_body_values()

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

	player:set_values()
end

players.item.update=function(player)
	player:get_values()

	player.pos=player.pos+player.vel

	player:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
players.item.draw=function(player)

	player:get_values()

	players.sprite( "ply"..player.idx , player.pos )

end


--------------------------------------------------------------------------------

-- lock globals to help catch future accidents
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
