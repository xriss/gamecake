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

Apart from that this is a file editor you can edit files but be aware 
that we constantly save the current state to an sqlite database 
(refered to as a collection). As such exiting and reloading should get 
you back to exactly where you where before. Also we retain changes so 
opening a previously saved file will allow you to undo changes and 
revert that file to its previous states.

Right now this is still a work in progress so try the examples and 
maybe even mess around a bit with some of the example code, some of the 
shaders have sliders you can adjust for instance.
