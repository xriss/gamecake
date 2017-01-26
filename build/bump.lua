#!/usr/local/bin/gamecake

local args={...}

local vplus=(tonumber(args[1] or 0) or 0)

local wbake=require("wetgenes.bake")

version=wbake.version_from_time(os.time(),vplus)

local ss="GAMECAKE_VERSION=\""..version.."\"\n"
wbake.writefile("../exe_gamecake/version.lua",ss)

print(version)
