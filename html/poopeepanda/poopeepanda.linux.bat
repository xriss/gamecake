#!/usr/bin/env bash
cd "$(dirname "$0")"

if [ "`uname -m`" = "aarch64" ] ; then

	exe/gamecake.a64 poopeepanda.fun.lua

else

	exe/gamecake.x64 poopeepanda.fun.lua

fi
