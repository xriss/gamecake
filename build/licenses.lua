#!/usr/bin/env gamecake

local lcs={

	["lib_freetype"]	="lib_freetype/freetype/docs/LICENSE.TXT",
	["lib_gif"]			="lib_gif/giflib/COPYING",
	["lib_jpeg"]		="lib_jpeg/jpeg/README",
	["lib_lua"]			="lib_lua/COPYRIGHT",
	["lib_ogg"]			="lib_ogg/ogg/COPYING",
	["lib_openal"]		="lib_openal/mojoal/LICENSE.txt",
	["lib_opus"]		="lib_opus/opus/COPYING",
	["lib_pcre"]		="lib_pcre/LICENCE",
	["lib_png"]			="lib_png/fixed/LICENSE",
	["lib_pq"]			="lib_pq/COPYRIGHT",
	["lib_speexdsp"]	="lib_speexdsp/speexdsp/COPYING",
--	["lib_sqlite"]		=PUBLIC_DOMAIN,
	["lib_vorbis"]		="lib_vorbis/vorbis/COPYING",
	["lib_wolfssl"]		="lib_wolfssl/LICENSE",
	["lib_z"]			="lib_z/README",
	["lib_zip"]			="lib_zip/LICENSE",
	["lib_zzip"]		="lib_zzip/COPYING.LIB",

--	["lua_al"]			=GAMECAKE_LICENSE,
	["lua_bit"]			="lua_bit/README",
	["lua_brimworkszip"]="lua_brimworkszip/README",
	["lua_bullet"]		="lua_bullet/LICENSE.txt",
	["lua_chipmunk"]	="lua_chipmunk/master/LICENSE.txt",
	["lua_cmsgpack"]	="lua_cmsgpack/README.md",
	["lua_djon"]		="lua_djon/git/LICENSE.md",
--	["lua_fats"]		=GAMECAKE_LICENSE,
--	["lua_freetype"]	=GAMECAKE_LICENSE,
--	["lua_gamecake"]	=GAMECAKE_LICENSE,
--	["lua_gles"]		=GAMECAKE_LICENSE,
	["lua_glslang"]		="lua_glslang/glslang/LICENSE.txt",
--	["lua_grd"]			=GAMECAKE_LICENSE,
--	["lua_grdmap"]		=GAMECAKE_LICENSE,
	["lua_kissfft"]		="lua_kissfft/kissfft/COPYING",
	["lua_lanes"]		="lua_lanes/lanes/COPYRIGHT",
	["lua_lash"]		="lua_lash/src/LICENSE",
	["lua_lfs"]			="lua_lfs/LICENSE",
	["lua_linenoise"]	="lua_linenoise/COPYING",
--	["lua_midi"]		=GAMECAKE_LICENSE,
--	["lua_ogg"]			=GAMECAKE_LICENSE,
--	["lua_opus"]		=GAMECAKE_LICENSE,
--	["lua_pack"]		=GAMECAKE_LICENSE,
	["lua_pegasus"]		="lua_pegasus/LICENSE",
	["lua_periphery"]	="lua_periphery/LICENSE",
	["lua_pgsql"]		="lua_pgsql/README",
	["lua_posix"]		="lua_posix/COPYING",
	["lua_rex"]			="lua_rex/LICENSE",
	["lua_sdl2"]		="lua_sdl2/luasdl2/LICENSE",
	["lua_sec"]			="lua_sec/LICENSE",
	["lua_socket"]		="lua_socket/LICENSE",
--	["lua_sod"]			=GAMECAKE_LICENSE,
--	["lua_sqlite"]		=MIT_LICENSE",
	["lua_sys"]			="lua_sys/luasys/COPYRIGHT",
--	["lua_tardis"]		=GAMECAKE_LICENSE,
--	["lua_utf8"]		=LUA_LICENSE,
--	["lua_v4l2"]		=GAMECAKE_LICENSE,
--	["lua_win"]			=GAMECAKE_LICENSE,
--	["lua_win_emcc"]	=GAMECAKE_LICENSE,
	["lua_zip"]			="lua_zip/LICENSE",
	["lua_zlib"]		="lua_zlib/src/README",

}

local wbake=require("wetgenes.bake")

local head=[[
Files: *
License: The MIT License (MIT)
 Copyright (c) 2013 Kriss Blank <kriss@wetgenes.com>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

]]


print(head)
for n,v in pairs(lcs) do

	local d=wbake.readfile(v)
	
	local t=" "..string.gsub(d,"\n","\n ")
	
	print("Files: "..n.."/*")
	print("License: ...")
	print(t.."\n")

end
