-- This file exists to flag the basedir of the INSERTFILENAMEHERE app
-- it may also contain some lua code that should be run



local lunatest=require("lunatest")

module(...)

function start()

	lunatest.suite("poop")
	lunatest.suite("test")

	lunatest.run()
	
end
