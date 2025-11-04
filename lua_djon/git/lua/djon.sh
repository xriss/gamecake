#!/bin/sh

eval `luarocks --lua-version 5.1 path`
#gdb -q --eval-command=r --eval-command=bt --eval-command=q --args luajit -- `dirname $0`/djon.cmd.lua "$@"
luajit -- `dirname $0`/djon.cmd.lua "$@"

