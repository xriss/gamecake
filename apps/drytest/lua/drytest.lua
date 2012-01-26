-- This file exists to flag the basedir of the INSERTFILENAMEHERE app
-- it may also contain some lua code that should be run



local lunatest=require("lunatest")

local apps=apps

local lfs=require("lfs")

module(...)

function start()

	lfs.chdir(apps.dir)

	lunatest.suite("wetgenes_string")
	lunatest.suite("wetgenes_grd")

	lunatest.run()
	
end
