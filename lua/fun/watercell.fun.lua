
local bit=require("bit")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local bitdown_font_4x8=require("wetgenes.gamecake.fun.bitdown_font_4x8")

local chipmunk=require("wetgenes.chipmunk")

local ls=function(...) print(wstr.dump(...)) end

local fatpix=not(args and args.pixel or false) -- pass --pixel on command line to turn off fat pixel filters

--request this hardware setup !The components will not exist until after main has been called!
cmap=bitdown.cmap -- use default swanky32 colors
screen={hx=424,hy=240,ss=3,fps=60}
oven.opts.fun="" -- back to menu on reset
hardware={
	{
		component="screen",
		size={screen.hx,screen.hy},
		bloom=fatpix and 0.75 or 0,
		filter=fatpix and "scanline" or nil,
		shadow=fatpix and "drop" or nil,
		scale=screen.ss,
		fps=screen.fps,
		layers=2,
	},
	{
		component="colors",
		cmap=cmap, -- swanky32 palette
	},
	{
		component="tiles",
		name="tiles",
		tile_size={8,8},
		bitmap_size={64,16},
	},
	{
		component="tilemap",
		name="map",
		tiles="tiles",
		tilemap_size={math.ceil(screen.hx/8),math.ceil(screen.hy/8)},
		layer=1,
	},
	{
		component="sprites",
		name="sprites",
		tiles="tiles",
		layer=1,
	},
	{
		component="autocell",
		name="cells",
		tiles="tiles",
		tilemap_size={math.ceil(screen.hx/8),math.ceil(screen.hy/8)},
		layer=2,
		shader_step_name="fun_step_watercell",
		shader_draw_name="fun_draw_watercell",
		rate=1,
	},
}


-- define all graphics in this global, we will convert and upload to tiles at setup
-- although you can change tiles during a game, we try and only upload graphics
-- during initial setup so we have a nice looking sprite sheet to be edited by artists

