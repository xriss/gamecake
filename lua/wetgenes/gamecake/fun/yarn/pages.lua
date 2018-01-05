
--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.yarn.pages

	pages = require("wetgenes.gamecake.fun.yarn.pages").create(items)

This module contains only one function which can be used to create an 
pages instance and the rest of this documentation concerns the return 
from this create function, not the module itself.


]]
-----------------------------------------------------------------------------
M.create=function(items)

	pages={} -- a place to store all pages
	
	pages.metatable={} -- unique meta table everytime we create
	pages.metatable.__index=pages.metatable -- metatable is full of functions
	setmetatable(pages.metatable,items.metatable) -- inherit


	pages.page_xh=32 -- size of each page in cells
	pages.page_yh=32

	pages.page_cx=0x8000*pages.page_xh -- centre page location in cells
	pages.page_cy=0x8000*pages.page_yh

	return pages

end
