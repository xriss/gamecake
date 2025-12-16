
gonna need something that does the inter task comms thing like this maybe

https://blog.bearcats.nl/simple-message-queue/

and probably users message pack although maybe just the parts that make lua sense

https://github.com/msgpack/msgpack/blob/master/spec.md

and I think maybe a way of readin and addressing parts without having to convert the whole message

possibly a jit wrapper  that lets you pretend its a table or array for dynamic access via meta without unpacking it.

probably similar to  https://github.com/catwell/luajit-msgpack-pure but a bit more dynamic

for now I think I'm jsut going to be doing a lot of data dump and copying between tasks as a replacement for lanes linda lock dodgyness (rando 20+ms spikes)
that I cannot be bothered to debug might as well just replace, then we can dump lanes and just use pthreds


TODO

Need named pipes, to spot a previously run app and possibly send new commands to it,  and that should probably happen here.
