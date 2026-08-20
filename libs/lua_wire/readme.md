
# gamecake-win

- v0.9
	- initial rocks release.

Lua code documentation auto built from source comments can be found at 
https://xriss.github.io/gamecake/docs/
https://xriss.github.io/gamecake/docs/lua.wire/

C11 threads and fifo message queues for lua, needs some small hacks for 
windows which is just to help enable C11 threads.

Contains a builtin static slightly modified version of cmsgpack ( used 
for squirting lua data between threads and will not conflict with a 
real install of cmsgpack ) from 

The table array sniffing is simplified and not guarenteed to spot an 
object. If a [1] key exists then we assume array. This is as oposed to 
doing a table search for all keys before serialising.

More hacks or possible replacement is possible so it is unsafe to rely 
on this internal format.

https://github.com/antirez/lua-cmsgpack

it is just safer to have an internal version that we can explicitly 
control and hack, also it might get replaced.

Also contains windows thread hacks from

https://github.com/jtsiomb/c11threads

to help with mingwin C11 builds
