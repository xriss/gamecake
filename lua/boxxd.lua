--
-- (C) 2026 Kriss@XIXs.com
--

--[[#lua.boxxd

	local boxxd=require("boxxd")

We use boxxd as the local name of this library.

Shared code for the box2d / box3d physics library

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local boxxd=M

