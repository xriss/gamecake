
local chatdown=require("wetgenes.gamecake.fun.chatdown")	-- conversation trees
local bitdown=require("wetgenes.gamecake.fun.bitdown")		-- ascii to bitmap
local chipmunk=require("wetgenes.chipmunk")					-- 2d physics https://chipmunk-physics.net/

-- debug text dump
local ls=function(t) print(require("wetgenes.string").dump(t)) end

-----------------------------------------------------------------------------
--[[#hardware

select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.

]]
-----------------------------------------------------------------------------
oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	graphics=function() return graphics end,
	update=function() update() end, -- called repeatedly to update
	draw=function() draw() end, -- called repeatedly to draw
})

-----------------------------------------------------------------------------
--[[#graphics


define all graphics in this global, we will convert and upload to tiles 
at setup although you can change tiles during a game, we try and only 
upload graphics during initial setup so we have a nice looking sprite 
sheet to be edited by artists

]]
-----------------------------------------------------------------------------
graphics={
{0x0000,"_font",0x0140}, -- allocate the font area
}

-- load a single sprite
graphics.load=function(idx,name,data)
	local found
	for i,v in ipairs(graphics) do
		if v[2]==name then
			found=v
			break
		end
	end
	if not found then -- add new graphics
		graphics[#graphics+1]={idx,name,data}
	else
		found[1]=idx
		found[2]=name
		found[3]=data
	end
end	

-- load a list of sprites
graphics.loads=function(tab)
	for i,v in ipairs(tab) do
		graphics.load(v[1],v[2],v[3])
	end
end

-----------------------------------------------------------------------------
--[[#entities

	entities.reset()
	
empty the list of entites to update and draw

	entities.caste(caste)

get the list of entities of a given caste, eg "bullets" or "enemies"

	entities.add(it,caste)
	entities.add(it)

add a new entity of caste or it.caste to the list of things to update 

	entities.call(fname,...)

for every entity call the function named fname like so it[fname](it,...)



	entities.get(name)

get a value previously saved, this is an easy way to find a unique 
entity, eg the global space but it can be used to save any values you 
wish not just to bookmark unique entities.

	entities.set(name,value)

save a value by a unique name

	entities.manifest(name,value)

get a value previously saved, or initalize it to the given value if it 
does not already exist. The default value is {} as this is intended for 
lists.



	entities.systems

A table to register or find a global system, these are not cleared by 
reset and should not contain any state data, just functions to create 
the actual entity or initialise data.



	entities.tiles

These functions are called as we generate a level from ascii, every 
value in the tile legend data is checked against all the strings in 
entities.tiles and if it matches it calls that function which is then 
responsible for adding the appropriate collision and drawing code to 
make that tile actually add something to the level.

The basic values of tile.tile and tile.back are used to write graphics 
into the two tile layers but could still be caught here if you need to.

Multiple hooks may get called for a single tile, think of each string 
as a flag to signal that something happens and its value describes what 
happens.


]]
-----------------------------------------------------------------------------
entities={systems={},tiles={}} -- a place to store everything that needs to be updated

entities.reset=function()
	entities.data={}
	entities.info={}
end

-- get items for the given caste
entities.caste=function(caste)
	caste=caste or "generic"
	if not entities.data[caste] then entities.data[caste]={} end -- create on use
	return entities.data[caste]
end

-- add an item to this caste
entities.add=function(it,caste)
	caste=caste or it.caste -- probably from item
	caste=caste or "generic"
	local items=entities.caste(caste)
	items[ #items+1 ]=it -- add to end of array
	return it
end

-- call this functions on all items in every caste
entities.call=function(fname,...)
	local count=0
	for caste,items in pairs(entities.data) do
		for idx=#items,1,-1 do -- call backwards so item can remove self
			local it=items[idx]
			if it[fname] then
				it[fname](it,...)
				count=count+1
			end
		end			
	end
	return count -- number of items called
end

-- get/set info associated with this entities
entities.get=function(name)       return entities.info[name]							end
entities.set=function(name,value)        entities.info[name]=value	return value	end
entities.manifest=function(name,empty)
	if not entities.info[name] then entities.info[name]=empty or {} end -- create empty
	return entities.info[name]
end

-- also reset the entities right now
entities.reset()



-----------------------------------------------------------------------------
--[[#entities.systems.space

	space = entities.systems.space.setup()

Create the space that simulates all of the physics.

]]
-----------------------------------------------------------------------------
entities.systems.space={
setup=function()

	local space=entities.set("space", chipmunk.space() )
	
	space:gravity(0,700)
	space:damping(0.5)
	space:sleep_time_threshold(1)
	space:idle_speed_threshold(10)

	-- run all arbiter space hooks that have been registered
	for n,v in pairs(entities.systems) do
		if v.space then v:space() end
	end

	return space
end,
}

-----------------------------------------------------------------------------
--[[#entities.systems.tile

setup background tile graphics

]]
-----------------------------------------------------------------------------
entities.systems.tile={

load=function() graphics.loads{

{nil,"tile_empty",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{nil,"tile_black",[[
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
]]},
{nil,"tile_wall",[[
O O R R R R O O 
O O R R R R O O 
r r r r o o o o 
r r r r o o o o 
R R O O O O R R 
R R O O O O R R 
o o o o r r r r 
o o o o r r r r 
]]},
{nil,"tile_floor",[[
j j j j j j j j j j j j j j j j j f f f f f f f f j j j j j j j j j j j j j j j 
f f f F F F F f f f f f f f f f f F F F F F F F F f f f f f f j j j j j f f f f 
F F F f f f f F F F F F F F F F F f f f f f f f f F F F F F F f f f f f F F F F 
f f f f f f f f f f F F F F f f f f f f f f f f f f f f f f f F F F F F f f f f 
f f f j j j j f f f f f f f f f f j j j j j j j j f f f f f f f f f f f f f f f 
j j j f f f f j j j f f f f j j j f f f f f f f f j j j j j j f f f f f j j j j 
f f f j j j j f f f j j j j f f f j j j j j j j j f f f f f f j j j j j f f f f 
j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j j 
]]},
{nil,"tile_bigwall",[[
j j j j r r r r i i i i f f f f r r r r i i i i i i i i f f f f r r r r f f f f 
j j j j r r r r i i i i f f f f r r r r i i i i i i i i f f f f r r r r f f f f 
O O R R R R j j j j O O O O r r r r O O O O f f f f F F F F O O O O f f f f O O 
O O R R R R j j j j O O O O r Y Y r O O O O f Y f f F F F F O O O O f f f f O O 
j j j j R R R R r r r r R R Y Y Y j j j R R R Y i i i i R R R R O O O O R R R R 
j j j j R R R R r r r r R Y Y Y Y Y Y Y Y Y Y Y Y i i i R R R R O O O O R R R R 
r r O O O O j j j j R R Y Y j Y Y Y Y Y Y R j Y Y Y R R R R j j j j R R R R r r 
r r O O O O j j j j R R Y R j Y Y Y Y R R R j Y Y Y Y Y R R j j j j R R R R r r 
i i i i f f f f r r r r f f f Y Y j j j r r r r i Y Y Y f f f f r r r r i i i i 
i i i i f f f f r r r r f f f Y j j j j r r r r i Y Y Y Y f f f r r r r i i i i 
f f F F F F O O O O f f f f O Y O O R R R R j j j j Y Y Y O r r r r O O O O f f 
f f F F F F O O O O f f f f O Y O O R R R R j j j j O O O O r r r r O O O O f f 
i i i i R R R R O O O O R R R R j j j j R R R R r r r r R R R R j j j j R R R R 
i i i i R R R R O O O O R R R R j j j j R R R R r r r r R R R R j j j j R R R R 
j j R R R R j j j j R R R R r r r r O O O O j j j j R R R R j j j j R R R R j j 
j j R R R R j j j j R R R R r r r r O O O O j j j j R R R R j j j j R R R R j j 
]]},

{nil,"tile_grass",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. G . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
g G g . . . . . . . . . . . . . g . . . . . . . . g . g . . . . 
g G G . G . g . g . G . . . . g . . . g g . g . . G . . g . . g 
]]},

{nil,"tile_stump",[[
. . F F F F . . 
f F f f f f F f 
j f F F F F f j 
j j f f f j f j 
j f f f j j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
]]},

{nil,"tile_sidewood",[[
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
j f F f f j f j 
]]},


{nil,"tile_tree",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . j . . . . . . 
. . . j . . . . . j . . . . . . 
. . . . f . . . j . . . . . . . 
. . . . f F . j . . . . . . . . 
. . . . . f F j . . j . . . . . 
. . . . . j f F . f . . . . . . 
. . . . . . . f . j . . . . . . 
. . . . . . . f F f . . . . . . 
. . . . . j F f f f . . . . . . 
. . . . . . f F j . . . . . . . 
. . . . . . j F j . . . . . . . 
. . . . . . j f j . . . . . . . 
. . . . . . . F j . . . . . . . 
. . . j F F . f f . . . . . . . 
. . . . j F F j f j . . . . . . 
. . . . . j f j f . . . . . . . 
. . . . . . f f F f j j . . . . 
. . . . . . j F f j . . . . . . 
. . . . . . . f F . . . . . . . 
. . . . . . . f f . . . . . . . 
. . . . . . . j F . . . . . . . 
. . . . . . f f F f . . . . . . 
]]},


{nil,"tile_sign",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . f . . . 
. . . . . . . . . . . . F f . . 
. . f f f 5 f f f 5 f f f F f . 
. . f F 4 F F F 4 F 4 F F f F f 
. . f j 3 j 3 j 3 j 3 j j f F f 
. . f j j 2 2 j j 2 j j f F f . 
. . . . . . j j j . . . F f . . 
. . . . . . F j j . . . f . . . 
. . . . . . F f j . . . . . . . 
. . . . . . F f j . . . . . . . 
. . . . . . F f j . . . . . . . 
. . . . . . F f j . . . . . . . 
]]},

