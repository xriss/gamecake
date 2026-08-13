build={
 modules={
  wire="lua_wire/code/wire.lua",
  ["wire.core"]={
   incdirs={
    "lua_wire",
    "lua_wire/c11threads/git",
   },
   sources={
    "lua_wire/code/lua_wire.c",
   },
  },
  wiretasks="lua_wire/code/wiretasks.lua",
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
 "luasocket",
 "luasec",
 "lsqlite3",
}
description={
 homepage="https://xriss.github.io/gamecake/docs/lua.wire/",
 license="MIT / Two-clause BSD",
 summary="C11 threads and FIFOs across lua states",
 detailed=[[

Provides a way of launching lua states in separate threads and a means 
of communicating with these threads via messages only.

Includes simple tasks for running a non blocking HTTP fetch or SQLITE 
query in another thread, hence the luasocket etc dependencies.

See https://github.com/xriss/gamecake/tree/master/libs/lua_wire and 
https://xriss.github.io/gamecake/docs/lua.wire/ for more docs.

Contains a builtin static slightly modified version of cmsgpack ( used 
for squirting lua data between threads and will not conflict with a 
real install of cmsgpack ) from 

https://github.com/antirez/lua-cmsgpack

it is just safer to have an internal version that we can explicitly 
control and hack, also it might get replaced.

Also contains windows thread hacks from

https://github.com/jtsiomb/c11threads

to help with mingwin C11 builds

 ]],
}