graphics={
{0x0100,"char_empty",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0101,"char_black",[[
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
]]},
{0x0102,"char_grey",[[
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
]]},
{0x0103,"char_white",[[
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
]]},
{0x0104,"char_dot",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0200,"char_water0",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0201,"char_water1",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . b b . . . 
. . . b b . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0202,"char_water2",[[
. . . . . . . . 
. . . . . . . . 
. . . b b . . . 
. . b b b b . . 
. . b b b b . . 
. . . b b . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0203,"char_water3",[[
. . . . . . . . 
. . b b b b . . 
. b b b b b b . 
. b b b b b b . 
. b b b b b b . 
. b b b b b b . 
. . b b b b . . 
. . . . . . . . 
]]},
{0x0204,"char_water4",[[
. . b b b b . . 
. b b b b b b . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
. b b b b b b . 
. . b b b b . . 
]]},
{0x0211,"char_waterb1",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
]]},
{0x0212,"char_waterb2",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0213,"char_waterb3",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0214,"char_waterb4",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0215,"char_waterb5",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0216,"char_waterb6",[[
. . . . . . . . 
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0217,"char_waterb7",[[
. . . . . . . . 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
{0x0218,"char_waterb8",[[
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
b b b b b b b b 
]]},
}


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
	[0]={ name="char_empty",	},

	[". "]={ name="char_empty",				cell={0,0,0,0}, },
	["0 "]={ name="char_black",				solid=1, dense=1, cell={0,0,0,1}, },		-- empty border
	["4 "]={ name="char_grey",				solid=1, dense=1, cell={0,0,0,1}, },		-- empty border
	["7 "]={ name="char_white",				solid=1, dense=1, cell={0,0,0,1}, },		-- empty border
	["W "]={ name="char_empty",				solid=1, dense=1, cell={0,0,0,2}, },		-- water generator

}
	
levels={}
levels[0]={
legend=combine_legends(default_legend,{
}),
title="This is a test",
map=[[
4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . W . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . . . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . . . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . . . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . . . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . 4 4 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . . . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . 4 . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . 4 . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . 4 . . . . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . 4 . . 4 . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 4 4 4 4 4 4 4 . 4 . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . 4 . . . . . . 4 . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . 4 4 4 4 . 4 4 4 . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . 4 . . . . 4 . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . 4 . 4 4 4 4 . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 . . . 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 4 4 4 4 . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4 
4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 
]],
}


-- handle tables of entities that need to be updated and drawn.

local entities -- a place to store everything that needs to be updated
local entities_info -- a place to store options or values
local entities_reset=function()
	entities={}
	entities_info={}
end
-- get items for the given caste
local entities_items=function(caste)
	caste=caste or "generic"
	if not entities[caste] then entities[caste]={} end -- create on use
	return entities[caste]
end
-- add an item to this caste
local entities_add=function(it,caste)
	caste=caste or it.caste -- probably from item
	caste=caste or "generic"
	local items=entities_items(caste)
	items[ #items+1 ]=it -- add to end of array
	return it
end
-- call this functions on all items in every caste
local entities_call=function(fname,...)
	local count=0
	for caste,items in pairs(entities) do
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
local entities_get=function(name)       return entities_info[name]							end
local entities_set=function(name,value)        entities_info[name]=value	return value	end
local entities_manifest=function(name)
	if not entities_info[name] then entities_info[name]={} end -- create empty
	return entities_info[name]
end
-- reset the entities
entities_reset()


-- call coroutine with traceback on error
local coroutine_resume_and_report_errors=function(co,...)
	local a,b=coroutine.resume(co,...)
	if a then return a,b end -- no error
	error( b.."\nin coroutine\n"..debug.traceback(co) , 2 ) -- error
end


-- create space and handlers
function setup_space()

	local space=entities_set("space", chipmunk.space() )
	
	space:gravity(0,700)
	space:damping(0.5)
	space:sleep_time_threshold(1)
	space:idle_speed_threshold(10)

	return space
end


function setup_level(idx)

	local level=entities_set("level",entities_add{})

	local names=system.components.tiles.names

	level.updates={} -- tiles to update (animate)
	level.update=function()
		for v,b in pairs(level.updates) do -- update these things
			if v.update then v:update() end
		end
	end

-- init map and space

	local space=setup_space()

	for n,v in pairs( levels[idx].legend ) do -- fixup missing values (this will slightly change your legend data)
		if v.name then -- convert name to tile idx
			v.idx=names[v.name].idx
		end
		if v.idx then -- convert idx to r,g,b,a
			v[1]=(          (v.idx    )%256)
			v[2]=(math.floor(v.idx/256)%256)
			v[3]=31
			v[4]=0
		end
	end

	local map=entities_set("map", bitdown.pix_tiles(  levels[idx].map,  levels[idx].legend ) )
	
	level.title=levels[idx].title
	
	bitdown.pix_grd(    levels[idx].map,  levels[idx].legend,      system.components.map.tilemap_grd  ) -- draw into the screen (tiles)

	bitdown.pix_grd(    levels[idx].map,  levels[idx].legend,      system.components.cells.autocell_grd  ,nil,nil,nil,nil,"cell" ) -- draw into cells, using cell member
	system.components.cells.dirty(true)

	bitdown.map_build_collision_strips(map,function(tile)
		if tile.coll then -- can break the collision types up some more by appending a code to this setting
			if tile.collapse then -- make unique
				tile.coll=tile.coll..tile.idx
			end
		end
	end)

	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			local shape
			if tile.solid and (not tile.parent) then -- if we have no parent then we are the master tile
			
				local l=1
				local t=tile
				while t.child do t=t.child l=l+1 end -- count length of strip

				if     tile.link==1 then -- x strip
					shape=space.static:shape("box",x*8,y*8,(x+l)*8,(y+1)*8,0)
				elseif tile.link==-1 then  -- y strip
					shape=space.static:shape("box",x*8,y*8,(x+1)*8,(y+l)*8,0)
				else -- single box
					shape=space.static:shape("box",x*8,y*8,(x+1)*8,(y+1)*8,0)
				end

				shape:friction(tile.solid)
				shape:elasticity(tile.solid)
				shape.cx=x
				shape.cy=y
				shape.coll=tile.coll
			end

			tile.map=map -- remember map
			tile.level=level -- remember level
			if shape then -- link shape and tile
				shape.tile=tile
				tile.shape=shape
			end
		end
	end


	for y,line in pairs(map) do
		for x,tile in pairs(line) do
		end
	end
	
end

-- create a fat controller coroutine that handles state changes, fills in entities etc etc etc

local fat_controller=coroutine.create(function()

-- copy font data tiles into top line
	system.components.tiles.bitmap_grd:pixels(0,0,128*4,8, bitdown_font_4x8.grd_mask:pixels(0,0,128*4,8,"") )

-- upload graphics
	system.components.tiles.upload_tiles( graphics )

-- setup background


-- setup game
	entities_reset()

	setup_level(0) -- load map


-- update loop

	while true do coroutine.yield()
	
		entities_call("update")
		entities_get("space"):step(1/screen.fps)

		-- run all the callbacks created by collisions 
		for _,f in pairs(entities_manifest("callbacks")) do f() end
		entities_set("callbacks",{}) -- and reset the list

	end


end)


-- this is the main function, code below called repeatedly to update and draw or handle other messages (eg mouse)

function main(need)

	if not need.setup then need=coroutine.yield() end -- wait for setup request (should always be first call)

	coroutine_resume_and_report_errors( fat_controller ) -- setup

-- after setup we should yield and then perform updates only if requested from a yield
	local done=false while not done do
		need=coroutine.yield()
		if need.update then
			coroutine_resume_and_report_errors( fat_controller ) -- update
		end
		if need.draw then
--			system.components.text.dirty(true)
--			system.components.text.text_window()
--			system.components.text.text_clear(0x00000000)
--			system.components.sprites.list_reset() -- remove old sprites here
			entities_call("draw") -- because we are going to add them all in again here
		end
		if need.clean then done=true end -- cleanup requested
	end

-- perform cleanup here

end

--[=[



#shader "fun_step_watercell"

uniform mat4 modelview;
uniform mat4 projection;
uniform sampler2D tex_cell;
uniform vec4  map_info; /* 2,3 the map texture size*/

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
	gl_Position = projection * vec4(a_vertex , 1.0);
	v_texcoord=a_texcoord;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;


// where should the center flow to, return a single direction only
vec3 get_flow_vert(vec4 ctr,vec4 top,vec4 bot,vec4 lft,vec4 rgt)
{
	if( (ctr.a==2.0) && (bot.a==0.0) ) // water generator
	{
		if(bot.b<128.0) { return  vec3(0.0,1.0,1.0); }
	}
	else
	if( (ctr.a==0.0) && (ctr.b>=1.0) )
	{
		if( (bot.a==0.0) )
		{
			if( (ctr.b>=bot.b) || (bot.b<=8.0) ) { return vec3(0.0,1.0,1.0); } // fall
		}

		if( (top.a==0.0) && (ctr.b>8.0) )
		{
			if(ctr.b>top.b) { return vec3(0.0,-1.0,1.0); } // push up
		}

		if( (lft.a==0.0) && (rgt.a==0.0) ) // try left or right?
		{
			if( (ctr.x<=0.0) )
			{
				if(ctr.b>lft.b) { return vec3(-1.0,0.0,1.0); }
			}
			if( (ctr.x>=0.0) )
			{
				if(ctr.b>rgt.b) { return vec3( 1.0,0.0,1.0); }
			}
			if(ctr.b>lft.b) { return vec3(-1.0,0.0,1.0); }
			if(ctr.b>rgt.b) { return vec3( 1.0,0.0,1.0); }
		}
		else
		if( (lft.a==0.0) ) // try left
		{
			if(ctr.b>lft.b) { return vec3(-1.0,0.0,1.0); }
		}
		else
		if( (rgt.a==0.0) ) // try right
		{
			if(ctr.b>rgt.b) { return vec3( 1.0,0.0,1.0); }
		}


	}

	return vec3(0.0,0.0,0.0);
}

void main(void)
{
	vec4 cc[13];
	vec2 vx=vec2(1.0, 0.0)/map_info.zw;
	vec2 vy=vec2(0.0, 1.0)/map_info.zw;
	float w,w1,w2;
	vec3 fctr,ftop,fbot,flft,frgt;


	cc[0]=texture2D(tex_cell, v_texcoord-vy-vx ).rgba*255.0; // toplft
	cc[1]=texture2D(tex_cell, v_texcoord-vy    ).rgba*255.0; // top
	cc[2]=texture2D(tex_cell, v_texcoord-vy+vx ).rgba*255.0; // toprgt

	cc[3]=texture2D(tex_cell, v_texcoord-vx ).rgba*255.0; // lft
	cc[4]=texture2D(tex_cell, v_texcoord    ).rgba*255.0; // ctr
	cc[5]=texture2D(tex_cell, v_texcoord+vx ).rgba*255.0; // rgt

	cc[6]=texture2D(tex_cell, v_texcoord+vy-vx ).rgba*255.0; // botlft
	cc[7]=texture2D(tex_cell, v_texcoord+vy    ).rgba*255.0; // bot
	cc[8]=texture2D(tex_cell, v_texcoord+vy+vx ).rgba*255.0; // botrgt

	cc[9]=texture2D(tex_cell, v_texcoord-vx-vx ).rgba*255.0; // lftlft
	cc[10]=texture2D(tex_cell, v_texcoord+vx+vx ).rgba*255.0; // rgtrgt
	cc[11]=texture2D(tex_cell, v_texcoord-vy-vy ).rgba*255.0; // toptop
	cc[12]=texture2D(tex_cell, v_texcoord+vy+vy ).rgba*255.0; // botbot
		
	if(cc[4].a == 0.0) // empty
	{
		w=0.0;
		
		fctr=get_flow_vert(cc[4],cc[1],cc[7],cc[3],cc[5]);
		
		ftop=get_flow_vert(cc[1],cc[11],cc[4],cc[0],cc[2]);
		fbot=get_flow_vert(cc[7],cc[4],cc[12],cc[6],cc[8]);
		flft=get_flow_vert(cc[3],cc[0],cc[6],cc[9],cc[4]);
		frgt=get_flow_vert(cc[5],cc[2],cc[8],cc[4],cc[10]);


		w-=fctr.z;

		if(abs(fctr.x)>0.0) { cc[4].x=fctr.x; }

		if(ftop.y>0.0) { w+=ftop.y; if(fctr.z==0.0){cc[4].x=cc[1].x; } }
		if(fbot.y<0.0) { w-=fbot.y; if(fctr.z==0.0){cc[4].x=cc[7].x; } }
		if(flft.x>0.0) { w+=flft.x; if(fctr.z==0.0){cc[4].x=flft.x;} }
		if(frgt.x<0.0) { w-=frgt.x; if(fctr.z==0.0){cc[4].x=frgt.x;} }

		cc[4].b+=w;
	}
	
	gl_FragColor=cc[4]/255.0;
}

#endif

#shader "fun_draw_watercell"

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;
uniform sampler2D tex_cmap;
uniform sampler2D tex_tile;
uniform sampler2D tex_map;
uniform vec4  tile_info; /* 0,1 tile size eg 8x8 and 2,3 the font texture size*/
uniform vec4  map_info; /* 0,1 just add this to texcoord and 2,3 the map texture size*/


#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(a_vertex.xy, 0.0 , 1.0);
    gl_Position.z+=a_vertex.z;
	v_texcoord=a_texcoord;
	v_color=color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;
varying vec4  v_color;


void main(void)
{
	vec2 vy=vec2(0.0, 1.0)/map_info.zw;

	vec4 bg,fg; // colors
	vec4 c;
	vec4 cc,cb;
	vec4 d;
	vec2 uv=v_texcoord.xy+map_info.xy;		// base uv
	vec2 tc=fract(uv);						// tile uv
	vec2 tm=(floor(mod(uv,map_info.zw))+vec2(0.5,0.5))/map_info.zw;			// map uv
	
	cc=texture2D(tex_map, tm).rgba*255.0;
	cb=texture2D(tex_map, tm+vy).rgba*255.0;
	
	d=vec4( 0.0 /255.0, 2.0 /255.0, 31.0 /255.0, 0.0 /255.0); // default to nothing

	if(cc.a == 0.0) // empty
	{

		if( (cb.a != 0.0) || (cb.b>=8.0) )// solid
		{
			d.r=(16.0+min(cc.b,8.0))/255.0;
			if(cc.b>=8.0) // some water
			{
				d.b=( 31.0-min((cc.b-8.0)/2.0,6.0) )/255.0;
			}
		}
		else
		{
			if(cc.b>=4.0) // some water
			{
				d.r=4.0/255.0;
			}
			else
			if(cc.b>=3.0) // some water
			{
				d.r=3.0/255.0;
			}
			else
			if(cc.b>=2.0) // some water
			{
				d.r=2.0/255.0;
			}
			else
			if(cc.b>=1.0) // some water
			{
				d.r=1.0/255.0;
			}
		}
	}
	else
	if(cc.a == 1.0) // solid
	{
	}
	else
	if(cc.a == 2.0) // water generator
	{
			d.r=4.0/255.0;
	}

//	d.r=4.0/255.0;
//	d.g=1.0/255.0;
//	d.b=31.0/255.0;
//	d.a=0.0/255.0;
	
	c=texture2D(tex_tile, (((d.rg*vec2(255.0,255.0))+tc)*tile_info.xy)/tile_info.zw ).rgba;
	fg=texture2D(tex_cmap, vec2( d.b,0.5) ).rgba;
	bg=texture2D(tex_cmap, vec2( d.a,0.5) ).rgba;

	c*=fg; // forground tint, can adjust its alpha
	c=((bg*(1.0-c.a))+c)* v_color; // background color mixed with pre-multiplied foreground and then finally tint all of it by the main color
 
	gl_FragColor=c;

}

#endif


//]=]
