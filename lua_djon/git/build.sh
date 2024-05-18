#!/bin/sh
cd `dirname $0`

c/build.sh

luarocks --lua-version 5.1 build --local

js/build.sh
npm install --prefix js

