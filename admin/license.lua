#!/usr/bin/env gamecake

local lcs={

	["lib_chipmonk"]	="lib_chipmonk/LICENSE.txt",
	["lib_freetype"]	="lib_freetype/freetype/docs/LICENSE.TXT",
	["lib_gif"]			="lib_gif/COPYING",
	["lib_hidapi"]		="lib_hidapi/LICENSE-bsd.txt",
	["lib_jpeg"]		="lib_jpeg/README",
	["lib_lua"]			="lib_lua/COPYRIGHT",
	["lib_luajit"]		="lib_luajit/COPYRIGHT",
	["lib_ogg"]			="lib_ogg/COPYING",
	["lib_openal"]		="lib_openal/asoft/COPYING",
	["lib_openssl"]		="lib_openssl/LICENSE",
	["lib_pcre"]		="lib_pcre/LICENCE",
	["lib_png"]			="lib_png/LICENSE",
	["lib_vorbis"]		="lib_vorbis/COPYING",
	["lib_vpx"]			="lib_vpx/git/LICENSE",
	["lib_z"]			="lib_z/README",
	["lib_zzip"]		="lib_zzip/COPYING.LIB",

	["lua_bit"]			="lua_bit/README",
	["lua_lanes"]		="lua_lanes/COPYRIGHT",
	["lua_lash"]		="lua_lash/src/LICENSE",
	["lua_lfs"]			="lua_lfs/LICENSE",
	["lua_posix"]		="lua_posix/COPYING",
	["lua_profiler"]	="lua_profiler/LICENSE",
	["lua_sec"]			="lua_sec/LICENSE",
	["lua_socket"]		="lua_socket/LICENSE",
	["lua_zip"]			="lua_zip/LICENSE",
	["lua_zlib"]		="lua_zip/README",

}

local wbake=require("wetgenes.bake")

local head=[[
Files: *
Copyright: 2013 Kriss Blank <kriss@wetgenes.com>
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
	print("Copyright: ...")
	print("License: ...")
	print(t.."\n")

end
