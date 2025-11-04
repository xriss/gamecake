#!/bin/sh
cd `dirname $0`

# copy test file
cp base.djon test.djon

eval `luarocks --lua-version 5.1 path`
luajit -- eg.lua "$@"

cat test.djon
