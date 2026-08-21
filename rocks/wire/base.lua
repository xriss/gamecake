build={
 modules={
  ["wetgenes.wire"]="lua_wire/code/wire.lua",
  ["wetgenes.wire.core"]={
   incdirs={
    "lua_wire",
    "lua_wire/c11threads/git",
   },
   sources={
    "lua_wire/code/lua_wire.c",
   },
  },
  ["wetgenes.wiretasks"]="lua_wire/code/wiretasks.lua",
  ["wetgenes.getsql"]="getsql.lua",
 },
 platform={
  windows={
   ["wire.core"]={
    sources={
     "lua_wire/code/lua_wire.c",
     "lua_wire/code/c11threads/git/c11threads_win32.c",
    },
   },
  },
 },
 type="builtin",
}
dependencies={
 "lua >= 5.1 <= 5.2",
 "djon",
}
description={
 homepage="https://github.com/xriss/gamecake",
 license="MIT / Two-clause BSD",
 summary="C11 threads and FIFOs across lua states",
 detailed=[[

Provides a way of launching lua states in separate threads and a means 
of communicating with these threads via messages only.

Includes simple tasks for running a non blocking HTTP fetch or SQLITE 
query in another thread, hence the optional luasocket etc dependencies.

See libs/lua_wire/readme.md in the gamecake repo 
https://github.com/xriss/gamecake/blob/master/libs/lua_wire/

 ]],
}
