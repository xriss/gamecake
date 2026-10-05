
luarocks and luajit should be installed as we target 5.1 not the silly 
new lua versions.


make sure luarocks is configured to 5.1 ( so luajit will find modules )

	luarocks config lua_version 5.1


to set up lua paths correctly for the current shell you will want to 
also use

	source <(luarocks path)

from then on we should be able to use luajit and it will find installed 
modules


run

	./rock.sh

to do rock packing things and build modules with luarocks



to make and install latest version of wire, this will generate a 
rockspec from env.sh and base.rockspec then generate a src.rock then 
make and install it.

	./rock.sh wire make


To test that everything makes OK, nuke the ~/.luarocks dir and 
reinstall everything.

	rm -rf ~/.luarocks
	luarocks config lua_version 5.1
	./allrocks make


to update published rocks

	./allrocks bump
	./allrocks make

At this point we can test the build using

	./test.sh

But it will probably complain about the pngs/etc being slightly 
different due to the libs we built with, so can only do a visual check.

finally we can upload all the rocks.

	./allrocks upload
