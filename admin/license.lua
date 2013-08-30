#!/usr/bin/env gamecake

local lcs={

	["lib_chipmonk"]	="lib_chipmonk/LICENSE.txt",
	["lib_freetype"]	="lib_freetype/freetype/docs/LICENSE.TXT",
	["lib_gif"]			="lib_gif/COPYING",
	["lib_hidapi"]		="lib_hidapi/LICENSE-bsd.txt",
	["lib_jpeg"]		="lib_jpeg/jpeg-6b/README",
	["lib_lua"]			="lib_lua/COPYRIGHT",
	["lib_luajit"]		="lib_luajit/COPYRIGHT",
	["lib_ogg"]			="lib_ogg/COPYING",
	["lib_openal"]		="lib_openal/asoft/COPYING",
	["lib_openssl"]		="lib_openssl/LICENSE",
	["lib_pcre"]		="lib_pcre/LICENSE",

}

local wbake=require("wetgenes.bake")

for n,v in pairs(lcs) do

	local d=wbake.readfile(v)
	
	print(n.." : "..(#d) )

end
