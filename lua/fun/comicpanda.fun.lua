-- 
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it. 
-- 

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

-- gonna render some sounds
local djon=require("djon")
local bitsynth=require("wetgenes.gamecake.fun.bitsynth")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wstr=require("wetgenes.string")

oven.opts.fun="" -- back to menu on reset				

sysopts={
	-- these do not read the global main_all until called so it can be set later.
	update=function() main_all:update() end, -- call global update
	draw=function() main_all:draw() end, -- call global draw

	mode="swordstone", -- select basic text setup using the swanky32 palette.
	hx=128,hy=192, -- fixed size
	layers=4,
	hardware={ -- modify hardware so we have sprites and move the text layer
		{
			component="copper",
			name="copper",
			layer=1,
		},
		{
			component="tilemap",
			name="map",
			tiles="tiles",
			tile_size={8,8},
			layer=2,
		},
		{
			component="sprites",
			name="sprites",
			tiles="tiles",
			layer=3,
		},
		{
			component="tilemap",
			name="text", -- will replace the old text
			tiles="tiles",
			tile_size={4,8},
			layer=4,
		},
	},
	icon=[[
. 0 0 0 . . . . . . . . 0 0 0 . 
0 0 2 0 0 . 6 6 6 6 6 0 0 2 0 0 
0 3 2 0 6 6 6 6 6 6 6 6 0 2 1 0 
0 3 0 6 6 6 6 6 6 6 6 6 6 0 1 0 
0 0 6 6 0 0 0 6 6 6 0 0 0 6 0 0 
0 0 6 0 0 6 0 0 6 0 0 6 0 0 6 0 
. 6 6 0 0 0 0 6 6 6 0 0 0 0 6 . 
. 6 6 6 0 0 6 6 6 6 6 0 0 6 6 . 
. 6 6 6 6 6 6 0 0 0 6 6 6 6 6 . 
. 6 6 6 6 6 6 6 0 6 6 6 6 6 6 . 
. . 6 6 6 6 6 6 6 6 6 6 6 6 . . 
. . 6 6 0 6 6 0 0 0 6 6 0 6 . . 
. . . 6 6 0 0 6 6 6 0 0 6 . . . 
. . . . 6 6 6 6 6 6 6 6 . . . . 
. . . . . . 6 6 6 6 6 . . . . . 
. . . . . . . . . . . . . . . . 
]],
}


--------------------------------------------------------------------------------
--
--#draws

draws={}
draws.sprite=function(it) -- note that we will modify this table
	local spr=system.components.tiles.names[it.n] -- sprite by name
	if it.i then spr=spr.cuts[it.i] end -- and cuts idx
	it.t=spr.idx
	if it.p then
		it.px=it.p[1]
		it.py=it.p[2]
		it.pz=it.p[3]
	end
--[[
	if not it.pz then it.pz=it.px+it.py end	-- auto pz
	local map=system.components.map
	it.px=it.px+map.window_px-map.px	-- auto map position
	it.py=it.py+map.window_py-map.py
]]
	if not it.hx then it.hx=spr.hx end -- auto size
	if not it.hy then it.hy=spr.hy end
	it.ox=(it.ox or 0)+(spr.hx)/2 -- auto center handle
	it.oy=(it.oy or 0)+(spr.hy)/2
	it.px=math.floor(it.px+0.5)
	it.py=math.floor(it.py+0.5)
	if it.rz then
		it.rz=360*it.rz
	end
	system.components.sprites.list_add(it)

end


--------------------------------------------------------------------------------
--#all
-- simple scene setup

all={}
all.meta={__index=all,is="all"}

all.sys={}

all.create=function(it)
	return setmetatable( it or {} , all.meta )
end

