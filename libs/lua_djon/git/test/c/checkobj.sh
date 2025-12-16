#!/bin/sh
cd `dirname $0`

gcc -c checkobj.c -o checkobj.o
objdump --syms checkobj.o >checkobj.txt

