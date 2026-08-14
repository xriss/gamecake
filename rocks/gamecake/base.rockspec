build={
 modules={
      ["wetgenes.string"] = "lua_src/wetgenes/string.lua",
      ["wetgenes.json"]   = "lua_src/wetgenes/json.lua",
      ["wetgenes"]        = "lua_src/wetgenes/init.lua",

      ["cmd"]        = "lua_src/cmd/init.lua",
      ["cmd.args"]   = "lua_src/cmd/args.lua",
      ["cmd.dump"]   = "lua_src/cmd/dump.lua",
      ["cmd.grd"]    = "lua_src/cmd/grd.lua",
      ["cmd.help"]   = "lua_src/cmd/help.lua",
      ["cmd.http"]   = "lua_src/cmd/http.lua",
      ["cmd.midi"]   = "lua_src/cmd/midi.lua",
      ["cmd.sdl"]    = "lua_src/cmd/sdl.lua",
      ["cmd.args"]   = "lua_src/cmd/args.lua",
      ["cmd.avatar"] = "lua_src/cmd/avatar.lua",
      ["cmd.swed"]   = "lua_src/cmd/swed.lua",

 },
 platform={
  windows={
  },
 },
 type="builtin",
}
external_dependencies = {
}
dependencies={
 "lua >= 5.1 <= 5.2",
 "gamecake-grd",
 "gamecake-wire",
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
