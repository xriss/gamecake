
OSX running inside a Vagrant VitualBox
--------------------------------------

Vagrant ssh access to this OSX box is working again so we can build 
gamecake using clang. Note that SDL2 and LuaJIT are dylibs so I'm not 
building a single file on OSX, which makes me sad but static linking 
would require me building them myself. The make script ( 
https://github.com/xriss/gamecake/blob/master/vbox_osx/make ) mucks 
around with the linker settings so they are loaded from an osx 
directory next to the gamecake.osx file. You will also find a steam 
dylib in there but that is optional unlike these two.

So you would need to take gamecake.osx and the osx directory from 
https://github.com/xriss/gamecake/tree/exe

This is using a rather large (14gb) osx-10.14 base which is the oldest 
version I couild get working. My real test hardware (mac mini 2011) 
will only go upto 10.13 but the binaries built here work OK on it so I 
guess they will work with future versions too?

See Vagrantfile ( 
https://github.com/xriss/gamecake/blob/master/vbox_osx/Vagrantfile ) 
for special hardware hacks to make virtualbox not fallover when booting 
osx. This also works on AMD hardware, which is why it has to lie about 
the cpu.

All of this was arrived at by tweaking and testing so its not canon but 
it works for me and maybe you.

See provision.sh ( 
https://github.com/xriss/gamecake/blob/master/vbox_osx/provision.sh ) 
for things we do to the system to make it more managable. The important 
bits are getting brew uptodate so we can install stuff and then getting 
a sane version of bash (osx is stuck in the dark ages for reasons) so 
we can run build scripts.

Finally I also git clone/update my gamecake code repo as we can not 
mount a directory from the host machine so I have to pull changes via 
github rather than from the local machine.

So if you want to use this for something else you would want to scrub 
that last part.


VAGRANT HAS FUCKED SOMETHING

provisioning fails, but vagrant ssh still works, so have added vagrant 
as the fallback password in the Vagrantfile and need to do the same on 
the box like so in a live fix before this can work.

	vagrant up

wait for it to fail

	vagrant ssh

then on the box

	sudo bash
	passwd vagrant
	vagrant
	vagrant
	exit
	exit


Now you can edit the Vagrantfile to enable the vagrant password and we are 
back to where we should have started.

	nano Vagrantfile


and finally halt the machine to cleanup

	vagrant halt


Now you can unset the password in the vagrantfile, so that vagrant ssh 
doesnt expect a password. IE undo what you just tweaked

	nano Vagrantfile


All this will have set the password to vagrant for the vagrant user and let 
vagrant unfuck itself and so that finally we can just use it like normal

	vagrant up
	vagrant provision

To do the provisioning that previously failed (note updating brew takes 
forever)

