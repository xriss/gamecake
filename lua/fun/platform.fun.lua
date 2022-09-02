
local bit=require("bit")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local bitdown_font_4x8=require("wetgenes.gamecake.fun.bitdown_font_4x8")

local chipmunk=require("wetgenes.chipmunk")

local ls=function(...) print(wstr.dump(...)) end

local fatpix=not(args and args.pixel or false) -- pass --pixel on command line to turn off fat pixel filters

local start_level

--request this hardware setup !The components will not exist until after main has been called!
cmap=bitdown.cmap -- use default swanky32 colors
screen={hx=424,hy=240,ss=3,fps=60}
hardware={
	{
		component="screen",
		size={screen.hx,screen.hy},
		bloom=fatpix and 0.75 or 0,
		filter=fatpix and "scanline" or nil,
		shadow=fatpix and "drop" or nil,
		scale=screen.ss,
		fps=screen.fps,
		layers=3,
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
		component="copper",
		name="copper",
		size={screen.hx,screen.hy},
		layer=1,
	},
	{
		component="tilemap",
		name="map",
		tiles="tiles",
		tilemap_size={math.ceil(screen.hx/8),math.ceil(screen.hy/8)},
		layer=2,
	},
	{
		component="sprites",
		name="sprites",
		tiles="tiles",
		layer=2,
	},
	{
		component="tilemap",
		name="text",
		tiles="tiles",
		tile_size={4,8}, -- use half width tiles for font
		tilemap_size={math.ceil(screen.hx/4),math.ceil(screen.hy/8)},
		layer=3,
	},
}


-- define all graphics in this global, we will convert and upload to tiles at setup
-- although you can change tiles during a game, we try and only upload graphics
-- during initial setup so we have a nice looking sprite sheet to be edited by artists

