

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


local p=assert(wv4l2.open("/dev/video0"))
print(wstr.dump(wv4l2.capture_list(p)))
wv4l2.capture_start(p,{width=640,height=480,buffer_count=2,format="UYVY"})
print(wstr.dump(wv4l2.info(p)))

local g
local gs={}
local miss=0
while #gs<20 do
	local t=wv4l2.capture_read_grd(p,g and g[0]) -- reuse last grd
	if t then
		g=g or wgrd.create(t)
		gs[#gs+1]=true
		print(os.time(),miss,t)
		miss=0
--		g:convert("FMT_U8_RGBA")
--		g:save("test"..#gs..".png")
	else
		miss=miss+1
	end
end
--print(wstr.dump(gs))

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
