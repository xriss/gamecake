
slightly broken, stuff is getting moved around from multiple hg repos 
to one git repo, check back when it is fixed


Be sure to clone with submodules as the engine binaries live in a 
permanently orphaned branch.

	git clone --recursive https://github.com/xriss/gamecake.git

The same also needs to be done when you pull updates, so remember to 
use the git pull script provided or your bin dir might get out of sync.

	./git-pull


