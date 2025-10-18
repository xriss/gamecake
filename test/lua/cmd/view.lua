
local tardis=require("wetgenes.tardis")
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

-- fake bake
local views=require("wetgenes.gamecake.views").bake({},{})

local hx=1920
local hy=1080

local view=views.create({
	px=0,py=0,
	hx=hx,hy=hy,
	vx=hx,vy=hy,
})

local v=V4(0,0,0,1)*view.cmtx*view.pmtx
print(v*V4(hx/2,hy/2,0,0))

local v=V4(hx/2,hy/2,0,1)*view.cmtx*view.pmtx
print(v*V4(hx/2,hy/2,0,0))

local v=V4(hx,hy,0,1)*view.cmtx*view.pmtx
print(v*V4(hx/2,hy/2,0,0))
