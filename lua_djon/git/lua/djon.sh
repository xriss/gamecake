#!/bin/sh

eval `luarocks --lua-version 5.1 path`
luajit -- `dirname $0`/djon.cmd.lua "$@"
