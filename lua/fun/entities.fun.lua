-----------------------------------------------------------------------------
--[[#hardware

select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.

This also provides a default main function that will upload the 
graphics and call the provided update/draw callbacks.

]]
-----------------------------------------------------------------------------
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() -- called at a steady 60fps
		if setup then setup() setup=nil end -- call an optional setup function *once*
		entities.call("update")
	end,
	draw=function() -- called at actual display frame rate
		entities.call("draw")
	end,
})

entities=require("wetgenes.gamecake.fun.entities").create({
	sortby={
		"before", -- 1
		"player", -- 2
		"later",  -- 3
		["laterlater"]=10,
		["inbetween"]=2.5,
	},
})

-- setup all entities
setup=function() entities.systems.call("setup") end

-----------------
-- ENTITY EXAMPLE
-----------------
entities.systems.insert{ caste="player",

	loads=function()

		hardware.graphics.loads{

{nil,"test_tile",[[
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
]]},

		}

	end,

	setup=function()

		local names=system.components.tiles.names

	-- a player entity
		local test_entity=entities.add{	caste="player",
		
			tile=names.test_tile.idx,
			px=160,py=120,sz=3,rz=0,
			update=function(it)
				local up=ups(0) -- get all connected controls, keyboard or gamepad

				if up.button("fire") then
					if up.button("up")    then it.sz=it.sz-(1/16) end
					if up.button("down")  then it.sz=it.sz+(1/16) end
					if up.button("left")  then it.rz=it.rz-1 end
					if up.button("right") then it.rz=it.rz+1 end
				else
					if up.button("up")    then it.py=it.py-1 end
					if up.button("down")  then it.py=it.py+1 end
					if up.button("left")  then it.px=it.px-1 end
					if up.button("right") then it.px=it.px+1 end
				end
				
			end,
			draw=function(it)
				system.components.sprites.list_add({t=it.tile,px=it.px,py=it.py,s=it.sz,rz=it.rz})
			end,
		}

	-- a system does not have to only create entities of its own caste
		entities.add({caste="later"})
		entities.add({caste="before"})
		entities.add({caste="laterlater"})
		entities.add({caste="inbetween"})
		entities.add({caste="noweight1"})
		entities.add({caste="noweight2"})
		entities.add({caste="noweight3"})

	-- dump order to show how sorting works
		for i=1,#entities.data do
			print(i,entities.data[i][1].caste)
		end

	end,

}


-- finally load all graphics from systems defined above
entities.systems.call("loads")
