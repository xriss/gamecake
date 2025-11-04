#!/bin/sh
cd `dirname $0`

c/build.sh

lua/build.sh

js/build.sh
cd js
npm install

