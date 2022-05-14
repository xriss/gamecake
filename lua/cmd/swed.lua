#!/usr/local/bin/gamecake

local luapath,apppath=require("apps").default_paths(debug.getinfo(1,"S").source:match("^@(.*[/\\])")) -- default search paths so things can easily be found

local global=require("global") -- prevent accidental global use

local opts={
--disable_sounds=true,
	times=true, -- request simple time keeping samples
	width=1280,
	height=720,
	name="swanky",
	version="Alpha+11c",
	fps="auto",
	title="Swanky Edit",
	name="swanky.edit",
	start="swanky.edit.main",
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
	... -- include commandline opts
}

math.randomseed( os.time() ) -- try and randomise a little bit better

-- setup oven with vanilla cake setup and save as a global value
global.oven=require("wetgenes.gamecake.oven").bake(opts).preheat()

-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
return oven:serv()
