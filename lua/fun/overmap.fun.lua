
local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wstr=require("wetgenes.string")
function ls(s) print(wstr.dump(s))end

hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() -- called at a steady 60fps
		if setup then setup() setup=nil end -- call an optional setup function *once*
		entities.systems.call("update")
		entities.call("update")
	end,
	draw=function() -- called at actual display frame rate
		entities.systems.call("draw")
		entities.call("draw")
	end,
})

hardware.screen.zxy={0.5,-0.5}

hardware.insert{
	component="overmap",
	name="map",					-- same name so will replace the foreground tilemap
	tiles="tiles",
	tilemap_size={128,128},
	tile_size={24,24,16},
	over_size={8,16},
	sort={-1,-1},
	mode="xz",
	layer=2,
}

entities=require("wetgenes.gamecake.fun.entities").create({
	sortby={
	},
})

-- setup all entities
setup=function() entities.systems.call("setup") end




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
	[   0]={ name="test_empty",				},
	[". "]={ name="test_empty",				},
	["1 "]={ name="test_blue",				},
	["2 "]={ name="test_orange",			},
}
	
levels={}

levels[1]={
legend=combine_legends(default_legend,{
}),
title="This is a test.",
map=[[
1 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
1 . 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 1 2 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 2 1 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 1 2 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 2 1 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 1 2 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 2 1 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 1 2 1 2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 2 1 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . 2 1 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
]],
}




entities.systems.insert{ caste="map",

	loads=function()

		hardware.graphics.loads{

{nil,"test_empty",[[
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
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},

{nil,"test_blue",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . B B B B B B B B b b b b b b b b I 
. . . . . . B B B B B B B B b b b b b b b b I I 
. . . . . B B B B B B B B b b b b b b b b I I I 
. . . . B B B B B B B B b b b b b b b b I I I I 
. . . b b b b b b b b B B B B B B B B b I I I I 
. . b b b b b b b b B B B B B B B B b b I I I I 
. b b b b b b b b B B B B B B B B b b b I I I I 
b b b b b b b b B B B B B B B B b b b b I I I I 
B B B B B B B B c c c c c c c c b b b b I I I . 
B B B B B B B B c c c c c c c c b b b b I I . . 
B B B B B B B B c c c c c c c c b b b b I . . . 
B B B B B B B B c c c c c c c c b b b b . . . . 
B B B B B B B B c c c c c c c c b b b . . . . . 
B B B B B B B B c c c c c c c c b b . . . . . . 
B B B B B B B B c c c c c c c c b . . . . . . . 
B B B B B B B B c c c c c c c c . . . . . . . . 
]]},

{nil,"test_orange",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . O O O O O O O O r r r r r r r r R 
. . . . . . O O O O O O O O r r r r r r r r R R 
. . . . . O O O O O O O O r r r r r r r r R R R 
. . . . O O O O O O O O r r r r r r r r R R R R 
. . . r r r r r r r r O O O O O O O O r R R R R 
. . r r r r r r r r O O O O O O O O r r R R R R 
. r r r r r r r r O O O O O O O O r r r R R R R 
r r r r r r r r O O O O O O O O r r r r R R R R 
O O O O O O O O o o o o o o o o r r r r R R R . 
O O O O O O O O o o o o o o o o r r r r R R . . 
O O O O O O O O o o o o o o o o r r r r R . . . 
O O O O O O O O o o o o o o o o r r r r . . . . 
O O O O O O O O o o o o o o o o r r r . . . . . 
O O O O O O O O o o o o o o o o r r . . . . . . 
O O O O O O O O o o o o o o o o r . . . . . . . 
O O O O O O O O o o o o o o o o . . . . . . . . 
]]},

		}
	end,
	
	setup=function()

		local legend={}
		for n,v in pairs( levels[1].legend ) do -- build tilemap from legend
			if v.name then -- convert name to tile
				legend[n]=system.components.tiles.names[v.name]
			end
		end
		bitdown.tile_grd( levels[1].map, legend, system.components.map.tilemap_grd, -- draw into the screen (tiles)
			nil,nil,nil,nil,function(tile,rx,ry)
			return	tile.pxt+(math.floor(rx/3)%(tile.hxt/3)) ,
					tile.pyt+(math.floor(ry/3)%(tile.hyt/3)) ,
					31 , -- color index tint
					255  -- lightness
		end)
		system.components.map.dirty(true)
		
		
		system.components.map.ax=160
		system.components.sprites.ax=160

	end,

	draw=function()
	
	system.components.text.text_tile =system.components.text.text_tile8x8
	system.components.text.text_print=system.components.text.text_print2
	
		local tx=wstr.trim([[

Use up/down/left/right to adjust the speed of the scrolling. 
Hit fire to reset the momentum.

]])
		local tl=wstr.smart_wrap(tx,system.components.text.text_hx/2-4)
		for i=1,#tl do
			local t=tl[i]
			system.components.text.text_print(t,2/2,1+i,31,0)
		end
		system.components.text.dirty(true)

	end,

}



entities.systems.insert{ caste="player",

	loads=function()

		hardware.graphics.loads{

{nil,"test_tile",[[
7 . . 7 7 . . 7 
. 7 . 7 7 . 7 . 
. . 7 7 7 7 . . 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
. . 7 7 7 7 . . 
. 7 . 7 7 . 7 . 
7 . . 7 7 . . 7 
]]},

		}

	end,

	setup=function()

		local names=system.components.tiles.names

	-- a player entity
		local test_entity=entities.add{	caste="player",
		
			tile=names.test_tile.idx,
			px=160,py=0,pz=0,sz=3,rz=0,
			update=function(it)
				local up=ups(0) -- get all connected controls, keyboard or gamepad

				if up.button("fire") then
					if up.button("up")    then it.sz=it.sz-(1/16) end
					if up.button("down")  then it.sz=it.sz+(1/16) end
					if up.button("left")  then it.rz=it.rz-1 end
					if up.button("right") then it.rz=it.rz+1 end
				else
					if up.button("up")    then it.pz=it.pz+1 end
					if up.button("down")  then it.pz=it.pz-1 end
					if up.button("left")  then it.px=it.px-1 end
					if up.button("right") then it.px=it.px+1 end
				end
				
			end,
			draw=function(it)
				system.components.sprites.list_add({t=it.tile,px=it.px,py=it.py,pz=it.pz,s=it.sz,rz=it.rz})
			end,
		}

	end,

}

-- finally load all graphics from systems defined above
entities.systems.call("loads")
