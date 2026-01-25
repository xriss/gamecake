--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

local bitdown=require("wetgenes.gamecake.fun.bitdown")

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
			tile_size={4,8},
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
		require("wetgenes.gamecake.zone.flat.kinetic"):import(),
		levels,
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
			{"kinetic"},
			{"level",idx=1},
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
	acc=V3( 0,200,0 ),
	idx=1,
	side=1,
	foot=8,
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
. . . . . . R R R R . . . . . . 
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
. . . g g g g g g g g g . . . . 
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

}

players.sprite=function(sname,pos,side)
	local spr=system.components.tiles.names[sname]
	system.components.sprites.list_add({
		t=spr.idx ,
		hx=spr.hx , hy=spr.hy ,
		ox=(spr.hx)/2 , oy=(spr.hy)/2 ,
		px=pos[1] , py=pos[2] , pz=0 ,
		sx=side,
		rz=0,
	})
end

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

	local space=player:get_singular("kinetic").space
	player.body=space:body(1,1)
	player.shape=player.body:shape("circle",4,0,0)
	player.shape:friction(0.5)
	player.shape:elasticity(0.5)
	player.shape:filter(player.idx,0x00010000,0xffffffff)


	player:set_values()
end

players.item.update=function(player)
	player:get_values()

	local up=player.scene.ups[player.idx] or player.sys.oven.ups.empty

	local lx=( up:axis("lx") ) or 0
	local ly=( up:axis("ly") ) or 0

	player.acc=V3( lx*256 , ly*256 ,0) -- reset force
	
	if lx<0 then player.side= 1 end
	if lx>0 then player.side=-1 end

--	player.pos=player.pos+player.vel

	local footspeed=0.25

	local space=player:get_singular("kinetic").space
	local hit=space:query_segment_first(player.pos[1],player.pos[2],player.pos[1],player.pos[2]+16,2,player.idx)
	if hit and hit.alpha and hit.alpha<0.75 then

		player.vel[2]=player.vel[2]*7/8 --  dampen vertical velocity
		
		local d=(hit.alpha*16)+2 -- distance + radius
		local o=player.vel[2] -- original velocity
		local v=((d-9)) -- distance to where we want to be
		local a=v*32 -- force to adjust velocity by

		player.foot=d
		if player.foot<7  then player.foot=7  end
		if player.foot>11 then player.foot=11 end
		player.acc[2]=player.acc[2]+a --  hover
	else
		if player.foot>11 then player.foot=player.foot-footspeed end
		if player.foot<11 then player.foot=player.foot+footspeed end

		player.acc[2]=player.acc[2]+200 -- gravity
	end
	

	player:set_body() -- then we call update_kinetic which will set_values before draw
end

players.item.update_kinetic=function(player)
	player:get_body()
	player:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
players.item.draw=function(player)

	player:get_values()

	players.sprite( "ply"..player.idx          , player.pos , player.side )
	players.sprite( "ply"..player.idx.."_feet" , player.pos+V3(0,player.foot-8,0) , player.side )

end


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
	idx=1,
}

levels.types={
	pos="tween",
	idx="get",
}


levels.graphics={

{nil,"char_empty",[[
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
I I I I I I I I 
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
7 7 7 7 7 7 7 7 
7 0 0 0 0 0 0 7 
7 0 0 0 0 0 0 7 
7 0 0 0 0 0 0 7 
7 0 0 0 0 0 0 7 
7 0 0 0 0 0 0 7 
7 0 0 0 0 0 0 7 
7 7 7 7 7 7 7 7 
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
	["00"]={ name="char_black",				solid=1, dense=1, },		-- black border
	["0 "]={ name="char_solid",				solid=1, dense=1, },		-- empty border
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
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . 0 0 0 . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . 0 0 0 . . . . . . 0 
0 . . . . . . . . 0 0 0 . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
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
		end
	end

	level:set_values()
end

levels.item.update=function(level)
	level:get_values()


	level:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
levels.item.draw=function(level)

	level:get_values()

end

--------------------------------------------------------------------------------

-- lock globals to help catch future accidents
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
