#!/usr/local/bin/gamecake

local bake=require("wetgenes.bake")
--dofile("../../funcs.lua")

local version=bake.version_from_time()
local opts={
        name="GameCake",
        namev="GameCake.v"..version,
        version=version,
        version_int=math.floor(version*1000),
}

bake.replacefile("AndroidManifest.xml.base","AndroidManifest.xml",opts)
bake.replacefile("build.xml.base","build.xml",opts)


