
ROCK_ROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source <(luarocks path)

cd ../test

# need to use a gamecake-loader.lua file that just does a require("gamecake")
# this is needed to get all the command line args parsed and into _G.arg
# this hack works on luajit and lua
# gamecake module can now pretend it is a lua command
LUA_PATH="$LUA_PATH;lua/?.lua" luajit "$ROCK_ROOT/gamecake/src/lua_gamecake/code/gamecake.lua" -- lua/unit.lua


# restore files we just broke
git checkout dat

