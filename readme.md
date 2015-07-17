[![Build Status](https://drone.io/bitbucket.org/xixs/lua/status.png)](https://drone.io/bitbucket.org/xixs/lua/latest) bleeding binaries at https://drone.io/bitbucket.org/xixs/lua/files

This is the collection of Lua libraries and their dependencies that are
built into the gamecake and pagecake engine .

C++ code/dependencies are avoided if at all possible but some 
still remains.

Unless noted otherwise the snapshots of public libraries will be 
unmodified except to contain a premake4.lua build control file in their 
directory and the occasional build/bug/tweak patch.

The bin dir (expected to be checked out side by side with this repo) 
is a testbed containing pre-built binaries and lua scripts. This is 
the final output of this build and is then shared with other, 
projects. This directory must exist when building as it contains Lua
source which is packaged into the output executable.

I know a little bit confusing that lua is full of C code and bin is 
full of Lua code but that's how things are.

To perform a normal build try the following.

	sudo ./apt-gets  # install all build dependencies
	./make           # build using gcc, see below for more options
	sudo ./install   # copy into /usr/local/bin/gamecake

After which gamecake will now be a valid command :)

The following commands should build everything under ubuntu/debian 
assuming you have a build environment setup, you will need to install 
some dev packages such as opengl. Hopefully all the bits you need can 
be installed by running ./apt-gets first. Note I have switched to clang 
so make sure you have that installed. gcc/make will also probably work 
but is not tested as often.

	clang/make


For windows I've switched to a mingwin cross compile. This assumes 
you have my sdks repo checked out side by side to this lua one. 
There are update or build scripts in the sdks repo that should be 
run to grab all sorts of horrible sdk and build files.

	mingw/make


Build for raspi using a cross-compiler that is built in sdks, you will 
need to build the cross compiler first. Currently we have a slight SDL2 problem
as it is not yet available under raspbian but will be needed to run gamecake.
So the appropriate .so needs to be included along with the gamecake binary.

	raspi/make


NaCl build using the sdk found in sdks so will not work if you have 
not populated that first.

	nacl/make

Emscripten build using the sdk found in sdks so will not work if you have 
not populated that first.

	emcc/make

Build for android, uses the ndk found in sdks and you need to bake a 
lua project into the source. IE it spits out an apk file that can be 
installed onto anandroid device, this apk must contain everything 
needed to run an app.

	android/make

Before running make you will need to import a project using

	android/bake projectdir 

For example any of the directories in gamecakejam can be built on
android this way.


We also have an nginx target which bakes all the lua goodies into 
nginx. This can be seen serving such websites as 
http://gamecake.4lfa.com/ It is built automatically alongside the 
main build on linux systems.


All the above scripts just call premake and premake does of course also 
support other build tools as output targets but these are the ones I use 
and test and you can assume are working.

If you are very very lucky then the linux and windows/wine binaries 
found under bin will simply work and there will be no need to build 
anything. These binaries actually have all of the lua scripts 
found in bin/lua baked into them so are "standalone" binary blobs. 

Just grab the appropriate binary from there and this engine is ready 
to go.