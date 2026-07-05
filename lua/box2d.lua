--
-- (C) 2026 Kriss@XIXs.com
--

--[[#lua.box2d

	local box2d=require("box2d")

We use box2d as the local name of this library.

A lua binding to the box2d physics library

]]

--module
local M={ modname=(...) }
package.loaded[M.modname]=M
local box2d=M

local core=require("box2d.core")

local boxxd=require("boxxd")

-- meta methods bound to the various objects

box2d.space_functions={is="space"}
box2d.space_metatable={__index=box2d.space_functions}

box2d.body_functions={is="body"}
box2d.body_metatable={__index=box2d.body_functions}

box2d.shape_functions={is="shape"}
box2d.shape_metatable={__index=box2d.shape_functions}

box2d.arbiter_functions={is="arbiter"}
box2d.arbiter_metatable={__index=box2d.arbiter_functions}

box2d.constraint_functions={is="constraint"}
box2d.constraint_metatable={__index=box2d.constraint_functions}


box2d.version=core.version
