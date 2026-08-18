build={
 type="builtin",
 modules={

      ["wetgenes.tardis"]     = "lua_tardis/code/tardis.lua",
      ["wetgenes.tardis.core"]={
         sources={
            "lua_tardis/code/lua_tardis.c",
         },
         incdirs={
            "lua_tardis",
            "lib_hacks/code",
         },
      },

      ["gamecake"]            = "lua_gamecake/code/gamecake.lua",
      ["wetgenes.gamecake.core"]={
         sources={
            "lua_gamecake/code/lua_gamecake.c",
            "lua_gamecake/cache/cache_funcs.c",
            "lua_gamecake/cache/cache.c",
         },
         incdirs={
            "lua_gamecake/code",
            "lua_gles/include",
            "lib_hacks/code",
         },
		defines={
			"INCLUDE_GLES_GL=\"GL/gl3w.h\"",
			"GAMECAKE_NOLIBS",
		},
      },
 },
 install = {
  bin = {
   ['gamecakerock'] = 'gamecakerock',
  },
 },
 platform={
  windows={
  },
 },
}
external_dependencies = {
}
dependencies={
 "lua >= 5.1 <= 5.2",

-- generic modules

 "luautf8",
 "luafilesystem",
 "luaposix",
 "lua-zlib",
 "luasocket",
 "luasec",
 "lsqlite3",
-- "luapgsql",
 "lua-cmsgpack",
 "Lrexlib-PCRE",
 "djon",
 "Lua-SDL2",

-- custom gamecake modules

 "gamecake-al",
 "gamecake-box2d",
 "gamecake-chipmunk",
 "gamecake-fats",
 "gamecake-gles",
 "gamecake-grd",
 "gamecake-ogg",
 "gamecake-opus",
 "gamecake-pack",
 "gamecake-sod",
 "gamecake-win",
 "gamecake-wire",
 "gamecake-zip",
 "gamecake-kissfft",
 "gamecake-dumbft",
 
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua/",
 license="MIT",
 summary="A collection of modules from the gamecake project",
 detailed=[[

See https://github.com/xriss/gamecake for the source.

This pulls in as many dependencies as it can to try and mimic the 
modules that gamecake provides in a luarocks environment.

This is not a perfect copy but is reasonably close and is primarily 
used to share gamecake code with openresty.


 ]],
}

