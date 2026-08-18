
export ROCK_ROOT=`pwd`


source <(luarocks path)

# need to use a gamecake-loader.lua file that just does a require("gamecake")
# this is needed to get all the command line args parsed and into _G.arg
# this hack works on luajit and lua
# gamecake module can now pretend it is a lua command
LUA_PATH="$LUA_PATH;lua/?.lua" luajit ${ROCK_ROOT}/gamecake-loader.lua -- "$@"
