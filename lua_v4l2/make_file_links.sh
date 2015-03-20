echo "linking .lua fles from the main lua dir so we dont accidently edit multiple copies"

#you will need to delete the local copies first...
#rm code/wetgenes/v4l2.lua

ln ../../bin/lua/wetgenes/v4l2.lua code
