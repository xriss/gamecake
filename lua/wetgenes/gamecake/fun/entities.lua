
--module had been moved and renamed to 

local deepcopy=require("wetgenes"):export("deepcopy")

local modname=(...)

local zscene=require("wetgenes.gamecake.zone.scene")
local fscene={}

package.loaded[modname]=fscene

fscene.create=zscene.create_fun
