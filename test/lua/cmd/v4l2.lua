

local M={}

local wstr=require("wetgenes.string")
local sod=require("wetgenes.sod")
local pack=require("wetgenes.pack")


local wv4l2=require("wetgenes.v4l2")


function M.cmd(...)

	print(wv4l2.test())

end



return M
