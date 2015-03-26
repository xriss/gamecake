

local M={}

local wstr=require("wetgenes.string")
local sod=require("wetgenes.sod")
local pack=require("wetgenes.pack")


local wv4l2=require("wetgenes.v4l2")


function M.cmd(...)

--[[
print(wv4l2.open())
print(wv4l2.open("/dev/video0"))
print(wv4l2.open("/dev/video1"))
print(wv4l2.open("/dev/video2"))
]]


local p=wv4l2.open("/dev/video0")
--print(wstr.dump(wv4l2.capture_list(p)))
wv4l2.capture_start(p)--,{width=640,height=480,count=4,format=nil})
wv4l2.close(p)

--[[
local p=wv4l2.open("/dev/video2")
print(wstr.dump(wv4l2.capture_list(p)))
wv4l2.capture_start(p,{})
wv4l2.close(p)
]]


--	print(wv4l2.test())

end



return M
