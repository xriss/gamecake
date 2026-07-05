--
-- (C) 2026 Kriss@XIXs.com
--

--[[#lua.box3d

	local box3d=require("box3d")

We use box3d as the local name of this library.

A lua binding to the box3d physics library

]]

local box3d={}

local core=require("box3d.core")

-- meta methods bound to the various objects

box3d.space_functions={is="space"}
box3d.space_metatable={__index=box3d.space_functions}

box3d.body_functions={is="body"}
box3d.body_metatable={__index=box3d.body_functions}

box3d.shape_functions={is="shape"}
box3d.shape_metatable={__index=box3d.shape_functions}

box3d.arbiter_functions={is="arbiter"}
box3d.arbiter_metatable={__index=box3d.arbiter_functions}

box3d.constraint_functions={is="constraint"}
box3d.constraint_metatable={__index=box3d.constraint_functions}

