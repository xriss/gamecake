
source <(luarocks path)

cd ../test

LUA_PATH="$LUA_PATH;lua/?.lua" luajit lua/unit.lua


