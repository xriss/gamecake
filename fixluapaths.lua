
-- create a global function that can be called to fix lua paths so we can find things
-- unfortunatly you still have to know where this file is first and run a 
-- dofile("thisfile") need to come up with a better plan
--
-- this needs to get more searchy so it can find where the lua stuff is without any explicit values

function fixluapaths(dir,dll)

	dir=dir or "."
	dll=dll or "dll"

	local cpath=	dir .. "?." .. dll ..
				";" .. dir .. "?/init." .. dll .. ";" ..
				package.cpath

	local path=	dir .. "lua/?.lua" ..
				";" .. dir .. "lua/?/init.lua" ..
				";../lua/?.lua" ..
				";../lua/?/init.lua" ..
				";"..package.path


	package.cpath=cpath
	package.path=path

end
