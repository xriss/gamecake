#!/bin/sh
cd `dirname $0`

cd ..
luarocks --lua-version 5.1 build --local

