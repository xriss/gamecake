
lua_wire

C11 threads and fifo message queues for lua, needs some small hacks for 
windows which is just to enable C11 threads....

Online documentation built from source code comments :
https://xriss.github.io/gamecake/docs/lua.wire/


We use a lightly modified version of lua-cmsgpack , static builtin for 
building message buffers from lua tables.

The table array sniffing is simplified and not guarenteed to spot an 
object. If a [1] key exists then we assume array. This is as oposed to 
doing a table search for all keys before serialising.

More hacks or possible replacement is possible so it is unsafe to rely 
on this internal format.

	version 1.0

Mostly harmless.


