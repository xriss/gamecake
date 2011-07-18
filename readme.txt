Welcome to my defenestration attempt to remove my code from the shackles of windows, hence building under ubuntu is the default.

All c/c++ code is packaged up inside lua libraries.

So this dir contains lua modules and premake build scripts for them either my own or snapshots of public libraries possibly with custom wrappers.

Unless noted otherwise the snapshots of public libraries will be unmodified except to contain a premake4.lua build control file in their directory.

the bin dir is a testbed containing pre-built binaries and lua scripts and data.

The following commands should build everything if run on a new checkout under ubuntu assuming you have a build environment setup, you will need to install some dev packages such as mysql/opengl/gtk.

apt.sh may manage to do this for you then

build/premake4 gmake
cd build-gmake
make config=release

for windows use the following and then proceed to wrangle the project created in build-vs2008

build/premake4.exe vs2008

Premake does of course suport other build tools as output targets but these are the ones I use and test.

-- 

Kriss@WetGenes.com
