Welcome to my defenestration attempt to remove my code from the shackles of windows, hence building under ubuntu is the default.

All c/c++ code is packaged up inside lua libraries.

So this dir contains lua modules and premake build scripts for them either my own or snapshots of public libraries possibly with custom wrappers.

Unless noted otherwise the snapshots of public libraries will be unmodified except to contain a premake4.lua build control file in their directory and the occasional build/bug patch.

The bin dir is a testbed containing pre-built binaries and lua scripts and data. This is the final output and is then shared with other, lua only projects or tools.

The following commands should build everything under ubuntu/debian assuming you have a build environment setup, you will need to install some dev packages such as opengl. ./get-apts may install them for you if you are lucky.

./gmake release

for windows I've swiched to a mingwin cross compile. This assumes you have my sdks repo checked out next to the lua one.

./mingw release

There are also some android and nacl build scripts, but these are mostly just tests at the moment and pre alfa.

./android
./nacl

We also have an nginx target which bakes all the lua goodies into nginx. I'm working on moving my aelua web platform code over to use with this but in the mean time it makes for a simple webserver with super bonus powers. It is built automatically alongside the gmake build.

All the above scripts just call premake and premake does of course also suport other build tools as output targets but these are the ones I use and test and you can assume are working.

If you are very very lucky then the linux and windows/wine binaries found under bin will simply work and there wll be no need to build anything. These Binaries actually have all of the lua scripts in bin baked into them so are "standalone" binary blobs. Hopefully they are usefull to just grab and use.

-- 

Kriss@WetGenes.com
