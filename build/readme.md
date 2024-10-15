

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.path


Manage file paths under linux or windows, so we need to deal with \ or 
/ and know the root difference between / and C:\

	local wpath=require("wetgenes.path")



## lua.wetgenes.path.currentdir


Get the current working directory, this requires lfs and if lfs is not 
available then it will return wpath.root this path will have a trailing 
separator so can be joined directly to a filename.

	wpath.currentdir().."filename.ext"



## lua.wetgenes.path.join


join a split path, tables are auto expanded



## lua.wetgenes.path.normalize


remove ".." and "." components from the path string



## lua.wetgenes.path.parent


Resolve input and go up a single directory level, ideally you should 
pass in a directory, IE a string that ends in / or \ and we will return 
the parent of this directory.

If called repeatedly, then eventually we will return wpath.root



## lua.wetgenes.path.parse


split a path into named parts like so

	|--------------------------------------------|
	|                     path                   |
	|-----------------------|--------------------|
	|         dir           |        file        |
	|----------|------------|----------|---------|
	| root [1] | folder [2] | name [3] | ext [4] |
	|----------|------------|----------|---------|
	| /        | home/user/ | file     | .txt    |
	|----------|------------|----------|---------|

this can be reversed with simple joins and checks for nil, note that 
[1][2][3][4] are forced strings so will be "" rather than nil unlike 
their named counterparts. This means you may use wpath.join to reverse 
this parsing.

	dir = (root or "")..(folder or "")
	file = (name or "")..(ext or "")
	path = (dir or "")..(file or "")
	path = (root or "")..(folder or "")..(name or "")..(ext or "")
	path = [1]..[2]..[3]..[4]
	path = wpath.join(it)
	
if root is set then it implies an absolute path and will be something 
like C:\ under windows.



## lua.wetgenes.path.relative


Build a relative path from point a to point b this will probably be a 
bunch of ../../../ followed by some of the ending of the second 
argument.



## lua.wetgenes.path.resolve


Join all path segments and resolve them to absolute using wpath.join 
and wpath.normalize with a prepended wpath.currentdir as necessary.



## lua.wetgenes.path.setup


setup for windows or linux style paths, to force one or the other use

	wpath.setup("win")
	wpath.setup("nix")

We automatically call this at startup and make a best guess, you can 
revert to this best guess with

	wpath.setup()

This is a global setting, so be careful with changes. Mostly its 
probably best to stick with the best guess unless we are mistakenly 
guessing windows.



## lua.wetgenes.path.split


split a path into numbered components
