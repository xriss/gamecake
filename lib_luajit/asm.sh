#build all we can on this machine which is most bits (64bit linux host)
./asm.lua all

#build lsb versions 32 and 64 (luajit does not want to build under LSB?)
#so this does not work...
#schroot -c saucy_amd64 -u kriss -- bash -c "cd ~/hg/lua/lib_luajit;luajit asm.lua lsb64"
#schroot -c saucy_i386 -u kriss -- bash -c "cd ~/hg/lua/lib_luajit;luajit asm.lua lsb32"

#finally jump through hoops to deal with macs
#build on remote mac machine and grab the binary back to this system
#luajit must be installed already eg in this dir do -> make && sudo make install
ssh donald.local " bash --login -c \"cd hg/lua;hg fetch\""
ssh donald.local " bash --login -c \"cd hg/lua/lib_luajit;luajit asm.lua osx\""
ssh donald.local " bash --login -c \"cd hg/lua;hg addremove;hg ci -mosx;hg push\""

hg addremove
hg ci -mluajitbump
hg fetch
hg push