graphics={
{0x0000,"_font",0x0140}, -- allocate the font area
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
{0x0102,"char_wall",[[
O O R R R R O O 
O O R R R R O O 
r r r r o o o o 
r r r r o o o o 
R R O O O O R R 
R R O O O O R R 
o o o o r r r r 
o o o o r r r r 
]]},
{0x0103,"char_floor",[[
R R R R R R R R 
r r r r r r r r 
r r r r r r r r 
. r r r r r r . 
. . r . r r . . 
. . . . r r . . 
. . . . . r . . 
. . . . . . . . 
]]},
{0x0106,"char_floor_collapse",[[
R r R r R r R r 
r r . r . r . r 
r . r . r . r . 
. r . r . r r . 
. . r . r . . . 
. . . . r . r . 
. . . . . r . . 
. . . . . . . . 
]]},
{0x0107,"char_floor_collapse_1",[[
. r . r . r . r 
. r . r . r . r 
r . . . . . r . 
. . . r . r . . 
. . r . r . . . 
. . . . r . r . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0108,"char_floor_collapse_2",[[
. r . r . r . r 
. . . r . . . . 
r . . . . . . . 
. . . . . r . . 
. . r . . . . . 
. . . . r . r . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x0109,"char_floor_collapse_3",[[
. r . r . r . r 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},
{0x010a,"char_floor_move_1",[[
R R R R R R R R 
r r r r r r r r 
. . R . . . R . 
R R R R R R R R 
R . . . R . . . 
r r r r r r r r 
R R R R R R R R 
. . . . . . . . 
]]},
{0x010b,"char_floor_move_2",[[
R R R R R R R R 
r r r r r r r r 
. R . . . R . . 
R R R R R R R R 
. . . R . . . R 
r r r r r r r r 
R R R R R R R R 
. . . . . . . . 
]]},
{0x010c,"char_floor_move_3",[[
R R R R R R R R 
r r r r r r r r 
R . . . R . . . 
R R R R R R R R 
. . R . . . R . 
r r r r r r r r 
R R R R R R R R 
. . . . . . . . 
]]},
{0x010d,"char_floor_move_4",[[
R R R R R R R R 
r r r r r r r r 
. . . R . . . R 
R R R R R R R R 
. R . . . R . . 
r r r r r r r r 
R R R R R R R R 
. . . . . . . . 
]]},
{0x010e,"char_stripe",[[
R R R R O O O O 
R R R O O O O R 
R R O O O O R R 
R O O O O R R R 
O O O O R R R R 
O O O R R R R O 
O O R R R R O O 
O R R R R O O O 
]]},

{0x0110,"char_spike_down",[[
R R 7 7 7 7 R R 
R R 7 7 7 7 R R 
. R R 7 7 R R . 
. R R 7 7 R R . 
. . R R R R . . 
. . R R R R . . 
. . . R R . . . 
. . . R R . . . 
]]},
{0x0111,"char_spike_up",[[
. . . R R . . . 
. . . R R . . . 
. . R R R R . . 
. . R R R R . . 
. R R 7 7 R R . 
. R R 7 7 R R . 
R R 7 7 7 7 R R 
R R 7 7 7 7 R R 
]]},


{0x0120,"char_dust",[[
r r r r R R R R 
r r r r R R R R 
r r r r R R R R 
r r r r R R R R 
R R R R r r r r 
R R R R r r r r 
R R R R r r r r 
R R R R r r r r 
]]},

{0x0121,"char_dust_white",[[
6 6 6 6 7 7 7 7 
6 6 6 6 7 7 7 7 
6 6 6 6 7 7 7 7 
6 6 6 6 7 7 7 7 
7 7 7 7 6 6 6 6 
7 7 7 7 6 6 6 6 
7 7 7 7 6 6 6 6 
7 7 7 7 6 6 6 6 
]]},

{0x0200,"player_f1",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . 4 4 4 4 . . . . . . . . . . 
. . . . . . . . . 4 2 7 7 1 4 . . . . . . . . . 
. . . . . . . . . 4 7 2 1 7 4 . . . . . . . . . 
. . . . . . . . 4 7 7 1 2 7 7 4 . . . . . . . . 
. . . . . . . 4 7 7 1 7 7 2 7 7 4 . . . . . . . 
. . . . . . . 4 4 4 4 4 4 4 4 4 4 . . . . . . . 
. . . . . . . . . . 7 7 0 7 . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . . . 7 7 . . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . 7 7 7 7 . 7 7 7 . . . . . . . . 
. . . . . . . . 7 7 7 7 7 . 7 7 . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . 7 7 7 . 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 . 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 7 . 7 7 7 . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},
{0x0203,"player_f2",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . 4 4 4 4 . . . . . . . . . . 
. . . . . . . . . 4 2 7 7 1 4 . . . . . . . . . 
. . . . . . . . . 4 7 2 1 7 4 . . . . . . . . . 
. . . . . . . . 4 7 7 1 2 7 7 4 . . . . . . . . 
. . . . . . . 4 7 7 1 7 7 2 7 7 4 . . . . . . . 
. . . . . . . 4 4 4 4 4 4 4 4 4 4 . . . . . . . 
. . . . . . . . . . 7 7 0 7 . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . . . 7 7 . . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . 7 7 . 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 . 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 . 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 7 . 7 7 . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . . . 7 7 . . . . . . . . . . . 
. . . . . . . . . . . 7 7 . . . . . . . . . . . 
. . . . . . . . . . . 7 7 7 . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},
{0x0206,"player_f3",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . 4 4 4 4 . . . . . . . . . . 
. . . . . . . . . 4 2 7 7 1 4 . . . . . . . . . 
. . . . . . . . . 4 7 2 1 7 4 . . . . . . . . . 
. . . . . . . . 4 7 7 1 2 7 7 4 . . . . . . . . 
. . . . . . . 4 7 7 1 7 7 2 7 7 4 . . . . . . . 
. . . . . . . 4 4 4 4 4 4 4 4 4 4 . . . . . . . 
. . . . . . . . . . 7 7 0 7 . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . . . 7 7 . . . . . . . . . . . 
. . . . . . . . . . 7 7 7 7 . . . . . . . . . . 
. . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . 7 7 7 7 7 7 7 7 . . . . . . . . 
. . . . . . . 7 7 7 7 7 7 7 7 7 7 . . . . . . . 
. . . . . . . 7 7 . 7 7 7 7 . 7 7 . . . . . . . 
. . . . . . . . . . 7 7 7 7 7 . . . . . . . . . 
. . . . . . . . . 7 7 7 . 7 7 . 7 . . . . . . . 
. . . . . . . . 7 7 . . . . 7 7 7 . . . . . . . 
. . . . . . . . 7 7 7 . . . 7 7 . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]]},
{0x0209,"cannon_ball",[[
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
{0x020C,"bubble",[[
. . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 
. . . . . . . 7 7 . . . . . . 7 7 . . . . . . . 
. . . . . 7 7 . . . . . . . . . . 7 7 . . . . . 
. . . . 7 . . . . . . . . . . . . . . 7 . . . . 
. . . 7 . . . 7 7 . . . . . . . . . . . 7 . . . 
. . 7 . . . 7 . . . . . . . . . . . . . . 7 . . 
. . 7 . . 7 . . . . . . . . . . . . . . . 7 . . 
. 7 . . . . . . . . . . . . . . . . . . . . 7 . 
. 7 . . . . . . . . . . . . . . . . . . . . 7 . 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
7 . . . . . . . . . . . . . . . . . . . . . . 7 
. 7 . . . . . . . . . . . . . . . . . . . . 7 . 
. 7 . . . . . . . . . . . . . . . . . . . . 7 . 
. . 7 . . . . . . . . . . . . . . . 7 . . 7 . . 
. . 7 . . . . . . . . . . . . . . 7 . . . 7 . . 
. . . 7 . . . . . . . . . . . 7 7 . . . 7 . . . 
. . . . 7 . . . . . . . . . . . . . . 7 . . . . 
. . . . . 7 7 . . . . . . . . . . 7 7 . . . . . 
. . . . . . . 7 7 . . . . . . 7 7 . . . . . . . 
. . . . . . . . . 7 7 7 7 7 7 . . . . . . . . . 
]]},

{0x0500,"coin",[[
. . . . . . . . 
. . Y Y Y Y . . 
. Y Y 0 0 Y Y . 
Y Y 0 Y Y 0 Y Y 
Y Y Y 0 0 Y Y Y 
Y Y 0 Y Y 0 Y Y 
. Y Y 0 0 Y Y . 
. . Y Y Y Y . . 
]]},

{0x0600,"body_p1",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . 4 4 4 4 . . . . . . 
. . . . . 4 2 7 7 1 4 . . . . . 
. . . . . 4 7 2 1 7 4 . . . . . 
. . . . 4 7 7 1 2 7 7 4 . . . . 
. . . 4 7 7 1 7 7 2 7 7 4 . . . 
. . . 4 4 4 4 4 4 4 4 4 4 . . . 
. . . . . . 7 7 0 7 . . . . . . 
. . . . . . 7 7 7 7 7 . . . . . 
. . . . . . 7 7 7 7 . . . . . . 
. . . . . . . 7 7 . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{0x0602,"body_p2",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . 7 7 . . . . . . . 
. . . . . . 7 7 7 7 . . . . . . 
. . . . . 7 7 7 7 7 7 . . . . . 
. . . . 7 7 7 7 7 7 7 7 . . . . 
. . . 7 7 7 7 7 7 7 7 7 7 . . . 
. . . 7 7 . 7 7 7 7 . 7 7 . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{0x0604,"body_p3",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . 7 7 7 7 . . . . . . 
. . . . . . 7 7 7 7 7 . . . . . 
. . . . . 7 7 7 . 7 7 . 7 . . . 
. . . . 7 7 . . . . 7 7 7 . . . 
. . . . 7 7 7 . . . 7 7 . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{0x0800,"door_open",[[
. . . . . . . . . 0 0 0 0 0 0 . . . . . . . . . 
. . . . . . . 0 0 0 0 0 0 0 0 0 0 . . . . . . . 
. . . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . 
. . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
. . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . 
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

	[". "]={ name="char_empty",				},
	["00"]={ name="char_black",				solid=1, dense=1, },		-- black border
	["0 "]={ name="char_empty",				solid=1, dense=1, },		-- empty border

	["||"]={ name="char_wall",				solid=1},				-- wall
	["//"]={ name="char_stripe",			solid=1},				-- wall
	["=="]={ name="char_floor",				solid=1},				-- floor
	["<<"]={ name="char_floor_move_1",		solid=1,push=-1},		-- floor push left
	[">>"]={ name="char_floor_move_3",		solid=1,push= 1},		-- floor push right
	["--"]={ name="char_floor_collapse",	solid=1,collapse=1},	-- floor collapse

	["X "]={ name="char_spike_down",		deadly= 1},				-- ceiling spike
	["x "]={ name="char_spike_up",			deadly=-1},				-- floor spike


-- items not tiles, so display tile 0 and we will add a sprite for display
	["$ "]={ name="char_empty",	loot=1,		},
	["? "]={ name="char_empty",	item=1,		},
	["S "]={ name="char_empty",	start=1,	},
	["M "]={ name="char_empty",	monster=1,	},
	["< "]={ name="char_empty",	trigger=-1,	},
	["> "]={ name="char_empty",	trigger= 1,	},

	["E "]={ name="char_empty",	exit=1, sign="EXIT", colors={cmap.red,cmap.orange,cmap.yellow,cmap.green,cmap.blue} },
	["?2"]={ name="char_empty",	spill=1,	},
	["?3"]={ name="char_empty",	bubble={start=0 ,rate=120},	},
	["?4"]={ name="char_empty",	bubble={start=60,rate=120},	},
}
	
levels={}
levels[0]={
legend=combine_legends(default_legend,{
	["D0"]={ name="char_empty",	sign_y=-24, sign="Level 1", colors={cmap.red,cmap.orange,cmap.yellow,cmap.green,cmap.blue},
		menu={
			title="level 1",
			items={
				{text="Back"},
				{text="Jump to Level 1",call=function(it) start_level(1) end},
			}
		},
		sprite="door_open",
	},
	["D1"]={ name="char_empty",	sign_y=-24, sign="Level 2", colors={cmap.red,cmap.orange,cmap.yellow,cmap.green,cmap.blue},
		menu={
			title="level 2",
			items={
				{text="Back"},
				{text="Jump to Level 2"},
			}
		},
		sprite="door_open",
	},
	["D2"]={ name="char_empty",	sign_y=-24, sign="Level 3", colors={cmap.red,cmap.orange,cmap.yellow,cmap.green,cmap.blue},
		menu={
			title="level 3",
			items={
				{text="Back"},
				{text="Jump to Level 3"},
			}
		},
		sprite="door_open",
	},
	["D3"]={ name="char_empty",	sign_y=-24, sign="Level 4", colors={cmap.red,cmap.orange,cmap.yellow,cmap.green,cmap.blue},
		menu={
			title="level 4",
			items={
				{text="Back"},
				{text="Jump to Level 4"},
			}
		},
		sprite="door_open",
	},
}),
title="Let's make a journey to the cave of monsters!",
map=[[
//000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ $ . . . . . . . . . . . . . . . . . . . //
//. . . . $ . . . . . . . . . . . . . $ $ . . . . . . . . . . $ $ . . . . . . . . . . . . . . $ . . . . //
//. . . . . . . . . . . . . . . . . . $ $ . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . D3. . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//////////////////. . . . . //////////////////////////////////////////////////. . . . . //////////////////
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. $ . . . . . . . . . . . . . . . . . . . . . . D2. . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//////////////////. . . . . //////////////////////////////////////////////////. . . . . //////////////////
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ . . //
//. . . . . . $ . . . . . . . . . . . . . . . . . D1. . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//////////////////. . . . . //////////////////////////////////////////////////. . . . . //////////////////
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ //
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . S . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . . . . . . . . . . . . . . . D0. . . . . . . . . . . . . . . . . . . . . . . . . . //
//. . . . . . . . . . ?3. . . . . . . . . . . . . . . . . . . . . . . . . . . . . ?4. . . . . . . . . . //
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 //
]],
}

levels[1]={
legend=combine_legends(default_legend,{
	["?0"]={ name="char_empty" },
}),
title="A small cave full of treasures.",
map=[[
||000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000||
||. . . . X . $ . X . $ . X . . . . . . . X . . X . . . . . . . . . . . . . . . . . . . . . . . . . $ . ||
||. . . . . . . . . . . . . . . . . . . . $ . . $ . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . ? . . . . . . . . . . . . . . . x $ . . x $ . . x $ . . . . . ||
||================================================--------====--------==================================||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ . . . . . . . . . . . . . . . . . . ||
||======. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ ||
||==========. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . ||||||||||||. . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . > . . . . . M . . . . < ||||||||||||. . $ x $ . . . . ? . . . . . . . . . . . . ||
||==============. . . . <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<. . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ==========||
||. . . . . . . . . . . . . . . . . $ . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ======||
||. . . S . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||||||||||||--------====--------==========||
||. . . . . . . . . . . . . . . . . x $ . . . . . ? . . . . . ||||||||||||. . . . . . . . . . . . . $ . ||
||. . . . . . . ==========================================================. . . . . . . . . . . . $ . $ ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . $ . ||
||. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . ||
||======================================================================================================||
||0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ||
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
	space:add_handler(arbiter_pass,0x1001)
	
	local arbiter_deadly={} -- deadly things
		arbiter_deadly.presolve=function(it)
			local callbacks=entities_manifest("callbacks")
			if it.shape_b.player then -- trigger die
				local pb=it.shape_b.player
				callbacks[#callbacks+1]=function() pb:die() end
			end
			return true
		end
	space:add_handler(arbiter_deadly,0x1002)

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
	space:add_handler(arbiter_crumbling,0x1003)

	local arbiter_walking={} -- walking things (players)
		arbiter_walking.presolve=function(it)
			local callbacks=entities_manifest("callbacks")
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
				local time=entities_get("time")
				it.shape_a.in_body.floor_time=time.game
				it.shape_a.in_body.floor=it.shape_b
			end
			return true
		end
	space:add_handler(arbiter_walking,0x2001) -- walking things (players)

	local arbiter_loot={} -- loot things (pickups)
		arbiter_loot.presolve=function(it)
			if it.shape_a.loot and it.shape_b.player then -- trigger collect
				it.shape_a.loot.player=it.shape_b.player
			end
			return false
		end
	space:add_handler(arbiter_loot,0x3001) 
	
	local arbiter_trigger={} -- trigger things
		arbiter_trigger.presolve=function(it)
			if it.shape_a.trigger and it.shape_b.triggered then -- trigger something
				it.shape_b.triggered.triggered = it.shape_a.trigger
			end
			return false
		end
	space:add_handler(arbiter_trigger,0x4001)

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
	space:add_handler(arbiter_menu,0x4002)

	return space
end


-- items, can be used for general things, EG physics shapes with no special actions
function add_item()
	local item=entities_add{caste="item"}
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
end

-- a floating item that you can/must collect
function add_loot()
	local space=entities_get("space")
	local loot=entities_add{caste="loot"}
	loot.update=function()
		if loot.active then				
			if loot.player then
				loot.player.score=loot.player.score+1
				loot.active=false
				space:remove(loot.shape)
			end
		end
	end
	loot.draw=function()
		if loot.active then
			local time=entities_get("time")
			local b=math.sin( (time.game*8 + (loot.px+loot.py)/16 ) )*2
			system.components.sprites.list_add({t=0x0500,h=8,px=loot.px,py=loot.py+b})				
		end
	end
	return loot
end

-- an item that just gets in the way
function add_detritus(sprite,h,px,py,bm,bi,bf,be,...)
	local space=entities_get("space")
	local item=add_item()

	item.sprite=sprite
	item.h=h

	item.active=true
	item.body=space:body(bm,bi)
	item.body:position(px,py)

	item.shape=item.body:shape(...)
	item.shape:friction(bf)
	item.shape:elasticity(be)
	
	return item
end


function setup_menu()

	local menu=entities_set("menu",entities_add{})

	menu.stack={}

	menu.width=64
	menu.cursor=0
	menu.cx=math.floor((106-menu.width)/2)
	menu.cy=0
	
	-- go back to the previous menu
	function menu.update_items(it)

		if it.call then top.call(it) end -- refresh
		
		menu.items=it.items
		menu.cursor=it.cursor or 1
		
		menu.lines={}
		for id=1,#it.items do
			local ls=wstr.smart_wrap(it.items[id].text,menu.width-8)
			for i=1,#ls do menu.lines[#menu.lines+1]={s=(i>1 and " " or "")..ls[i],id=id,item=it.items[id]} end
		end

	end

	-- set a menu to display
	function menu.show(it)
	
		if #menu.stack>0 then
			menu.stack[#menu.stack].cursor=menu.cursor -- remember cursor position
		end

		menu.stack[#menu.stack+1]=it --push

		menu.update_items(it)

	end
	
	-- go back to the previous menu
	function menu.back()

		menu.stack[#menu.stack]=nil -- this was us
		
		if #menu.stack==0 then return menu.hide() end -- clear all menus
		
		local it=menu.stack[#menu.stack] -- pop up a menu
		
		menu.update_items(it)
				
	end
	
	-- stop showing all menus and clear the stack

	function menu.hide()
		menu.stack={}
		menu.items=nil
		menu.lines=nil
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

			local it=menu.items[ menu.cursor ]	
		
			if it.call then -- do this
			
				it.call( it )
				
			else -- just back by default
			
				menu.back()
			
			end
		
		end
		
--		if m.keyname=="back" then menu.hide() end
		
		if bleft or bup then
		
			menu.cursor=menu.cursor-1
			if menu.cursor<1 then menu.cursor=#menu.items end

		end
		
		if bright or bdown then
			
			menu.cursor=menu.cursor+1
			if menu.cursor>#menu.items then menu.cursor=1 end
		
		end
	
	end
	
	menu.draw=function()
	
		local tprint=system.components.text.text_print
		local tgrd=system.components.text.tilemap_grd

		if not menu.lines then return end
		
		menu.cy=math.floor((30-(#menu.lines+4))/2)
		
		tgrd:clip(menu.cx,menu.cy,0,menu.width,#menu.lines+4,1):clear(0x02000000)
		tgrd:clip(menu.cx+2,menu.cy+1,0,menu.width-4,#menu.lines+4-2,1):clear(0x01000000)
		
		local top=menu.stack[#menu.stack]

--		yarn_canvas.draw_box(0,0,menu.width,#menu.lines+4)
		
		if top.title then
			local title=" "..(top.title).." "
			local wo2=math.floor(#title/2)
			tprint(title,menu.cx+(menu.width/2)-wo2,menu.cy+0,31,2)
		end
		
		for i,v in ipairs(menu.lines) do
			tprint(v.s,menu.cx+4,menu.cy+i+1,31,1)
		end
		
		local it=nil
		for i=1,#menu.lines do
			if it~=menu.lines[i].item then -- first line only
				it=menu.lines[i].item
				if it == menu.items[menu.cursor] then
					tprint(">",menu.cx+3,menu.cy+i+1,31,1)
				else
--					tprint(".",menu.cx+3,menu.cy+i+1,31,1)
				end
			end
		end

		system.components.text.dirty(true)

	end
	
	return menu
end

function setup_dust()

	local names=system.components.tiles.names
	local dust=entities_set("dust",entities_add{})
	
	dust.parts={}
	
	dust.add=function(it)
		dust.parts[it]=true
		
		local space=entities_get("space")

		it.life=it.life or 60
		it.sprite=it.sprite or names.char_dust.idx
		it.color=it.color or {r=1,g=1,b=1,a=1}		
		it.h=it.h or 8
		it.s=it.s or 1/4

		it.body=space:body(it.mass or 0.1,it.inertia or 0.1)
		it.body:position(it.px,it.py)
		it.body:velocity(it.vx,it.vy)

		if it.shape_args then
			it.shape=it.body:shape(unpack(it.shape_args))
		else
			it.shape=it.body:shape("circle",1,0,0)
		end
		it.shape:friction(it.friction or 0.5)
		it.shape:elasticity(it.elasticity or 0.5)
		
		if it.gravity then
			local gravity_x=it.gravity[1]
			local gravity_y=it.gravity[2]
			it.body:velocity_func(function(body)
				body.gravity_x=gravity_x
				body.gravity_y=gravity_y
				if it.die_speed then
					local vx,vy=body:velocity()
					if (vx*vx+vy*vy) > it.die_speed*it.die_speed then it.life=0 end
				end
				return true
			end)

		end
		
	end

	dust.update=function()
		local space=entities_get("space")
		for it,_ in pairs(dust.parts) do
		
			it.life=it.life-1
			if it.life<0 then
				if it.on_die then it.on_die(it) end
				space:remove(it.shape) -- auto?
				space:remove(it.body)
				dust.parts[it]=nil
			end
			
		end
	end

	dust.draw=function()
		for it,_ in pairs(dust.parts) do

			local px,py=it.body:position()
			local rz=it.body:angle()
			rz=it.draw_rz or rz -- always face up?
			local color=it.color
			if it.fade then
				local a=it.life/it.fade
				color={}
				color.r=it.color.r*a
				color.g=it.color.g*a
				color.b=it.color.b*a
				color.a=a
				
			end
			
			system.components.sprites.list_add({t=it.sprite,s=it.s,h=it.h,hx=it.hx,hy=it.hy,px=px,py=py,rz=180*rz/math.pi,color=color,pz=it.pz})
			
		end
	end
	
	return dust
end

function setup_score()

	local score=entities_set("score",entities_add{})
	
	entities_set("time",{
		game=0,
	})
	
	score.update=function()
		local time=entities_get("time")
		time.game=time.game+(1/screen.fps)
	end

	score.draw=function()
	
		local time=entities_get("time")
	
		local remain=0
		for _,loot in ipairs( entities_items("loot") ) do
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

		local s=""
		
		local level=entities_get("level")
		
		s=level.title or s

		for i,player in pairs(entities_items("player")) do
			if player.near_menu then
				s=player.near_menu.title
			end
		end

		system.components.text.text_print(s,math.floor((system.components.text.tilemap_hx-#s)/2),system.components.text.tilemap_hy-1)
		
	end
	
	return score
end

-- move it like a player or monster based on
-- it.move which is "left" or "right" to move 
-- it.jump which is true if we should jump
function char_controls(it,fast)
	fast=fast or 1

	local time=entities_get("time")

	local jump=fast*200 -- up velocity we want when jumping
	local speed=fast*60 -- required x velocity
	local airforce=speed*2 -- replaces surface velocity
	local groundforce=speed/2 -- helps surface velocity
	
	if ( time.game-it.body.floor_time < 0.125 ) or ( it.floor_time-time.game > 10 ) then -- floor available recently or not for a very long time (stuck)
	
		it.floor_time=time.game -- last time we had some floor

		it.shape:friction(1)

		if it.jump_clr and it.near_menu then
			local menu=entities_get("menu")
			local near_menu=it.near_menu
			local callbacks=entities_manifest("callbacks")
			callbacks[#callbacks+1]=function() menu.show(near_menu) end -- call later so we do not process menu input this frame
		end

		if it.jump then

			local vx,vy=it.body:velocity()

			if vy>-20 then -- only when pushing against the ground a little

				if it.near_menu then -- no jump
				
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
end

function add_monster(opts)

	local space=entities_get("space")

	local monster=entities_add{caste="monster"}

	monster.color=opts.color or {r=0,g=0,b=0,a=1}
	monster.dir=1		
	monster.frame=0
	monster.frames={0x0200,0x0203,0x0200,0x0206}

	monster.active=true
	monster.body=space:body(1,math.huge)
	monster.body:position(opts.px,opts.py)
	monster.body:velocity(opts.vx,opts.vy)
	monster.body.headroom={}
	
	monster.body:velocity_func(function(body)
--				body.gravity_x=-body.gravity_x
--				body.gravity_y=-body.gravity_y
		return true
	end)
				
	monster.floor_time=0 -- last time we had some floor

	monster.shape=monster.body:shape("segment",0,-4,0,4,4)
	monster.shape:friction(1)
	monster.shape:elasticity(0)
	monster.shape:collision_type(0x2001) -- walker
	monster.shape.monster=monster
	monster.shape.triggered=monster
	
	monster.body.floor_time=0


	monster.move="left"

	monster.last_cx=0
	monster.last_cx_count=0

	monster.update=function()
		if monster.active then
		
			if monster.triggered then
				if     monster.triggered.trigger==-1 then
					monster.move="left"
				elseif monster.triggered.trigger== 1 then
					monster.move="right"
				end
			end
			
			local px,py=monster.body:position()
			local cx=math.floor((px+1)/2) -- if this number changes then we are moving
			
			if monster.last_cx == cx then -- not moving
				monster.last_cx_count=monster.last_cx_count+1
				if monster.last_cx_count > 30 then
					monster.triggered=nil
					monster.last_cx_count=0
					if monster.move=="left" then monster.move="right" else monster.move="left" end -- change dir
				end
			else -- moved
				monster.last_cx=cx
				monster.last_cx_count=0
			end
		
		
			char_controls(monster,0.5)
		end
	end


	monster.draw=function()
		if monster.active then
			local px,py=monster.body:position()
			local rz=monster.body:angle()
			monster.frame=monster.frame%16
			local t=monster.frames[1+math.floor(monster.frame/4)]
			
			system.components.sprites.list_add({t=t,h=24,px=px,py=py,sx=monster.dir,sy=1,rz=180*rz/math.pi,color=monster.color})				
		end
	end

end

function add_player(i)
	local players_colors={30,14,18,7,3,22}

	local names=system.components.tiles.names
	local space=entities_get("space")

	local player=entities_add{caste="player"}

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
	player.frames={0x0200,0x0203,0x0200,0x0206}

	player.bubble=function()
		local players_start=entities_get("players_start") or {64,64}
		player.bubble_active=true

		player.bubble_body=space:body(1,1)
		player.bubble_body:position(players_start[1]+i,players_start[2]-i)

		player.bubble_shape=player.bubble_body:shape("circle",6,0,0)
		player.bubble_shape:friction(0.5)
		player.bubble_shape:elasticity(1)

		player.bubble_shape:collision_type(0x2002) -- bubble
		player.bubble_shape.player=player

		player.bubble_body:velocity_func(function(body)
			local px,py=body:position()
			
			body.gravity_x=(players_start[1]-px)*16
			body.gravity_y=(players_start[2]-py)*16
			return true
		end)

	end
	
	player.join=function()
		local players_start=entities_get("players_start") or {64,64}
	
		local px,py=players_start[1]+i,players_start[2]
		local vx,vy=0,0

		if player.bubble_active then -- pop bubble
			px,py=player.bubble_body:position()
			vx,vy=player.bubble_body:velocity()
			space:remove(player.bubble_shape) -- auto?
			space:remove(player.bubble_body)
		end

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

		player.shape=player.body:shape("segment",0,-4,0,4,4)
		player.shape:friction(1)
		player.shape:elasticity(0)
		player.shape:collision_type(0x2001) -- walker
		player.shape.player=player
		
		player.body.floor_time=0
		local time=entities_get("time")
		if not time.start then
			time.start=time.game -- when the game started
		end
	end

	player.die=function()
		if not player.active then return end -- not alive
		
		local px,py=player.body:position()
		local vx,vy=player.body:velocity()

		player.active=false -- die
--			player.dead=true

		space:remove(player.shape) -- auto?
		space:remove(player.body)
		
		local it
		it=add_detritus(names.body_p1.idx,16,px,py-4,0.25,16,0.1,0.5,"box",-4,-3,4,3,0) it.body:velocity(vx*3,vy*3) it.color=player.color
		it=add_detritus(names.body_p2.idx,16,px,py+0,0.25,16,0.1,0.5,"box",-3,-2,3,2,0) it.body:velocity(vx*2,vy*2) it.color=player.color
		it=add_detritus(names.body_p3.idx,16,px,py+4,0.25,16,0.1,0.5,"box",-3,-2,3,2,0) it.body:velocity(vx*1,vy*1) it.color=player.color

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
				
		if not player.bubble_active and not player.active then -- can add as bubble
			if up.button("up") or up.button("down") or up.button("left") or up.button("right") or up.button("fire") then
				player.bubble() -- add bubble
			end
		end

		if player.bubble_active then
			if not player.active then
				if player.jump then
					if player.joined then player.score=player.score-1 end-- first join is free, next join costs 1 point
					player.joined=true
					player:join() -- join for real and remove bubble
				end
			end
		end
		
		if player.bubble_active then
		
			local px,py=player.bubble_body:position()

			if up.button("left") then
				
				player.bubble_body:apply_force(-120,0,px,py,"world")
				player.dir=-1
				player.frame=player.frame+1
				
			elseif  up.button("right") then

				player.bubble_body:apply_force(120,0,px,py,"world")
				player.dir= 1
				player.frame=player.frame+1

			elseif up.button("up") then
				
				player.bubble_body:apply_force(0,-120,px,py,"world")
				
			elseif  up.button("down") then

				player.bubble_body:apply_force(0,120,px,py,"world")

			end

		elseif player.active then
		
			char_controls(player)
		
		end
	end
	

	player.draw=function()
		if player.bubble_active then

			local px,py=player.bubble_body:position()
			local rz=player.bubble_body:angle()
			player.frame=player.frame%16
			local t=player.frames[1+math.floor(player.frame/4)]
			
			system.components.sprites.list_add({t=t,h=24,px=px,py=py,sx=(player.dir or 1)*0.5,s=0.5,rz=180*rz/math.pi,color=player.color})
			
			system.components.sprites.list_add({t=names.bubble.idx,h=24,px=px,py=py,s=1})

		elseif player.active then
			local px,py=player.body:position()
			local rz=player.body:angle()
			player.frame=player.frame%16
			local t=player.frames[1+math.floor(player.frame/4)]
			
			system.components.sprites.list_add({t=t,h=24,px=px,py=py,sx=player.dir,sy=1,rz=180*rz/math.pi,color=player.color})			
		end

		if player.joined then
			local s=string.format("%d",player.score)
			system.components.text.text_print(s,math.floor(player.up_text_x-(#s/2)),0,player.color.idx)
		end

	end
	
	return player
end



function change_level(idx)

	setup_level(idx)
	
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

	local tilemap={}
	for n,v in pairs( levels[idx].legend ) do -- build tilemap from legend
		if v.name then -- convert name to tile
			tilemap[n]=names[v.name]
		end
	end

	local map=entities_set("map", bitdown.pix_tiles(  levels[idx].map,  levels[idx].legend ) )
	
	level.title=levels[idx].title
	
	bitdown.tile_grd( levels[idx].map, tilemap, system.components.map.tilemap_grd  ) -- draw into the screen (tiles)

	local unique=0
	bitdown.map_build_collision_strips(map,function(tile)
		unique=unique+1
		if tile.coll then -- can break the collision types up some more by appending a code to this setting
			if tile.collapse then -- make unique
				tile.coll=tile.coll..unique
			end
		end
	end)

	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			local shape
			if tile.deadly then -- a deadly tile

				if tile.deadly==1 then
					shape=space.static:shape("poly",{x*8+4,y*8+8,(x+1)*8,(y+0)*8,(x+0)*8,(y+0)*8},0)
				else
					shape=space.static:shape("poly",{x*8+4,y*8,(x+1)*8,(y+1)*8,(x+0)*8,(y+1)*8},0)
				end
				shape:friction(1)
				shape:elasticity(1)
				shape.cx=x
				shape.cy=y
				shape:collision_type(0x1002) -- a tile that kills

			elseif tile.solid and (not tile.parent) then -- if we have no parent then we are the master tile
			
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
				if tile.collapse then
					shape:collision_type(0x1003) -- a tile that collapses when we walk on it
					tile.update=function(tile)
						tile.anim=(tile.anim or 0) + 1
						
						if tile.anim%4==0 then
							local dust=entities_get("dust")
							dust.add({
								vx=0,
								vy=0,
								px=(tile.x+math.random())*8,
								py=(tile.y+math.random())*8,
								life=60*2,
								friction=1,
								elasticity=0.75,
							})
						end

						if tile.anim > 60 then
							space:remove( tile.shape )
							tile.shape=nil
							system.components.map.tilemap_grd:pixels(tile.x,tile.y,1,1,{0,0,0,0})
							system.components.map.dirty(true)
							level.updates[tile]=nil
						else
							local name
							if     tile.anim < 20 then name="char_floor_collapse_1"
							elseif tile.anim < 40 then name="char_floor_collapse_2"
							else                       name="char_floor_collapse_3"
							end
							local idx=names[name].idx
							local v={}
							v[1]=(          (idx    )%256)
							v[2]=(math.floor(idx/256)%256)
							v[3]=31
							v[4]=0
							system.components.map.tilemap_grd:pixels(tile.x,tile.y,1,1,v)
							system.components.map.dirty(true)
						end
					end
				elseif not tile.dense then 
					shape:collision_type(0x1001) -- a tile we can jump up through
				end
			end
			if tile.push then
				if shape then
					shape:surface_velocity(tile.push*12,0)
				end
				level.updates[tile]=true
				tile.update=function(tile)
					tile.anim=( (tile.anim or 0) + 1 )%20
					
					local name
					if     tile.anim <  5 then name="char_floor_move_1"
					elseif tile.anim < 10 then name="char_floor_move_2"
					elseif tile.anim < 15 then name="char_floor_move_3"
					else                       name="char_floor_move_4"
					end
					local idx=names[name].idx
					local v={}
					v[1]=(          (idx    )%256)
					v[2]=(math.floor(idx/256)%256)
					v[3]=31
					v[4]=0
					system.components.map.tilemap_grd:pixels(tile.x,tile.y,1,1,v)
					system.components.map.dirty(true)
				end
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

			if tile.loot then
				local loot=add_loot()

				local shape=space.static:shape("box",x*8,y*8,(x+1)*8,(y+1)*8,0)
				shape:collision_type(0x3001)
				shape.loot=loot
				loot.shape=shape
				loot.px=x*8+4
				loot.py=y*8+4
				loot.active=true
			end
			if tile.item then
				local item=add_item()
				
				item.sprite=names.cannon_ball.idx
				item.h=24

				item.active=true
				item.body=space:body(2,2)
				item.body:position(x*8+4,y*8+4)

				item.shape=item.body:shape("circle",8,0,0)
				item.shape:friction(0.5)
				item.shape:elasticity(0.5)

			end
			if tile.start then
				entities_set("players_start",{x*8+4,y*8+4}) --  remember start point
			end
			if tile.monster then
				local item=add_monster{
					px=x*8+4,py=y*8+4,
					vx=0,vy=0,
				}
			end
			if tile.trigger then
				local item=add_item()

				local shape=space.static:shape("box", x*8 - (tile.trigger*6) ,y*8, (x+1)*8 - (tile.trigger*6) ,(y+1)*8,0)
				item.shape=shape
				
				shape:collision_type(0x4001)
				shape.trigger=tile
			end
			if tile.menu then
				local item=add_item()

				item.shape=space.static:shape("box", (x-1)*8,(y-1)*8, (x+2)*8,(y+2)*8,0)
				
				item.shape:collision_type(0x4002)
				item.shape.menu=tile.menu
			end
			if tile.sign then
				local items={}
				tile.items=items
				local px,py=x*8-(#tile.sign)*4 + (tile.sign_x or 0) ,y*8 + (tile.sign_y or 0)
				for i=1,#tile.sign do
					local item=add_item()
					items[i]=item

					item.sprite=tile.sign:byte(i)/2
					item.hx=4
					item.hy=8
					item.s=2

					item.active=true
					item.body=space:body(1,100)
					item.body:position(px+i*8-4 ,py+8 )

					item.shape=item.body:shape("box", -4 ,-8, 4 ,8,0)
					item.shape:friction(1)
					item.shape:elasticity(0.5)
					
					if tile.colors then item.color=tile.colors[ ((i-1)%#tile.colors)+1 ] end
										
					if items[i-1] then -- link
						item.constraint=space:constraint(item.body,items[i-1].body,"pin_joint", 0,-8 , 0,-8 )
						item.constraint:collide_bodies(false)
					end					
				end
				local item=items[1] -- first
				item.constraint_static=space:constraint(item.body,space.static,"pin_joint", 0,-8 , px-4,py )

				local item=items[#tile.sign] -- last
				item.constraint_static=space:constraint(item.body,space.static,"pin_joint", 0,-8 , px+#tile.sign*8+4,py )
			end
			if tile.spill then
				level.updates[tile]=true
				tile.update=function(tile)
					local dust=entities_get("dust")
					dust.add({
						vx=0,
						vy=0,
						px=(tile.x+math.random())*8,
						py=(tile.y+math.random())*8,
						life=60*2,
						friction=1,
						elasticity=0.75,
					})
				end
			end
			if tile.bubble then
				level.updates[tile]=true
				tile.update=function(tile)
					tile.count=((tile.count or tile.bubble.start )+ 1)%tile.bubble.rate
					if tile.count==0 then
						local dust=entities_get("dust")
						dust.add({
							vx=0,
							vy=0,
							px=(tile.x+math.random())*8,
							py=(tile.y+math.random())*8,
							sprite = names.bubble.idx,
							mass=1/64,inertia=1,
							h=24,
							s=1,
							shape_args={"circle",12,0,0},
							life=60*16,
							friction=0,
							elasticity=15/16,
							gravity={0,-64},
							draw_rz=0,
							die_speed=128,
							on_die=function(it) -- burst
								local px,py=it.body:position()
								for i=1,16 do
									local r=math.random(math.pi*2000)/1000
									local vx=math.sin(r)
									local vy=math.cos(r)
									dust.add({
										gravity={0,-64},
										mass=1/16384,
										vx=vx*100,
										vy=vy*100,
										px=px+vx*8,
										py=py+vy*8,
										friction=0,
										elasticity=0.75,
										sprite= names.char_dust_white.idx,
										life=15*(2+i),
									})
								end
							end
						})
					end
				end
			end
			if tile.sprite then
				local item=add_item()
				item.active=true
				item.px=tile.x*8+4
				item.py=tile.y*8+4
				item.sprite = names.door_open.idx
				item.h=24
				item.s=1
				item.draw_rz=0
				item.pz=-1
			end
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

	if fatpix then -- do something funky
		local it=system.components.copper
		it.shader_name="fun_copper_back_y5"
		it.shader_uniforms.cy0={ 0.5  , 0    , 0.0  , 1   }
		it.shader_uniforms.cy1={ 0    , 0    , 1.0  , 1   }
		it.shader_uniforms.cy2={ 0.125, 0.125, 1.0  , 1   }
		it.shader_uniforms.cy3={ 0    , 0    , 1.0  , 1   }
		it.shader_uniforms.cy4={ 0    , 0.5  , 0.0  , 1   }
		
--		it.shader_name="fun_copper_back_wave"
		

	else -- just pick a background color
		local it=system.components.copper
		it.shader_name="fun_copper_back_y5"
		it.shader_uniforms.cy0={ 0    , 0    , 1/4  , 1   }
		it.shader_uniforms.cy1={ 0    , 0    , 1/4  , 1   }
		it.shader_uniforms.cy2={ 0    , 0    , 1/4  , 1   }
		it.shader_uniforms.cy3={ 0    , 0    , 1/4  , 1   }
		it.shader_uniforms.cy4={ 0    , 0    , 1/4  , 1   }

	end

	local menu
	start_level=function(idx)

		entities_reset()

		setup_level(idx) -- load map
		setup_score() -- gui for the score

		setup_dust() -- dust particles

		for i=1,6 do add_player(i) end -- players 1-6
		ups(1).touch="left_fire_right" -- request this touch control scheme for player 1 only
		
		menu=setup_menu()

	end
	
	start_level(0)

-- update loop

	while true do coroutine.yield()
	
		if menu.lines then -- menu only
			menu.update()
		else
			entities_call("update")
			local space=entities_get("space")
			space:step(1/(screen.fps*2)) -- double step for increased stability, allows faster velocities.
			space:step(1/(screen.fps*2))
		end

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
			system.components.text.dirty(true)
			system.components.text.text_window()
			system.components.text.text_clear(0x00000000)
			system.components.sprites.list_reset() -- remove old sprites here
			entities_call("draw") -- because we are going to add them all in again here
		end
		if need.clean then done=true end -- cleanup requested
	end

-- perform cleanup here


end
