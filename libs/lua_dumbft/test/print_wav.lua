
phead=[[

SIZE 384 dot,384 dot
CLS
GAP 0,0
BLINE 0,0
OFFSET 0
REFERENCE 0,0
DIRECTION 0,0
BITMAP 0,0,48,384,0,]]

pfoot=[[

PRINT 1
BACKUP 64
]]

local wgrd=require("wetgenes.grd")

local ga=wgrd.create("./wav.png","png")
local gb=wgrd.create(wgrd.FMT_U8_INDEXED,384,384,1)
gb:palette(0,2,{0,0,0,255,128,128,128,255})
ga:scale(384,384,1)
ga:remap(gb,2,2)

--gb:save("test.png")

print(ga.width,ga.height)
local d=gb:pixels(0,0,384,384,"")

local b=0
local a=0
local t={}
for i=1,#d do
	local c=d:sub(i,i)=="\0" and 1 or 0
	a=a*2+c
	b=b+1
	if b==8 then
		b=0
		t[#t+1]=string.char(a)
		a=0
	end
end
local d=table.concat(t)

print(phead..d..pfoot)
