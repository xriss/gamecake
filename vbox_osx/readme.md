
Vagrant ssh access to this OSX box is working again so we can build 
gamecake using clang. Note that SDL2 and LuaJIT are dylibs so I'm 
failing to build a single file on OSX but that was the best I can do. 
The make script here mucks around with the linker settings so they are 
loaded from an osx directory next to the gamecake.osx file. You will 
also find a steam dylib in there but that is optional unlike these two.


Using a rather large (14gb) osx-10.14 base which is the oldest version 
I couild get working. My real test hardware (mac mini 2011) will only 
go upto 10.13 but the binaries built here work OK on it.

See Vagrantfile for special hardware hacks to make virtualbox not 
fallover when booting osx. This also works on AMD hardware.

All of this was arrived at by tweaking and testing so its not canon but 
it works for me and maybe you.

See provision.sh for things we do to the system to make it more 
managable. The important bits are getting brew uptodate so we can 
install stuff and then getting a sane version of bash (osx is stuck in 
the dark ages for reasons) so we can run build scripts.

I also git clone/update my gamecake code repo as we can not mount a 
directory from the host machine so I have to pull changes via github 
rather than from the local machine.

