#!/usr/local/bin/gamecake

-- we do not want to pickup non embeded data by default
--[[
local luapath,apppath=require("apps").default_paths(debug.getinfo(1,"S").source:match("^@(.*[/\\])")) -- default search paths so things can easily be found
]]
-- we should be good to work with *only* internal files so disable *all* module path loading
package.path=""
package.cpath=""
-- this is so this installed swed will continue to work using only its internal installed code

local global=require("global") -- prevent accidental global use

local opts={
--disable_sounds=true,
	times=true, -- request simple time keeping samples
	width=1280,
	height=720,
	name="swanky",
	version=require("swanky.avatar.version").version,
	fps="auto",
	title="Swanky Avatar",
	name="swanky.avatar",
	start="swanky.avatar.main",
	disable_sounds=true, -- we have no sounds
	icon=[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . 
. 0 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 0 . . 
. 0 7 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 7 0 . . 
. 0 7 0 7 7 7 0 7 0 7 0 7 7 7 0 7 7 0 0 7 0 . . 
. 0 7 0 7 0 0 0 7 0 7 0 7 0 0 0 7 0 7 0 7 0 . . 
. 0 7 0 7 7 7 0 7 7 7 0 7 7 0 0 7 0 7 0 7 0 . . 
. 0 7 0 0 0 7 0 7 7 7 0 7 0 0 0 7 0 7 0 7 0 . . 
. 0 7 0 7 7 7 0 7 0 7 0 7 7 7 0 7 7 0 0 7 0 . . 
. 0 7 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 7 0 . . 
. 0 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 0 . . 
. 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
]],
	unpack(arg) -- include commandline opts
}

math.randomseed( os.time() ) -- try and randomise a little bit better

-- setup oven with vanilla cake setup and save as a global value
global.oven=require("wetgenes.gamecake.oven").bake(opts).preheat()

-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
return oven:serv()
