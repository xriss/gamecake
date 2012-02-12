echo "linking .lua fles from my main bin dir"

#you will need to delete the local copies first
#rm code/al.lua
#rm code/alc.lua

ln ../../bin/lua/al.lua code
ln ../../bin/lua/alc.lua code
