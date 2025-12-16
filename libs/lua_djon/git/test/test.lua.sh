#!/bin/sh
cd `dirname $0`

eval `luarocks --lua-version 5.1 path`

luajit ./test.lua -- "$@"

