#!/usr/local/bin/gamecake


args={...}

vplus=(tonumber(args[1] or 0) or 0)

wbake=require("wetgenes.bake")

version=wbake.version_from_time(os.time(),vplus)

ss="GAMECAKE_VERSION=\""..version.."\"\n"
wbake.writefile("../exe_gamecake/version.lua",ss)

print(version)


