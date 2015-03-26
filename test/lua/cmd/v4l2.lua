

local M={}

local wstr=require("wetgenes.string")
local sod=require("wetgenes.sod")
local pack=require("wetgenes.pack")
local wgrd=require("wetgenes.grd")


local wv4l2=require("wetgenes.v4l2")


function M.cmd(...)


--print(wv4l2.test())


--[[
print(wv4l2.open())
print(wv4l2.open("/dev/video0"))
print(wv4l2.open("/dev/video1"))
print(wv4l2.open("/dev/video2"))
]]


local p=wv4l2.open("/dev/video1")
print(wstr.dump(wv4l2.capture_list(p)))
wv4l2.capture_start(p,{width=640,height=400,buffer_count=4,format="UYVY"})
print(wstr.dump(wv4l2.info(p)))

local gs={}
for i=1,10000000 do
	local t=wv4l2.capture_read_grd(p)
	if t then
		local g=wgrd.create(t)
		gs[#gs+1]=g
		g:convert("FMT_U8_RGBA")
		g:save("test"..#gs..".png")
	end
end
print(wstr.dump(gs))

wv4l2.capture_stop(p)
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
