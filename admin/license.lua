#!/usr/bin/env gamecake

local lcs={

	["lib_chipmonk"]	="lib_chipmonk/LICENSE.txt",
	["lib_freetype"]	="lib_freetype/freetype/docs/LICENSE.TXT",
	["lib_gif"]			="lib_gif/COPYING",
	["lib_hidapi"]		="lib_hidapi/LICENSE-bsd.txt",

}

local wbake=require("wetgenes.bake")

for n,v in pairs(lcs) do

	local d=wbake.readfile(v)
	
	print(n.." : "..(#d) )

end
