#!/usr/local/bin/gamecake


local args={...}

local wbake=require("wetgenes.bake")

local version=	string.format([[ return { version="%02.04f" } ]],
		( ( os.time() / ( 365.25*24*60*60 ) ) - 30 ) )

local ss="GAMECAKE_VERSION=\""..version.."\"\n"

wbake.writefile("../exe_gamecake/version.lua",ss)

print(version)
