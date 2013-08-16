echo "linking .lua fles from the main lua dir so we dont accidently edit multiple copies"

#you will need to delete the local copies first...
#rm code/wetgenes/pack.lua

ln ../../bin/lua/wetgenes/pack.lua code