{nil,"tile_postbox",[[
. m m m m m m . 
m R R R R R R m 
m m m m m m m f 
m R R R R R R f 
m R 0 0 0 0 R f 
m R R R R R R f 
m R 3 2 3 2 R f 
m R 2 3 2 3 R f 
m R R R R R R f 
m R R R R R R f 
m R R R R R R f 
m R R R R R f f 
m R R R R f R f 
R R R R f R f f 
f R R f R f f f 
. f f f f f f . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_deadly={} -- deadly things
		arbiter_deadly.presolve=function(it)
			local callbacks=entities.manifest("callbacks")
			if it.shape_b.player then -- trigger die
				local pb=it.shape_b.player
				callbacks[#callbacks+1]=function() pb:die() end
			end
			return true
		end
	space:add_handler(arbiter_deadly,space:type("deadly"))

	local arbiter_crumbling={} -- crumbling tiles
		arbiter_crumbling.presolve=function(it)
			local points=it:points()
	-- once we trigger headroom, we keep a table of headroom shapes and it is not reset until total separation
			if it.shape_b.in_body.headroom then
				local headroom=false
	--				for n,v in pairs(it.shape_b.in_body.headroom) do headroom=true break end -- still touching an old headroom shape?
	--				if ( (points.normal_y>0) or headroom) then -- can only headroom through non dense tiles
				if ( (points.normal_y>0) or it.shape_b.in_body.headroom[it.shape_a] ) then
					it.shape_b.in_body.headroom[it.shape_a]=true
					return it:ignore()
				end
				local tile=it.shape_a.tile -- a humanoid is walking on this tile
				if tile then
					tile.level.updates[tile]=true -- start updates to animate this tile crumbling away
				end
			end
			
			return true
		end
		arbiter_crumbling.separate=function(it)
			if it.shape_a and it.shape_b and it.shape_b.in_body then
				if it.shape_b.in_body.headroom then -- only players types will have headroom
					it.shape_b.in_body.headroom[it.shape_a]=nil
				end
			end
		end
	space:add_handler(arbiter_crumbling,space:type("crumbling"))

end,

}

-----------------------------------------------------------------------------
--[[#entities.systems.item

	item = entities.systems.item.add()

items, can be used for general things, EG physics shapes with no special actions

]]
-----------------------------------------------------------------------------
entities.systems.item={

load=function() graphics.loads{

{nil,"cannon_ball",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . O O O O . . . . . . . . . . 
. . . . . . . R O O O O O O O O R . . . . . . . 
. . . . . . R R R O O O O O O R R R . . . . . . 
. . . . . R R R R O O O O O O R R R R . . . . . 
. . . . . 5 R R R R O O O O R R R R c . . . . . 
. . . . . 5 5 5 R R O O O O R R c c c . . . . . 
. . . . 5 5 5 5 5 5 R 0 0 R c c c c c c . . . . 
. . . . 5 5 5 5 5 5 0 0 0 0 c c c c c c . . . . 
. . . . 5 5 5 5 5 5 0 0 0 0 c c c c c c . . . . 
. . . . 5 5 5 5 5 5 R 0 0 R c c c c c c . . . . 
. . . . . 5 5 5 R R o o o o R R c c c . . . . . 
. . . . . 5 R R R R o o o o R R R R c . . . . . 
. . . . . R R R R o o o o o o R R R R . . . . . 
. . . . . . R R R o o o o o o R R R . . . . . . 
. . . . . . . R o o o o o o o o R . . . . . . . 
. . . . . . . . . . o o o o . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},

}end,

space=function()


end,

add=function()
	local item=entities.add{caste="item"}
	item.draw=function()
		if item.active then
			local px,py,rz=item.px,item.py,item.rz
			if item.body then -- from fizix
				px,py=item.body:position()
				rz=item.body:angle()
			end
			rz=item.draw_rz or rz -- always face up?
			system.components.sprites.list_add({t=item.sprite,h=item.h,hx=item.hx,hy=item.hy,s=item.s,sx=item.sx,sy=item.sy,px=px,py=py,rz=180*rz/math.pi,color=item.color,pz=item.pz})
		end
	end
	return item
end,
}


-----------------------------------------------------------------------------
--[[#entities.systems.score

	score = entities.systems.score.setup()

Create entity that handles the score hud update and display

]]
-----------------------------------------------------------------------------
entities.systems.score={

space=function()

	local space=entities.get("space")
	local arbiter_loot={} -- loot things (pickups)
		arbiter_loot.presolve=function(it)
			if it.shape_a.loot and it.shape_b.player then -- trigger collect
				it.shape_a.loot.player=it.shape_b.player
			end
			return false
		end
	space:add_handler(arbiter_loot,space:type("loot")) 

end,

setup=function()

	local score=entities.set("score",entities.add{})
	
	entities.set("time",{
		game=0,
	})
	
	score.update=function()
		local time=entities.get("time")
		time.game=time.game+(1/60)
	end

	score.draw=function()
	
--[[
		local time=entities.get("time")
		local remain=0
		for _,loot in ipairs( entities.caste("loot") ) do
			if loot.active then remain=remain+1 end -- count remaining loots
		end
		if remain==0 and not time.finish then -- done
			time.finish=time.game
		end

		local t=time.start and ( (time.finish or time.game) - ( time.start ) ) or 0
		local ts=math.floor(t)
		local tp=math.floor((t%1)*100)

		local s=string.format("%d.%02d",ts,tp)
		system.components.text.text_print(s,math.floor((system.components.text.tilemap_hx-#s)/2),0)
]]

		local s=""
		
		local level=entities.get("level")
		
		s=level.title or s

		for i,player in pairs(entities.caste("player")) do
			if player.near_menu then
			
			
				s=player.near_menu.title
			end

			if player.near_npc then
				local chats=entities.get("chats")
				local chat=chats:get_subject(player.near_npc.name)
				s=chat:get_tag("title") or s
			end
		end

		system.components.text.text_print(s,math.floor((system.components.text.tilemap_hx-#s)/2),system.components.text.tilemap_hy-1)
		
	end
	
	return score
end,
}

-----------------------------------------------------------------------------
--[[#entities.systems.player

	player = entities.systems.player.add(idx)

Add a player, level should be setup before calling this

]]
-----------------------------------------------------------------------------
entities.systems.player={

load=function() graphics.loads{

-- 4 x 16x32
{nil,"skel_walk_4",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 7 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 7 7 7 . . . . . 
. . . . . . 7 7 7 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 7 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . 
. . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 0 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 0 7 7 . . . . . 
. . . . . . 7 7 0 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 0 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . 
. . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . 7 7 7 . 7 . 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . 7 7 . . . . . . . . 7 . . 7 7 7 7 7 . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . . . . 7 . . 7 7 7 7 . 7 . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . 7 . 7 . 7 . . 7 . . . . . . . . 7 . . 7 7 7 7 . 7 . . . . . . . 7 . 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . . 7 . . . . . . . . 7 . . 7 7 7 7 7 . . . . . . . . 7 . 7 . 7 . . 7 7 . . . . . . . 7 . . 7 7 7 7 . 7 . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . . . 7 7 . . 7 7 7 7 . 7 7 . . . . . . 7 7 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . . 7 . . . . . . . . . 7 . 7 7 7 7 . 7 . . . . . . 7 . . 7 . 7 . . 7 . 7 . . . . . . . 7 . 7 7 7 7 7 . . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . . 7 7 . 7 7 . 7 7 . . . . . . 7 . . . 7 7 7 7 . . 7 . . . . . . . 7 7 . 7 7 . 7 . . . . . 
. . . . 7 7 . 7 7 . 7 . . . . . . . . . 7 7 7 7 7 7 7 7 7 . . . . . 7 . . 7 . 7 7 . 7 . 7 . . . . . . . 7 7 7 7 7 7 7 . . . . . 
. . . . 7 7 7 7 7 7 7 . . . . . . . . . 7 7 7 7 . 7 . 7 7 . . . . . 7 7 . 7 7 7 7 7 7 7 7 . . . . . . . 7 7 7 . . 7 7 . . . . . 
. . . . 7 7 7 . . 7 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . 7 7 . . 7 . . 7 . 7 7 . . . . . . . . . 7 . . . 7 7 . . . . 
. . . . . 7 . . . . 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 . . 7 . . . . . . . . . . . . 7 . . . . 7 7 . . . 
. . . . 7 7 . . . . 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 . . . . 7 7 . . . 
. . . . 7 7 . . . . 7 7 . . . . . . . . . . 7 . . 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 . . . 7 . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . . 7 . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . . 7 . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . 7 . . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . 7 . . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . 7 . . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . 7 . . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . 7 7 7 7 . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . 7 7 7 7 . . . . 
. . . 7 7 7 7 . . 7 7 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 7 7 7 7 . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_pass={}  -- background tiles we can jump up through
		arbiter_pass.presolve=function(it)
			local points=it:points()
-- once we trigger headroom, we keep a table of headroom shapes and it is not reset until total separation
			if it.shape_b.in_body.headroom then
				local headroom=false
--					for n,v in pairs(it.shape_b.in_body.headroom) do headroom=true break end -- still touching an old headroom shape?
--					if ( (points.normal_y>0) or headroom) then -- can only headroom through non dense tiles
				if ( (points.normal_y>0) or it.shape_b.in_body.headroom[it.shape_a] ) then
					it.shape_b.in_body.headroom[it.shape_a]=true
					return it:ignore()
				end
			end
			
			return true
		end
		arbiter_pass.separate=function(it)
			if it.shape_a and it.shape_b and it.shape_b.in_body then
				if it.shape_b.in_body.headroom then it.shape_b.in_body.headroom[it.shape_a]=nil end
			end
		end
	space:add_handler(arbiter_pass,space:type("pass"))
	
	local arbiter_walking={} -- walking things (players)
		arbiter_walking.presolve=function(it)
			local callbacks=entities.manifest("callbacks")
			if it.shape_a.player and it.shape_b.monster then
				local pa=it.shape_a.player
				callbacks[#callbacks+1]=function() pa:die() end
			end
			if it.shape_a.monster and it.shape_b.player then
				local pb=it.shape_b.player
				callbacks[#callbacks+1]=function() pb:die() end
			end
			if it.shape_a.player and it.shape_b.player then -- two players touch
				local pa=it.shape_a.player
				local pb=it.shape_b.player
				if pa.active then
					if pb.bubble_active and pb.joined then -- burst
						callbacks[#callbacks+1]=function() pb:join() end
					end
				end				
				if pb.active then
					if pa.bubble_active and pa.joined then -- burst
						callbacks[#callbacks+1]=function() pa:join() end
					end
				end				
			end
			return true
		end
		arbiter_walking.postsolve=function(it)
			local points=it:points()
			if points.normal_y>0.25 then -- on floor
				local time=entities.get("time")
				it.shape_a.in_body.floor_time=time.game
				it.shape_a.in_body.floor=it.shape_b
			end
			return true
		end
	space:add_handler(arbiter_walking,space:type("walking")) -- walking things (players)
	
	local arbiter_trigger={} -- trigger things
		arbiter_trigger.presolve=function(it)
			if it.shape_a.trigger and it.shape_b.triggered then -- trigger something
				it.shape_b.triggered.triggered = it.shape_a.trigger
			end
			return false
		end
	space:add_handler(arbiter_trigger,space:type("trigger"))

end,

-----------------------------------------------------------------------------
--[[#entities.systems.player

	entities.systems.player.controls(it,fast)

Handle player style movement, so we can reuse this code for player 
style monsters. it is a player or monster, fast lets us tweak the speed 
and defaults to 1

movement controls are set in it

it.move which is "left" or "right" to move left or right
it.jump which is true if we should jump

]]
-----------------------------------------------------------------------------
controls=function(it,fast)
	fast=fast or 1
	
	local menu=entities.get("menu")
	local chats=entities.get("chats")

	local time=entities.get("time")

	local jump=fast*200 -- up velocity we want when jumping
	local speed=fast*60 -- required x velocity
	local airforce=speed*2 -- replaces surface velocity
	local groundforce=speed/2 -- helps surface velocity
	
	if ( time.game-it.body.floor_time < 0.125 ) or ( it.floor_time-time.game > 10 ) then -- floor available recently or not for a very long time (stuck)
	
		it.floor_time=time.game -- last time we had some floor

		it.shape:friction(1)

		if it.jump_clr and it.near_menu then
			local menu=entities.get("menu")
			local near_menu=it.near_menu
			local callbacks=entities.manifest("callbacks")
			callbacks[#callbacks+1]=function() menu.show(near_menu) end -- call later so we do not process menu input this frame
		end

		if it.jump_clr and it.near_npc then

			local callbacks=entities.manifest("callbacks")
			callbacks[#callbacks+1]=function()

				local chat=chats:get_subject(subject_name)
				chat:set_topic("welcome")
				menu.show( menu.chat_to_menu_items(chat) )

			end -- call later so we do not process menu input this frame

		end

		if it.jump then

			local vx,vy=it.body:velocity()

			if vy>-20 then -- only when pushing against the ground a little

				if it.near_menu or it.near_npc then -- no jump
				
				else
				
					vy=-jump
					it.body:velocity(vx,vy)
					
					it.body.floor_time=0
				
				end
				
			end

		end

		if it.move=="left" then
			
			local vx,vy=it.body:velocity()
			if vx>0 then it.body:velocity(0,vy) end
			
			it.shape:surface_velocity(speed,0)
			if vx>-speed then it.body:apply_force(-groundforce,0,0,0) end
			it.dir=-1
			it.frame=it.frame+1
			
		elseif it.move=="right" then

			local vx,vy=it.body:velocity()
			if vx<0 then it.body:velocity(0,vy) end

			it.shape:surface_velocity(-speed,0)
			if vx<speed then it.body:apply_force(groundforce,0,0,0) end
			it.dir= 1
			it.frame=it.frame+1

		else

			it.shape:surface_velocity(0,0)

		end
		
	else -- in air

		it.shape:friction(0)

		if it.move=="left" then
			
			local vx,vy=it.body:velocity()
			if vx>0 then it.body:velocity(0,vy) end

			if vx>-speed then it.body:apply_force(-airforce,0,0,0) end
			it.shape:surface_velocity(speed,0)
			it.dir=-1
			it.frame=it.frame+1
			
		elseif  it.move=="right" then

			local vx,vy=it.body:velocity()
			if vx<0 then it.body:velocity(0,vy) end

			if vx<speed then it.body:apply_force(airforce,0,0,0) end
			it.shape:surface_velocity(-speed,0)
			it.dir= 1
			it.frame=it.frame+1

		else

			it.shape:surface_velocity(0,0)

		end

	end
end,

add=function(i)
	local players_colors={[0]=30,30,14,18,7,3,22}

	local names=system.components.tiles.names
	local space=entities.get("space")

	local player=entities.add{caste="player"}

	player.idx=i
	player.score=0
	
	local t=bitdown.cmap[ players_colors[i] ]
	player.color={}
	player.color.r=t[1]/255
	player.color.g=t[2]/255
	player.color.b=t[3]/255
	player.color.a=t[4]/255
	player.color.idx=players_colors[i]
	
	player.up_text_x=math.ceil( (system.components.text.tilemap_hx/16)*( 1 + ((i>3 and i+2 or i)-1)*2 ) )

	player.frame=0
	player.frames={ names.skel_walk_4.idx+0 , names.skel_walk_4.idx+2 , names.skel_walk_4.idx+4 , names.skel_walk_4.idx+6 }
	
	player.join=function()
		local players_start=entities.get("players_start") or {64,64}
	
		local px,py=players_start[1]+i,players_start[2]
		local vx,vy=0,0

		player.bubble_active=false
		player.active=true
		player.body=space:body(1,math.huge)
		player.body:position(px,py)
		player.body:velocity(vx,vy)
		player.body.headroom={}
		
		player.body:velocity_func(function(body)
--				body.gravity_x=-body.gravity_x
--				body.gravity_y=-body.gravity_y
			return true
		end)
					
		player.floor_time=0 -- last time we had some floor

		player.shape=player.body:shape("segment",0,2,0,11,4)
		player.shape:friction(1)
		player.shape:elasticity(0)
		player.shape:collision_type(space:type("walking")) -- walker
		player.shape.player=player
		
		player.body.floor_time=0
		local time=entities.get("time")
		if not time.start then
			time.start=time.game -- when the game started
		end
	end
	
	player.update=function()
		local up=ups(player.idx) -- the controls for this player
		
		player.move=false
		player.jump=up.button("fire")
		player.jump_clr=up.button("fire_clr")

		if use_only_two_keys then -- touch screen control test?

			if up.button("left") and up.button("right") then -- jump
				player.move=player.move_last
				player.jump=true
			elseif up.button("left") then -- left
				player.move_last="left"
				player.move="left"
			elseif up.button("right") then -- right
				player.move_last="right"
				player.move="right"
			end

		else

			if up.button("left") and up.button("right") then -- stop
				player.move=nil
			elseif up.button("left") then -- left
				player.move="left"
			elseif up.button("right") then -- right
				player.move="right"
			end

		end


		if not player.joined then
			player.joined=true
			player:join() -- join for real and remove bubble
		end

		if player.active then
		
			entities.systems.player.controls(player)
		
		end
	end
	

	player.draw=function()
		if player.active then
			local px,py=player.body:position()
			local rz=player.body:angle()
			player.frame=player.frame%16
			local t=player.frames[1+math.floor(player.frame/4)]
			
			system.components.sprites.list_add({t=t,hx=16,hy=32,px=px,py=py,sx=player.dir,sy=1,rz=180*rz/math.pi,color=player.color})			
		end

--		if player.joined then
--			local s=string.format("%d",player.score)
--			system.components.text.text_print(s,math.floor(player.up_text_x-(#s/2)),0,player.color.idx)
--		end

	end
	
	return player
end,
}


-----------------------------------------------------------------------------
--[[#entities.systems.npc

	npc = entities.systems.npc.add(opts)

Add an npc.

]]
-----------------------------------------------------------------------------
entities.systems.npc={

chat_text=[[

#npc1

	=title A door shaped like a dead man.

	=donuts 0

<welcome

	Good Day Kind Sir,
	
	If you fetch me all the donuts I will let you out.
	
	>exit?donuts<10

		Fine I'll go look.

	>donut?donuts>9

		Here you go, 10 donuts that have barely touched the ground.

	>out

		Where is out?
<out

	Out is nearby, I may have been here a long long time but I know 
	exactly how to get out and I can show you, just bring me donuts, 
	all of them!
	
	>exit
	
		Be seeing you.
	
]],
chat_hook_topic=function(chat,a)
	if chat.subject_name=="npc1" then
		if a.name=="exit" then
			if not entities.get("added_donuts") then
				entities.set("added_donuts",true)
				entities.systems.donut.spawn(10)
			end
		end
		if a.name=="donut" then
			entities.systems.level.setup(2)
		end
	end
end,

load=function() graphics.loads{

-- 4 x 16x32
{nil,"npc1_walk_4",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 7 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 7 7 7 . . . . . 
. . . . . . 7 7 7 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 7 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . 
. . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 0 7 7 . . . . . . . . . . . 7 0 7 0 7 . . . . . . . . . . . 7 7 0 7 7 . . . . . 
. . . . . . 7 7 0 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 7 0 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . 
. . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . . 7 0 7 . . . . . . . . . . . . . 7 7 7 . . . . . . 
. . . . . . . 7 7 7 . . . . . . . . . 7 7 7 . 7 . 7 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . 7 7 . . . . . . . . 7 . . 7 7 7 7 7 . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . . . . 7 . . 7 7 7 7 . 7 . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . 7 . 7 . 7 . . 7 . . . . . . . . 7 . . 7 7 7 7 . 7 . . . . . . . 7 . 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . . 7 . . . . . . . . 7 . . 7 7 7 7 7 . . . . . . . . 7 . 7 . 7 . . 7 7 . . . . . . . 7 . . 7 7 7 7 . 7 . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . 7 7 7 . 7 . . 7 7 . . . . . . 7 7 . . 7 7 7 7 . 7 7 . . . . . . 7 7 7 . 7 . . 7 7 . . . . 
. . . . 7 7 . 7 . . 7 . . . . . . . . . 7 . 7 7 7 7 . 7 . . . . . . 7 . . 7 . 7 . . 7 . 7 . . . . . . . 7 . 7 7 7 7 7 . . . . . 
. . . . 7 . 7 7 7 7 7 . . . . . . . . . 7 7 . 7 7 . 7 7 . . . . . . 7 . . . 7 7 7 7 . . 7 . . . . . . . 7 7 . 7 7 . 7 . . . . . 
. . . . 7 7 . 7 7 . 7 . . . . . . . . . 7 7 7 7 7 7 7 7 7 . . . . . 7 . . 7 . 7 7 . 7 . 7 . . . . . . . 7 7 7 7 7 7 7 . . . . . 
. . . . 7 7 7 7 7 7 7 . . . . . . . . . 7 7 7 7 . 7 . 7 7 . . . . . 7 7 . 7 7 7 7 7 7 7 7 . . . . . . . 7 7 7 . . 7 7 . . . . . 
. . . . 7 7 7 . . 7 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . 7 7 . . 7 . . 7 . 7 7 . . . . . . . . . 7 . . . 7 7 . . . . 
. . . . . 7 . . . . 7 . . . . . . . . . . . . 7 7 7 . . . . . . . . . . . . 7 . . 7 . . . . . . . . . . . . 7 . . . . 7 7 . . . 
. . . . 7 7 . . . . 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 . . . . 7 7 . . . 
. . . . 7 7 . . . . 7 7 . . . . . . . . . . 7 . . 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . . . 7 7 . . . 7 . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . . 7 . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . . 7 . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . 7 . . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . 7 . . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . . 7 . . . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . . 7 . . . . . . 
. . . . 7 . . . . . 7 . . . . . . . . . 7 7 7 7 . 7 . . . . . . . . . . . . 7 . 7 . . . . . . . . . . . . 7 . . 7 7 7 7 . . . . 
. . . 7 7 7 7 . . 7 7 7 7 . . . . . . . . . . . 7 7 7 7 . . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 7 7 7 7 . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_npc={} -- npc menu things
		arbiter_npc.presolve=function(it)
			if it.shape_a.npc and it.shape_b.player then -- remember npc menu
				it.shape_b.player.near_npc=it.shape_a.npc
			end
			return false
		end
		arbiter_npc.separate=function(it)
			if it.shape_a and it.shape_a.npc and it.shape_b and it.shape_b.player then -- forget npc menu
				it.shape_b.player.near_npc=false
			end
			return true
		end
	space:add_handler(arbiter_npc,space:type("npc"))

end,


add=function(opts)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local npc=entities.add{caste="npc"}

	npc.frame=0
	npc.frames={ names.npc1_walk_4.idx+0 , names.npc1_walk_4.idx+2 , names.npc1_walk_4.idx+4 , names.npc1_walk_4.idx+6 }
		
	npc.update=function()

		npc.move=false
		npc.jump=false
		npc.jump_clr=false

		if npc.active then
		
			entities.systems.player.controls(npc)
		
		end
	end
	

	npc.draw=function()
		if npc.active then
			local px,py=npc.body:position()
			local rz=npc.body:angle()
			local t=npc.frames[1]
			
			system.components.sprites.list_add({t=t,hx=16,hy=32,px=px,py=py,sx=npc.dir,sy=1,rz=180*rz/math.pi,color=npc.color})			
		end

	end
	

	local px,py=opts.px,opts.py
	local vx,vy=0,0
	
	npc.dir=-1
	npc.color={r=1/2,g=1,b=1/2,a=1}

	npc.active=true
	npc.body=space:body(1,math.huge)
	npc.body:position(px,py)
	npc.body:velocity(vx,vy)
	npc.body.headroom={}
	
	npc.body:velocity_func(function(body)
--				body.gravity_x=-body.gravity_x
--				body.gravity_y=-body.gravity_y
		return true
	end)
				
	npc.floor_time=0 -- last time we had some floor

	npc.shape=npc.body:shape("segment",0,2,0,11,4)
	npc.shape:friction(1)
	npc.shape:elasticity(0)
	npc.shape:collision_type(space:type("walking")) -- walker

	npc.shape2=npc.body:shape("segment",0,2,0,11,8) -- talk area
	npc.shape2:collision_type(space:type("npc"))
	npc.shape2.npc=npc
	
	npc.body.floor_time=0
	
	npc.name="npc1"


	return npc
end,
}



-----------------------------------------------------------------------------
--[[#entities.systems.donut

	donut = entities.systems.donut.add(opts)

Add an donut.

]]
-----------------------------------------------------------------------------
entities.systems.donut={

load=function() graphics.loads{

-- 1 x 24x24
{nil,"donut_1",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . O O O O . . . . . . . . . . 
. . . . . . . O O O O O O O O O O . . . . . . . 
. . . . . . O O O O O O O O O O O O . . . . . . 
. . . . . O O O O O O O O O O O O O O . . . . . 
. . . . . O O O O O O O O O O O O O O . . . . . 
. . . . . O O O O O O . . O O O O O O . . . . . 
. . . . O O O O O O . . . . O O O O O O . . . . 
. . . . O O O O O . . . . . . O O O O O . . . . 
. . . . O O O O O . . . . . . O O O O O . . . . 
. . . . O O O O O O . . . . O O O O O O . . . . 
. . . . . O O O O O O . . O O O O O O . . . . . 
. . . . . O O O O O O O O O O O O O O . . . . . 
. . . . . O O O O O O O O O O O O O O . . . . . 
. . . . . . O O O O O O O O O O O O . . . . . . 
. . . . . . . O O O O O O O O O O . . . . . . . 
. . . . . . . . . . O O O O . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")
	local arbiter_loot={} -- loot things (pickups)
		arbiter_loot.presolve=function(it)
			if it.shape_a.loot and it.shape_b.player then -- trigger collect
				local loot=it.shape_a.loot
				local player=it.shape_b.player
				local callbacks=entities.manifest("callbacks")
				callbacks[#callbacks+1]=function() loot:pickup(player) end
				return false
			end
			return true
		end
	space:add_handler(arbiter_loot,space:type("donut")) 


end,

spawn=function(count)

	for i=1,count do
		local px=math.random(32,320-32)
		local py=32
		local vx=math.random(-4000,4000)/100
		local vy=math.random(-4000,4000)/100
		entities.systems.donut.add({px=px,py=py,vx=vx,vy=vy})
	end
	
end,

add=function(opts)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local donut=entities.add{caste="donut"}

	donut.frame=0
	donut.frames={ names.donut_1.idx+0 }
		
	donut.update=function()
	end
	
	donut.draw=function()
		if donut.active then
			local px,py=donut.body:position()
			local rz=donut.body:angle()
			local t=donut.frames[1]
			system.components.sprites.list_add({t=t,h=24,px=px,py=py,rz=180*rz/math.pi})			
		end

	end

	donut.pickup=function(it,player)
		if donut.active then
			donut.active=false
			space:remove(donut.shape)
			space:remove(donut.body)
			
			local chats=entities.get("chats")
			chats:set_tag("npc1/donuts","+1")
		end
	end
		
	donut.active=true
	
	donut.body=space:body(1,1)
	donut.body:position(opts.px,opts.py)
	donut.body:velocity(opts.vx,opts.vy)
	donut.body.headroom={}
	

	donut.shape=donut.body:shape("circle",8,0,0)
	donut.shape:friction(1)
	donut.shape:elasticity(0)
	donut.shape:collision_type(space:type("donut"))
	donut.shape.loot=donut

	return donut
end,
}

----------------------------------------------------------------------------
--[[#entities.tiles.start

The player start point, just save the x,y

]]
-----------------------------------------------------------------------------
entities.tiles.start=function(tile)
	entities.set("players_start",{tile.x*8+4,tile.y*8+4}) --  remember start point
end

-----------------------------------------------------------------------------
--[[#entities.tiles.sprite

Display a sprite

]]
-----------------------------------------------------------------------------
entities.tiles.sprite=function(tile)

	local names=system.components.tiles.names

	local item=entities.systems.item.add()
	item.active=true
	item.px=tile.x*8+4
	item.py=tile.y*8+4
	item.sprite = names[tile.sprite].idx
	item.h=24
	item.s=1
	item.draw_rz=0
	item.pz=-1
end

-----------------------------------------------------------------------------
--[[#entities.tiles.npc

Display a npc

]]
-----------------------------------------------------------------------------
entities.tiles.npc=function(tile)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local item=entities.systems.npc.add({px=tile.x*8,py=tile.y*8})

end

-----------------------------------------------------------------------------
--[[#levels

Design levels here

]]
-----------------------------------------------------------------------------

local combine_legends=function(...)
	local legend={}
	for _,t in ipairs{...} do -- merge all
		for n,v in pairs(t) do -- shallow copy, right side values overwrite left
			legend[n]=v
		end
	end
	return legend
end

local default_legend={

	[0]={ tile="tile_empty",back="tile_empty",uvworld=true},

-- screen edges
	["00"]={ tile="tile_black",				solid=1, dense=1, },		-- black border
	["0 "]={ tile="tile_empty",				solid=1, dense=1, },		-- empty border


-- solid features
	["||"]={ solid=1, tile="tile_sidewood", },				-- wall
	["=="]={ solid=1, back="tile_floor",    },				-- floor
	["WW"]={ solid=1, tile="tile_bigwall",  },
	["S="]={ solid=1, tile="tile_stump",    },
	["P="]={ solid=1, tile="tile_postbox",  },

-- foreground features
	[",,"]={ back="tile_grass", },
	["t."]={ tile="tile_tree", },
	["s."]={ tile="tile_sign", },

-- special locations
	["S "]={ 	start=1,	},

-- items not tiles, so display tile 0 and we will add a sprite for display
	["N1"]={ 	npc="npc1",  },

}
	
levels={}

levels[1]={
legend=combine_legends(default_legend,{
	["?0"]={ },
}),
title="Welcome!",
map=[[
||0000000000000000000000000000000000000000000000000000000000000000000000000000||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . S . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||,,. . . . . . ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,. . . . . ||
||==. . . . . . ====================================================. . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . N1. . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||============================================================================||
||0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ||
]],
}

levels[2]={
legend=combine_legends(default_legend,{
	["?0"]={ },
}),
title="This is a test.",
map=[[
||0000000000000000000000000000000000000000000000000000000000000000000000000000||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . t.t.. . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . t.t.. . . . . . . s.s.. . . . . . . . . . . . . . . . . . ||
||,,,,,,,,,,,,,,,,,,t.t.,,,,,,,,,,,,,,s.s.,,,,,,,,. . . . ,,,,,,,,,,,,,,,,,,,,||
||================================================. . . . ====================||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||,,,,,,. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||======. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||,,,,,,,,,,. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||==========. . . . . . . . . . t.t.. . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . t.t.. . . . . . WWWWWWWWWWWW. . . . . . . . . ||
||,,,,,,,,,,,,,,. . . . ,,,,,,,,t.t.,,,,,,,,,,,,WWWWWWWWWWWW,,,,,,,,,,,,,,,,,,||
||==============. . . . ======================================================||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||,,. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||==. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||,,. . S . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||==. . . . . . . . . . . . . . . . . . . . . . S=. . . . . . . ,,,,,,,,,,,,,,||
||. . . . . . . . . . . . . . . . P=. . S=. . . S=. . . . . . . ==============||
||,,. . . . . . ,,,,,,,,,,S=,,,,,,P=,,,,S=,,,,,,S=,,,,,,,,,,,,,,,,,,. . . . . ||
||==. . . . . . ====================================================. . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||============================================================================||
||0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ||
]],
}

-----------------------------------------------------------------------------
--[[#entities.systems.level

	entities.systems.level.setup(level)

reset and setup everything for this level idx

]]
-----------------------------------------------------------------------------
entities.systems.level={
setup=function(idx)

	entities.reset()

	local menu=entities.systems.menu.setup()

	entities.systems.score.setup()

	local level=entities.set("level",entities.add{})

	local names=system.components.tiles.names

	level.updates={} -- tiles to update (animate)
	level.update=function()
		for v,b in pairs(level.updates) do -- update these things
			if v.update then v:update() end
		end
	end

-- init map and space


	local space=entities.systems.space.setup()

	local tilemap={}
	for n,v in pairs( levels[idx].legend ) do -- build tilemap from legend
		if v.tile then -- convert name to tile
			local t={}
			for n,v in pairs( v ) do t[n]=v end
			for n,v in pairs( names[v.tile] ) do t[n]=v end
			tilemap[n]=t
		end
	end

	local backmap={}
	for n,v in pairs( levels[idx].legend ) do -- build tilemap from legend
		if v.back then -- convert name to tile
			local t={}
			for n,v in pairs( v ) do t[n]=v end
			for n,v in pairs( names[v.back] ) do t[n]=v end
			backmap[n]=t
		end
	end

	local map=entities.set("map", bitdown.pix_tiles(  levels[idx].map,  levels[idx].legend ) )
	
	level.title=levels[idx].title
	
	bitdown.tile_grd( levels[idx].map, backmap, system.components.back.tilemap_grd  ) -- draw into the screen (tiles)
	bitdown.tile_grd( levels[idx].map, tilemap, system.components.map.tilemap_grd  ) -- draw into the screen (tiles)

	local unique=0
	bitdown.map_build_collision_strips(map,function(tile)
		unique=unique+1
		if tile.coll then -- can break the collision types up some more by appending a code to this setting
			if tile.unique then -- make unique
				tile.coll=tile.coll..unique
			end
		end
	end)

	for y,line in pairs(map) do
		for x,tile in pairs(line) do

			tile.map=map -- remember map
			tile.level=level -- remember level

			if tile.solid and (not tile.parent) then -- if we have no parent then we are the master tile
			
				local l=1
				local t=tile
				while t.child do t=t.child l=l+1 end -- count length of strip

				local shape

				if     tile.link==1 then -- x strip
					shape=space.static:shape("box",x*8,y*8,(x+l)*8,(y+1)*8,0)
				elseif tile.link==-1 then  -- y strip
					shape=space.static:shape("box",x*8,y*8,(x+1)*8,(y+l)*8,0)
				else -- single box
					shape=space.static:shape("box",x*8,y*8,(x+1)*8,(y+1)*8,0)
				end
				tile.shape=shape
				shape.tile=tile

				shape:friction(tile.solid)
				shape:elasticity(tile.solid)
				shape.cx=x
				shape.cy=y
				shape.coll=tile.coll
				
				if not tile.dense then 
					shape:collision_type(space:type("pass")) -- a tile we can jump up through
				end
			end

		end
	end


	for y,line in pairs(map) do
		for x,tile in pairs(line) do		
			for n,f in pairs(entities.tiles) do
				if tile[n] then f(tile) end
			end
		end
	end
	
	system.components.back.dirty(true)
	system.components.map.dirty(true)
		
	entities.systems.player.add(0) -- add a player
end,
}

-----------------------------------------------------------------------------
--[[#entities.systems.menu

	menu = entities.systems.menu.setup()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.


]]
-----------------------------------------------------------------------------
entities.systems.menu={
space=function()
	local space=entities.get("space")
	local arbiter_menu={} -- menu things
		arbiter_menu.presolve=function(it)
			if it.shape_a.menu and it.shape_b.player then -- remember menu
				it.shape_b.player.near_menu=it.shape_a.menu
			end
			return false
		end
		arbiter_menu.separate=function(it)
			if it.shape_a and it.shape_a.menu and it.shape_b and it.shape_b.player then -- forget menu
				it.shape_b.player.near_menu=false
			end
			return true
		end
	space:add_handler(arbiter_menu,space:type("menu"))
end,
setup=function(items)


-- join all entity chat_text to build chat_texts with
	local chat_texts={}
	for n,v in pairs(entities.systems) do
		if v.chat_text then
			chat_texts[#chat_texts+1]=v.chat_text
		end
	end
	chat_texts=table.concat(chat_texts,"\n\n")
	
-- manage chat hooks from entities and print debug text
	local chat_hook=function(chat,change,...)
		local a,b=...

		if change=="subject"     then			print("subject ",chat.subject_name)

			for n,v in pairs(entities.systems) do
				if v.chat_hook_response then
				   v.chat_hook_response(chat,a)
				end
			end

		elseif change=="topic"   then			print("topic   ",chat.subject_name,a and a.name)

			for n,v in pairs(entities.systems) do
				if v.chat_hook_topic then
				   v.chat_hook_topic(chat,a)
				end
			end

		elseif change=="goto"   then			print("goto   ",chat.subject_name,a and a.name)

			for n,v in pairs(entities.systems) do
				if v.chat_hook_goto then
				   v.chat_hook_goto(chat,a)
				end
			end

		elseif change=="tag"     then			print("tag     ",chat.subject_name,a,b)

			for n,v in pairs(entities.systems) do
				if v.chat_hook_tag then
				   v.chat_hook_tag(chat,a,b)
				end
			end

		end
		
	end

	local chats=entities.set( "chats", chatdown.setup_chats(chat_texts,chat_hook) )

	local wstr=require("wetgenes.string")

	local menu=entities.set("menu",entities.add{})
--	local menu={}

	menu.stack={}

	menu.width=80-4
	menu.cursor=0
	menu.cx=math.floor((80-menu.width)/2)
	menu.cy=0
	
	menu.chat_to_menu_items=function(chat)
		local items={cursor=1,cursor_max=0}
		
		items.title=chat:get_tag("title")
		items.portrait=chat:get_tag("portrait")
		
		local ss=chat.topic and chat.topic.text or {} if type(ss)=="string" then ss={ss} end
		for i,v in ipairs(ss) do
			if i>1 then
				items[#items+1]={text="",chat=chat} -- blank line
			end
			items[#items+1]={text=chat:replace_tags(v)or"",chat=chat}
		end

		for i,v in ipairs(chat.gotos or {}) do

			items[#items+1]={text="",chat=chat} -- blank line before each goto

			local ss=v and v.text or {} if type(ss)=="string" then ss={ss} end

			local color=30
			if chat.viewed[v.name] then color=28 end -- we have already seen the response to this goto
			
			local f=function(item,menu)

				if item.topic and item.topic.name then

					chats.changes(chat,"topic",item.topic)

					chat:set_topic(item.topic.name)

					chat:set_tags(item.topic.tags)
					
					if item.topic.name=="exit" then
						menu.show(nil)
					else
						menu.show(menu.chat_to_menu_items(chat))
					end


				end
			end
			
			items[#items+1]={text=chat:replace_tags(ss[1])or"",chat=chat,topic=v,cursor=i,call=f,color=color} -- only show first line
			items.cursor_max=i
		end

		return items
	end

--[[
	menu.chat=function(chat)
		local items={cursor=1,cursor_max=0}
		
		items.title=chat.description and chat.description.text or chat.description_name
		
		local ss=chat.response and chat.response.text or {} if type(ss)=="string" then ss={ss} end
		for i,v in ipairs(ss) do
			if i>1 then
				items[#items+1]={text="",chat=chat} -- blank line
			end
			items[#items+1]={text=chat.replace_proxies(v)or"",chat=chat}
		end

		for i,v in ipairs(chat.decisions or {}) do

			items[#items+1]={text="",chat=chat} -- blank line before each decision

			local ss=v and v.text or {} if type(ss)=="string" then ss={ss} end

			local color=30
			if chat.viewed[v.name] then color=28 end -- we have already seen the response to this decision
			
			local f=function(item,menu)

				if item.decision and item.decision.name then

					chats.changes(chat,"decision",item.decision)

					chat.set_response(item.decision.name)

					chat.set_proxies(item.decision.proxies)

					menu.show( menu.chat(chat) )

				end
			end
			
			items[#items+1]={text=chat.replace_proxies(ss[1])or"",chat=chat,decision=v,cursor=i,call=f,color=color} -- only show first line
			items.cursor_max=i
		end

		return items
	end
]]

	function menu.show(items,subject_name,topic_name)
	
		if subject_name and topic_name then

			local chat=chats:get_subject(subject_name)
			chat:set_topic(topic_name)
			items=menu.chat_to_menu_items(chat)

		elseif subject_name then

			local chat=chats:get_subject(subject_name)
			items=menu.chat_to_menu_items(chat)

		end
	
		if not items then
			menu.items=nil
			menu.lines=nil
			return
		end

		if items.call then items.call(items,menu) end -- refresh
		
		menu.items=items
		menu.cursor=items.cursor or 1
		
		menu.lines={}
		for idx=1,#items do
			local item=items[idx]
			local text=item.text
			if text then
				local ls=wstr.smart_wrap(text,menu.width-8)
				if #ls==0 then ls={""} end -- blank line
				for i=1,#ls do
					local prefix=""--(i>1 and " " or "")
					if item.cursor then prefix=" " end -- indent decisions
					menu.lines[#menu.lines+1]={s=prefix..ls[i],idx=idx,item=item,cursor=item.cursor,color=item.color}
				end
			end
		end

	end


	
	menu.update=function()
	
		if not menu.items then return end

		local bfire,bup,bdown,bleft,bright
		
		for i=0,5 do -- any player, press a button, to control menu
			local up=ups(i)
			if up then
				bfire =bfire  or up.button("fire_clr")
				bup   =bup    or up.button("up_set")
				bdown =bdown  or up.button("down_set")
				bleft =bleft  or up.button("left_set")
				bright=bright or up.button("right_set")
			end
		end
		

		if bfire then

			for i,item in ipairs(menu.items) do
			
				if item.cursor==menu.cursor then
			
					if item.call then -- do this
					
						if item and item.decision and item.decision.name=="exit" then --exit menu
							item.call( item , menu )
							menu.show()	-- hide
						else
							item.call( item , menu )
						end
					end
					
					break
				end
			end
		end
		
		if bleft or bup then
		
			menu.cursor=menu.cursor-1
			if menu.cursor<1 then menu.cursor=menu.items.cursor_max end

		end
		
		if bright or bdown then
			
			menu.cursor=menu.cursor+1
			if menu.cursor>menu.items.cursor_max then menu.cursor=1 end
		
		end
	
	end
	
	menu.draw=function()

		local tprint=system.components.text.text_print
		local tgrd=system.components.text.tilemap_grd

		if not menu.lines then return end
		
		menu.cy=math.floor((30-(#menu.lines+4))/2)
		
		tgrd:clip(menu.cx,menu.cy,0,menu.width,#menu.lines+4,1):clear(0x02000000)
		tgrd:clip(menu.cx+2,menu.cy+1,0,menu.width-4,#menu.lines+4-2,1):clear(0x01000000)
		
		if menu.items.title then
			local title=" "..(menu.items.title).." "
			local wo2=math.floor(#title/2)
			tprint(title,menu.cx+(menu.width/2)-wo2,menu.cy+0,31,2)
		end
		
		for i,v in ipairs(menu.lines) do
			tprint(v.s,menu.cx+4,menu.cy+i+1,v.color or 31,1)
		end
		
		local it=nil
		for i=1,#menu.lines do
			if it~=menu.lines[i].item then -- first line only
				it=menu.lines[i].item
				if it.cursor == menu.cursor then
					tprint(">",menu.cx+4,menu.cy+i+1,31,1)
				end
			end
		end

		system.components.text.dirty(true)

	end
	

	if items then menu.show(items) end
	
	return menu
end,
}


-----------------------------------------------------------------------------
--[[#update

	update()

Update called every 1/60 of a second, possibly many times before we get 
a draw call.

]]
-----------------------------------------------------------------------------
update=function()

	if not setup_done then
		entities.systems.level.setup(1) -- load map
		setup_done=true
	end

	local menu=entities.get("menu")
	
	if menu.lines then -- menu only, pause the entities and draw the menu
		menu.update()
	else
		entities.call("update")
		local space=entities.get("space")
		space:step(1/(60*2)) -- double step for increased stability, allows faster velocities.
		space:step(1/(60*2))
	end

	local cb=entities.get("callbacks") or {} -- get callback list 
	entities.set("callbacks",{}) -- and reset it
	-- run all the callbacks created by collisions 
	for _,f in pairs(cb) do f() end

	
end

-----------------------------------------------------------------------------
--[[#draw

	draw()

Draw called every frame, there may be any number of updates between 
each draw but hopefully we are looking at one update followed by a 
draw, if you have an exceptionally fast computer then we may even get 0 
updates between some draws.

]]
-----------------------------------------------------------------------------
draw=function()

	local menu=entities.get("menu")

	if menu.lines then
		menu.draw()
	end

	entities.call("draw") -- draw everything, well, actually just prepare everything to be drawn by the system

end


-- load graphics into texture memory
for n,v in pairs(entities.systems) do
	if v.load then v:load() end
end
