

local box2d=require("box2d")

print( "box2d version", box2d.version() )

box2d.meter(16)

local w=box2d.world({},{})
