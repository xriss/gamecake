
# gamecake

- v26.260812
	- Second luarocks release with all gamecake libs set as dependent 
	rocks and mostly versioned to v0.9 since I am not sure this is 
	working well yet.

- v22.001
	- initial luarocks release all code in one rock, missing lots.

Lua code documentation auto built from source comments can be found at 
https://xriss.github.io/gamecake/docs/

We bundle all gamecake code inside a custom loaders that are appended 
to packages.loaders

Two binfiles are provided

	gamecakerock

Which runs gamecake.lua in the standard luarocks lua

	gamecakejit

Which trys to run gamecake.lua using luajit if available, with fall 
back to standard luarocks lua.

Jit version is recommended.

These commands mostly mimic the standard lua command line interface but 
with extras. eg:

	gamecakejit -lcmd

For builtin commands, such as the swed editor

	gamecakejit -lcmd swed

We automatically run .fun.lua files (all in one game text files) inside a fun 
harness and even have a few examples builtin.

	gamecakejit lua/fun/poopeepanda.fun.lua

We automatically run .cake files (zip with lua and assets) by mounting 
the zip and running lua/init.lua to start it. gamecake .apk files are 
very similar to cake files (just with mangled filenames for android) 
and can also be run on as if they are cake files.

