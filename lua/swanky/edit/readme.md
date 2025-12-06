#^hello.md

# Welcome To Swanky Edit

To your right you should see a tree file explorer panel where / will be 
the root of your file system and // is used for internal files, eg 
config, that exist only inside the editor and not on your filesystem.

Clicking on // then gists and then finally one of the hashs will load a 
gist from github provided we have internet access. Here you can find 
some fun64 or GL shader(toy) examples. These will automatically run in 
the bottom right panel and can toggle fullscreen by pressing the ESC 
key.

Apart from that this is a file editor you can edit files but be aware 
that we constantly save the current state to an sqlite database 
(refered to as a collection). As such exiting and reloading should get 
you back to exactly where you where before. Also we retain changes so 
opening a previosly saved file will allow you to undo changes and 
revert that file to its previous states.
