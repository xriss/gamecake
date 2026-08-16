
source <(luarocks path)

cd ../test

LUA_PATH="$LUA_PATH;lua/?.lua" luajit lua/unit.lua


# restore files we just broke
git checkout dat

