
This is my windows defenestration attempt, please build and run 
under Xubuntu if you have a choice. The Ubuntu and Gnome3 fancy 
desktops impact the performance of OpenGL code and are not 
recomended as development or runtime environments.

All C code is packaged up inside lua libraries and C++ 
code/dependencies have been removed from the project.

So this dir contains lua modules and premake build scripts for them 
either my own or snapshots of public libraries possibly with custom 
wrappers.

Unless noted otherwise the snapshots of public libraries will be 
unmodified except to contain a premake4.lua build control file in their 
directory and the occasional build/bug/tweak patch.

The bin dir (expected to be checked out side by side with this repo) 
is a testbed containing pre-built binaries and lua scripts. This is 
the final output of this build and is then shared with other, 
projects. This dir must exist when building.

	sudo ./apt-gets  # install all build dependencies
	./make           # build using gcc, see below for more options
	sudo ./install   # copy into /usr/local/bin/gamecake

Afterwhich gamecake will now be a valid command :)


The following commands should build everything under ubuntu/debian 
assuming you have a build environment setup, you will need to install 
some dev packages such as opengl. Hopefully all the bits you need can 
be installed by running ./apt-gets first. Note I have switched to
clang so makes sure you have that installed.

	clang/make


For windows I've swiched to a mingwin cross compile. This assumes 
you have my sdks repo checked out side by side to this lua one. 
There are update or build scripts in the sdks repo that should be 
run to grab all sorts of horrible sdk and build files.

	mingw/make


Build for raspi using a crosscompiler that is built in sdks, you will 
need to build the cross compiler first.

	raspi/make


NaCl build using the sdk found in sdks so will not work if you have 
not populated that first.

	nacl/make


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
suport other build tools as output targets but these are the ones I use 
and test and you can assume are working.

If you are very very lucky then the linux and windows/wine binaries 
found under bin will simply work and there wll be no need to build 
anything. These Binaries actually have all of the lua scripts in 
found in bin/lua baked into them so are "standalone" binary blobs. 

Just grab the apropriate binary from there and this engine is ready 
to go.
