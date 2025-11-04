The initial C implementation of DJON flavored JSON.

Although this is written in C it is mostly intended to be used from Lua 
or JS rather than from C.

Mostly this is due to string use and garbage collection, Lua is simply 
my goto library for such things. I think the included path system works 
for standalone C but my choice would probably be to use Lua instead.

