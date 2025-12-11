#^hello.md

# Welcome To Swanky Edit ( SWED )

To your right you should see a tree file explorer panel where / will be 
the root of your file system and // is used for internal files, eg 
config, these internal files exist only inside the editor and not on 
your filesystem.

Clicking on // ( to expand it ) then gists/ and then finally 
fun64s.HASH/ ( where HASH is the actual gist id so not something you 
should remember ) will load a gist from github ( internet access 
required ). Inside this gist you can find some fun64 examples. These 
will automatically run in the bottom right panel and can be toggle to 
fullscreen by pressing the ESC key. Shadertoy like examples can also be 
found in the shadertoys.HASH/ which is also under gists/.

This is a file editor so you can load and edit files but be aware that 
we constantly save the current state to an sqlite database (referred to 
as a collection). As such exiting and reloading should maintain state. 
Also we retain changes past saving. So closing and then reopening a 
previously saved file will allow you to undo changes and revert that 
file to its state before it was saved.

Right now this is still a work in progress so try the examples and 
maybe even mess around a bit with some of the example code, some of the 
shaders have sliders you can adjust for instance.