all.list_add=function(all,it)
	all.list[#all.list+1]=it
end

all.setup=function(all)
    all.setup_done=true

	all.list={} -- simple list of objects

	text.create() -- add an object
	panda.create() -- add an object
	
	for idx,it in ipairs(all.list) do it:setup() end

	for _,sys in pairs(all.sys) do
		if sys.graphics then
			system.components.tiles.upload_tiles( sys.graphics )
		end
	end

end

all.update=function(all)
	if not all.setup_done then all:setup() end

	local delete_me
	for idx,it in ipairs(all.list) do
		it:update()
		if it.delete_me then delete_me=true end
	end

	if delete_me then
		for idx=#all.list,1,-1 do
			if all.list[idx].delete_me then
				table.remove(all.list,idx):delete_me()
			end
		end
	end
end

all.draw=function(all)

	for idx,it in ipairs(all.list) do it:draw() end
end


--------------------------------------------------------------------------------
--#panda
-- manage panda

panda={}
panda.meta={__index=panda,is="panda"}

all.sys.panda=panda

panda.create=function(it)
	it=setmetatable( it or {} , panda.meta )
	main_all:list_add(it)
	return it
end

panda.setup=function(panda)

	panda.dir=1
	panda.pos=V3(32,164,0)
	panda.frame=0
	panda.walk_frame=1

end

panda.update=function(panda)

	panda.frame=panda.frame+1
	if panda.frame>=6 then
		panda.frame=0
		panda.walk_frame=panda.walk_frame+1
		if panda.walk_frame>4 then panda.walk_frame=1 end
		panda.pos[1]=panda.pos[1]+panda.dir
		if panda.pos[1]>108 then
			panda.dir=-1
		end
		if panda.pos[1]<20 then
			panda.dir=1
		end
	end

end

panda.draw=function(panda)

	draws.sprite({p=panda.pos,n="panda_walk",i=panda.walk_frame,sx=panda.dir})

end

panda.graphics={

{nil,"panda_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . 
. . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . 
. . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 0 0 6 6 6 0 0 6 0 0 6 0 0 6 0 0 6 0 . . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . 
. . 0 0 0 6 6 6 0 0 6 0 0 6 0 0 6 0 0 6 0 . . . . . . 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . 0 0 0 6 6 6 0 0 6 0 0 6 0 0 6 0 0 6 0 . . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . 
. . . 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . . . 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . . 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . 0 0 0 6 6 6 0 0 6 0 0 6 0 0 6 0 0 6 0 . . . 
. . . . 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . . 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . 
. . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . 
. . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . 
. . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . 
. . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . . . . . . . 6 6 6 6 0 0 6 6 6 0 0 6 6 . . . . . . . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . 
. . . . . . 6 6 6 6 0 0 6 6 6 0 0 6 6 . . . . . . . . . . . 0 0 6 6 6 6 6 6 6 6 6 0 . . . . . . . . 0 0 0 0 6 6 6 6 0 0 6 6 6 0 0 6 6 0 0 0 . . . . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . 
. . . . . . 0 0 6 6 6 6 6 6 6 6 6 0 0 . . . . . . . 0 0 0 0 0 0 0 0 6 6 6 6 6 1 0 0 0 0 0 0 . . . 0 1 2 1 0 0 0 6 6 6 6 6 6 6 6 6 0 0 1 2 1 0 . . . 0 0 0 0 6 6 6 6 0 0 6 6 6 0 0 6 6 0 0 0 . . 
. . 0 0 0 0 0 0 0 0 6 6 6 6 6 1 0 0 0 0 0 0 . . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 6 6 6 6 6 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 6 6 6 6 6 6 6 6 6 0 0 1 2 1 0 . 
. 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 6 6 6 6 6 1 0 0 0 2 3 2 0 . 
. 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . . 0 0 0 0 0 0 0 0 1 2 0 0 2 1 0 0 0 0 0 0 . . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . 
. 0 1 2 1 0 0 0 0 0 6 6 6 6 2 1 0 0 0 1 2 1 0 . . . 0 0 0 0 . . . 0 1 6 6 6 2 1 0 . 0 0 0 0 . . . . . . . . 0 0 0 0 1 6 6 6 6 1 0 0 0 . . . . . . . 0 0 0 0 0 0 0 0 1 2 0 0 2 1 0 0 0 0 0 0 . . 
. . 0 0 0 0 . . 0 6 6 6 6 6 6 1 0 . 0 0 0 0 . . . . . . . . . . . 0 6 6 6 6 6 1 0 . . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 0 . . . . . . . . . . . . . 0 0 0 0 6 6 6 6 6 1 0 0 0 . . . . . 
. . . . . . . 0 6 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 6 0 . . . . . . . 
. . . . . . 0 0 0 0 0 0 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 6 6 6 0 0 0 6 6 6 0 . . . . . . 
. . . . . . 0 1 3 2 0 0 . 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 . 0 1 3 2 0 . . . . . . . . . . . 0 0 0 0 0 0 0 . 0 0 0 0 0 0 . . . . . 
. . . . . . 0 0 0 0 0 . . 0 1 3 2 0 . . . . . . . . . . . . . 0 1 3 2 0 0 1 3 2 0 . . . . . . . . . . . . . 0 1 3 2 0 . . 0 0 0 0 0 . . . . . . . . . . . 0 1 3 2 0 . . . . 0 1 3 2 0 . . . . . 
. . . . . . . . . . . . . 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 . . . . . . . . . . . . . . . . . . 0 0 0 0 0 . . . . 0 0 0 0 0 . . . . . 
]],4},

}

--------------------------------------------------------------------------------
--#text
-- manage text

text={}
text.meta={__index=text,is="text"}

all.sys.text=text

text.create=function(it)
	it=setmetatable( it or {} , text.meta )
	main_all:list_add(it)
	return it
end

text.setup=function(text)

    local cmap=system.components.map    
    cmap.text_clear(0x09000000) -- clear text forcing a background color

	-- reset tiles
    local ctiles=system.components.tiles
	ctiles.reset_tiles()
	ctiles.upload_default_font_4x8()
	ctiles.upload_default_font_8x8()
	ctiles.upload_default_font_8x16()

end

text.update=function(text)

end

text.draw=function(text)

    local ctext=system.components.text
	ctext.text_print(" PanDaa Says What               ",0,0,26,24)
    for y=1,22 do
		ctext.text_print("  ",0,y,26,24)
		ctext.text_print("  ",30,y,26,24)
    end
	ctext.text_print("                       4lfa.com ",0,23,26,24)

end

--------------------------------------------------------------------------------
--#start

hardware,main=system.configurator(sysopts)
main_all=all.create()

-- we are in a sandbox and global has probably already been required
-- so we need to force lock globals like so
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
