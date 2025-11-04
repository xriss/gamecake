#!/bin/sh
cd `dirname $0`

# copy test file
cp base.djon test.djon

node -- eg.js "$@"

cat test.djon
