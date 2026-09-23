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
0 3 2 0 6 6 6 6 6 6 6 6 0 2 3 0 
0 3 0 6 6 6 6 6 6 6 6 6 6 0 3 0 
0 0 6 6 0 0 0 6 6 6 0 0 0 6 0 0 
0 0 6 0 0 7 0 0 6 0 0 7 0 0 6 0 
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

draws.char4=function(c,x,y,z)
	if not z then z=1 end
	local cx,cy=system.components.text.text_tile4x8(c)
	system.components.sprites.list_add({
		t=cy*256+(cx/2) ,
		hx=4 , hy=8 ,
		ox=0 , oy=0 ,
		px=x , py=y , pz=-1 ,
		sx=z,sy=z,
		color=0xffffffff,
	})
end

draws.string4=function(s,x,y,z)
	if not z then z=1 end
	x=x-#s*2*z
	for i=1,#s do
		local c=s:sub(i,i)
		draws.char4(c,x+(i-1)*4*z,y,z)
	end
end

draws.char16=function(c,x,y,z)
	if not z then z=1 end
	local cx,cy=system.components.text.text_tile8x16(c)
	system.components.sprites.list_add({
		t=cy*256+cx ,
		hx=8 , hy=16 ,
		ox=0 , oy=0 ,
		px=x , py=y , pz=-1 ,
		sx=z,sy=z,
		color=0xffffffff,
	})
end

draws.string16=function(s,x,y,z)
	if not z then z=1 end
	x=x-#s*4*z
	for i=1,#s do
		local c=s:sub(i,i)
		draws.char16(c,x+(i-1)*8*z,y,z)
	end
end

draws.charS=function(c,x,y,z)
	if not z then z=4 end
	if z>6 then
		draws.char16(c,x,y,z/8)
	else
		draws.char4(c,x,y,z/4)
	end
end

draws.stringS=function(s,x,y,z)
	if not z then z=4 end
	if z>6 then
		draws.string16(s,x,y,z/8)
	else
		draws.string4(s,x,y,z/4)
	end
end

--------------------------------------------------------------------------------
--#all
-- simple scene setup

-- all is everything
local all={}
all.is="all"
all.__index=all

all.class_meta={}
all.class=function(all,name,...)
	if all.class_meta[name] then return all.class_meta[name] end
	local it={}
	all.class_meta[name]=it
--	it.__index=it
	it.is=name
	it.is_also={...}
	return it
end


all.create=function(all,it)
	return setmetatable( it or {} , all )
end

all.order_sort=function(all,orderby)
	if orderby then -- optionally change order
		for n,v in pairs(orderby) do
			all.orderby[n]=v
		end
	end

	all.order={} -- reset
	for n,v in pairs(all.class_meta) do -- fill with names
		all.order[#all.order+1]=n
	end

	table.sort(all.order,function(a,b)
		local aw=all.orderby[a] or 0
		local bw=all.orderby[b] or 0
		if aw<bw then -- sort by weight
			return true
		elseif aw>bw then
			return false
		else -- then sort by name
			if a<b then
				return true
			else
				return false
			end
		end
	end)
end

all.lists_pairs=function(all)
	local idx=0
	return function()
		idx=idx+1
		local name=all.order[idx]
		return name,all.lists[ name ]
	end
end

all.class_manifest=function(all,name)
	if all.classes[name] then return all.classes[name] end

	local it={}
	local its={}
	all.lists[name]=its
	all.classes[name]=it -- meta prototype

	-- perform inheritance
	local fill_it=function(from)
		for n,v in pairs( from ) do
			if type(it[n])=="nil" then
				it[n]=v
			end
		end
	end
	fill_it( assert(all.class_meta[name]) ) -- must exist
	for i,n in ipairs(it.is_also) do
		fill_it( all:class_manifest(n) ) -- fix call order with recursion
	end
	
	-- bind live values for quick access
	it.__index=it
	it.all=all
	it.class=it
	
	return it
end

all.singleton=function(all,name)
	local list=all.lists[name]
	return list[#list]
end

all.setup=function(all)
	if all.setup_done then return end
    all.setup_done=true
    PRINT("SETUP")

--    system.components.screen.bloom=0
--    system.components.screen.filter=nil

	all.order={} -- order list of names
	all.orderby={} -- order weights map or default to 0
	all.lists={} -- map of name to items list
	all.classes={} -- meta proto table for each class


	for name,it in pairs(all.class_meta) do
		all:class_manifest(it.is)
	end
	all:order_sort() -- this creates all.order
    
	-- reset tiles
    local ctiles=system.components.tiles
	ctiles.reset_tiles()

 	-- and upload all the tiles we are going to use, first all the fonts
 	ctiles.upload_default_font_4x8()
	ctiles.upload_default_font_8x8()
	ctiles.upload_default_font_8x16()

	for _,class in pairs(all.classes) do
		if class.graphics then
			class.tiles_sprites={}
			for idx,v in ipairs( class.graphics ) do
				local t={}
				t.idx=v[1]
				t.name=v[2]
				t.ascii=v[3]
				t.cuts=v[4]
				class.tiles_sprites[idx]=ctiles.upload_tile( t )
			end
		end
		if class.graphics_maps then
			class.tiles_maps={}
			for idx,v in ipairs( class.graphics_maps.bmaps ) do
				local t={}
				t.ascii=v.bmap
				class.tiles_maps[idx]=ctiles.upload_tile( t )
			end
		end
	end

	local panda={}


	panda.text=([[
	
	The name Poopee Pandaa has been generating unwanted attention from a
	certain group of perverts.
	
	To de-escalate this situation you may also refer to me by my old school
	nickname.
	
	DOBERMAN UNCUT
	
	So called because of my very large floppy ears and loveable nature.
	
]]):match("^%s*(.-)%s*$")
	panda.title="Hello Pandaa"



	panda.text=([[
	
	You call yourself a traditionalist and yet you refuse to crawl into the
	giant wicker man?
		
	Not only would your sacrifice guarantee the harvest but it's also a great
	day out for the kids.
	
	I am beginning to suspect that you might be picking and choosing
	"acceptable" traditions.
	
]]):match("^%s*(.-)%s*$")
	panda.title="Of Your Own Free Will"



	panda.text=([[
	
	When are flat earthers going to grow up and realise the TRUTHINESS?
	
	We have to live on the outside of a sphere!
	
	It drastically reduces the draw distance, lowering the rendering cost and
	enabling you to exist.
	
	Would you rather be an NPC?
	
]]):match("^%s*(.-)%s*$")
	panda.title="Simple Common Sense"



	panda.text_idx=1
	panda.text_wait=0

	all.classes.back:create():setup() -- add an object
	all.classes.text:create():setup() -- add an object
	all.classes.panda:create(panda):setup() -- add an object
	
end

all.update=function(all)
	if not all.setup_done then all:setup() end

	for _,list in all:lists_pairs() do
		for idx=#list,1,-1 do -- backwards so safe to remove or add
			list[idx]:update()
		end
	end
end

all.draw=function(all)

	for _,list in all:lists_pairs() do
		for idx=#list,1,-1 do -- backwards so safe to remove or add
			list[idx]:draw()
		end
	end
end


--------------------------------------------------------------------------------
--#item
-- manage item

local item=all:class("item")

item.create=function(item,it)
	local all=item.all

	it=setmetatable( it or {} , all.classes[ item.is ] )

	local list=all.lists[item.is]
	list[#list+1]=it

	return it
end




--------------------------------------------------------------------------------
--#panda
-- manage panda

local panda=all:class("panda","item")

panda.setup=function(panda)

	panda.dir=1
	panda.pos=V3(28,162,0)
	panda.frame=0
	panda.walk_frame=1
	panda.text_pos=V3(13,12)
	panda.mouth=1

end

panda.update=function(panda)

	panda.frame=panda.frame+1
	if panda.frame>=6 then
		panda.frame=0
		panda.walk_frame=panda.walk_frame+1
		if panda.walk_frame>4 then panda.walk_frame=1 end
		panda.pos[1]=panda.pos[1]+panda.dir
		if panda.pos[1]>100 then
			panda.dir=-1
		end
		if panda.pos[1]<28 then
			panda.dir=1
		end
	end

	local talk=panda.all.classes.talk

	panda.text_wait=panda.text_wait-1
	if (panda.text_wait<=0) and (#panda.text>panda.text_idx) then
		panda.text_wait=60
		local idx=panda.text_idx
		if panda.text:match("^%s",idx) then -- whitespace
			local word=panda.text:match("^%s*",idx)
			panda.text_idx=idx+#word
			panda.text_wait=4
			panda.text_pos[1]=panda.text_pos[1]+4
			if word:match("\n%s*\n") then
				panda.text_pos[1]=13
				panda.text_pos[2]=panda.text_pos[2]+16
				panda.text_wait=90
			end
		else
			local word=panda.text:match("^%S*",idx)
			panda.text_idx=idx+#word
			panda.text_wait=2*#word

			if panda.text_pos[1]+#word*4 > 128-10 then -- wrap
				panda.text_pos[1]=13
				panda.text_pos[2]=panda.text_pos[2]+8
			end

			local ww={
				text=word,
				pos_from=panda.pos+V3(0,-16),
				pos_goal=V3(panda.text_pos),
				age_max=0,
			}
			ww.age_max=30+math.ceil(math.abs(ww.pos_from[2]-ww.pos_goal[2])/1)
--			ww.pos_from[1]=ww.pos_from[1]-#word*2
			ww.pos_goal[1]=ww.pos_goal[1]+#word*2
			talk:create_word(ww)
			
			panda.mouth=panda.mouth+1
			
			panda.text_pos[1]=panda.text_pos[1]+#word*4
		end
	end
	panda.mouth=math.min(math.max( panda.mouth-(1/8) ,1),4)


end

panda.draw=function(panda)

	local b=({0,-1,0,1})[panda.walk_frame]
	local bob=V3(0,b,0)
	local mouth=math.ceil(math.min( panda.mouth ,3))
	draws.sprite({p=panda.pos+bob,n="panda_head",i=mouth,sx=panda.dir})
	draws.sprite({p=panda.pos+V3(0,8,0),n="panda_walk",i=panda.walk_frame,sx=panda.dir})

end

panda.graphics={

{nil,"panda_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . 0 1 2 0 0 2 1 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 . . . 0 1 2 0 0 2 1 . . 0 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . 0 0 0 0 1 2 0 0 2 1 0 0 0 . . . . . . . 0 0 0 0 . . . 0 1 2 0 0 2 1 . . 0 0 0 0 . . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . . 0 0 0 0 . . . 0 1 2 0 0 2 1 . . 0 0 0 0 . . 
. . 0 0 0 0 0 0 0 0 1 2 0 0 2 1 0 0 0 0 0 0 . . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . 
. 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . 
. 0 2 3 2 0 0 0 0 0 1 2 0 0 2 1 0 0 0 2 3 2 0 . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . . . 0 0 0 0 0 0 0 0 1 2 0 0 2 1 0 0 0 0 0 0 . . . 0 1 2 1 0 0 0 0 0 1 2 0 0 2 1 0 0 0 1 2 1 0 . 
. 0 1 2 1 0 0 0 0 0 6 6 6 6 2 1 0 0 0 1 2 1 0 . . . 0 0 0 0 . . . 0 1 6 6 6 2 1 0 . 0 0 0 0 . . . . . . . . 0 0 0 0 1 6 6 6 6 1 0 0 0 . . . . . . . 0 0 0 0 0 0 0 0 1 2 0 0 2 1 0 0 0 0 0 0 . . 
. . 0 0 0 0 . . 0 6 6 6 6 6 6 1 0 . 0 0 0 0 . . . . . . . . . . . 0 6 6 6 6 6 1 0 . . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 0 . . . . . . . . . . . . . 0 0 0 0 6 6 6 6 6 1 0 0 0 . . . . . 
. . . . . . . 0 6 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 6 6 6 6 0 . . . . . . . 
. . . . . . 0 0 0 0 0 0 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 6 0 6 6 6 0 . . . . . . . . . . . . . . . 0 6 6 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 6 6 6 0 0 0 6 6 6 0 . . . . . . 
. . . . . . 0 1 3 2 0 0 . 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 . 0 1 3 2 0 . . . . . . . . . . . 0 0 0 0 0 0 0 . 0 0 0 0 0 0 . . . . . 
. . . . . . 0 0 0 0 0 . . 0 1 3 2 0 . . . . . . . . . . . . . 0 1 3 2 0 0 1 3 2 0 . . . . . . . . . . . . . 0 1 3 2 0 . . 0 0 0 0 0 . . . . . . . . . . . 0 1 3 2 0 . . . . 0 1 3 2 0 . . . . . 
. . . . . . . . . . . . . 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 0 0 0 0 0 . . . . . . . . . . . . . 0 0 0 0 0 . . . . . . . . . . . . . . . . . . 0 0 0 0 0 . . . . 0 0 0 0 0 . . . . . 
]],4},

{nil,"panda_head",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . . 0 0 0 0 0 . . 6 6 6 6 6 . 0 0 0 0 0 . . . 
. . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 1 3 2 0 0 6 6 6 6 6 6 6 6 6 0 2 3 1 0 . . 
. . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 3 1 0 0 6 6 6 6 6 6 6 6 6 6 6 0 1 3 0 . . 
. . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . . . 0 0 0 6 6 6 0 0 7 0 0 6 0 0 7 0 0 6 0 0 . . . . 0 2 0 0 6 6 6 0 0 0 6 6 6 0 0 0 6 0 2 0 . . 
. . 0 0 0 6 6 6 0 0 7 0 0 6 0 0 7 0 0 6 0 . . . . . 0 0 0 6 6 6 0 0 7 0 0 6 0 0 7 0 0 6 0 0 . . . . 0 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . 0 0 0 6 6 6 0 0 7 0 0 6 0 0 7 0 0 6 0 0 . . 
. . . 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . 0 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . . . . 0 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . 0 0 0 6 6 6 0 0 0 0 6 6 6 0 0 0 0 6 0 . . . 
. . . . 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . . 0 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . 0 6 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 . . . 
. . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 . . . 
. . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 . . . 
. . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . . . 6 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 6 . . . . . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . 
. . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . . . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . . . . . . 6 6 6 6 0 0 0 0 0 0 0 0 0 6 6 . . . . . . . . . 6 6 6 6 0 6 6 0 0 0 6 6 0 6 6 . . . . 
. . . . . . 6 6 6 6 0 0 6 6 6 0 0 6 6 . . . . . . . . . . 6 6 6 6 0 0 0 0 0 0 0 0 0 6 6 . . . . . . . . . 6 6 6 6 6 0 0 0 0 0 0 0 6 6 6 . . . . . . . . . 6 6 6 6 0 0 0 0 0 0 0 0 0 6 6 . . . . 
. . . . . . . . 6 6 6 6 6 6 6 6 6 . . . . . . . . . . . . . 6 6 6 6 0 0 6 6 6 0 0 6 6 . . . . . . . . . . . 6 6 6 6 6 0 0 0 0 0 6 6 6 . . . . . . . . . . . 6 6 6 6 0 0 6 6 6 0 0 6 6 . . . . . 
. . . . . . . . . . 6 6 6 6 6 . . . . . . . . . . . . . . . . . 6 6 6 6 6 6 6 6 6 . . . . . . . . . . . . . . . 6 6 6 6 6 6 6 6 6 . . . . . . . . . . . . . . . 6 6 6 6 6 6 6 6 6 . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6 6 6 6 6 . . . . . . . . . . . . . . . . . . . 6 6 6 6 6 . . . . . . . . . . . . . . . . . . . 6 6 6 6 6 . . . . . . . . . 
]],4},

}

--------------------------------------------------------------------------------
--#text
-- manage text

local talk=all:class("talk","item")

talk.setup=function(talk)

	talk.siz=1
	
	talk:update()

end

talk.update=function(talk)


	local t=0
	if talk.age>=talk.age_max then
		t=1
	elseif talk.age>=0 then
		t=talk.age/talk.age_max
	end
	local h=1-(math.abs(t-0.5)*2)
	local t2=t^0.8
	local t3=t^0.4
	local h2=h^0.5
	talk.siz=1+h2*2
	
	talk.pos[1]=talk.pos_from[1]*(1-t2)+(talk.pos_goal[1]*t2)
	talk.pos[2]=talk.pos_from[2]*(1-t2)+(talk.pos_goal[2]*t2)

	talk.age=talk.age+1
end

talk.draw=function(talk)

	draws.stringS( talk.word.text , talk.pos[1] , talk.pos[2] , 4*talk.siz )

end

talk.create_word=function(talk,word)

--print(word.text)

--	for idx=1,#word.text do
	
		local it={}
		
		it.word=word
--		it.letter=word.text:sub(idx,idx)
		it.age=0
		it.age_max=word.age_max
		it.pos_from=V3(word.pos_from)
		it.pos_goal=V3(word.pos_goal)
--		it.pos_goal[1]=talk.pos_goal[1]+((idx-1)*4)
--		it.pos_from[1]=talk.pos_from[1]+((idx-1)*8)

		it.pos=V3(it.pos_from)

		talk.all.classes.talk:create(it):setup()

--	end

end

--------------------------------------------------------------------------------
--#text
-- manage text

local text=all:class("text","item")

text.setup=function(text)

end

text.update=function(text)

end

text.draw=function(text)

    local ctext=system.components.text
	ctext.text_print("                                ",0,0,26,24)
	local title=text.all:singleton("panda").title
	ctext.text_print( title ,1,0,26,24)
    for y=1,22 do
		ctext.text_print("  ",0,y,26,24)
		ctext.text_print("  ",30,y,26,24)
    end
	ctext.text_print("                       4lfa.com ",0,23,26,24)

end

--------------------------------------------------------------------------------
--#back
-- manage back

local back=all:class("back","item")

back.setup=function(back)

	local cmap=system.components.map
	cmap.text_clear(0x08000000) -- clear forcing a background color
	
	local tmap=back.graphics_maps.tmaps[1].tmap
	bitdown.tile_grd( tmap, back.tiles_maps, system.components.map.tilemap_grd  ) -- draw into the screen (tiles)
	system.components.map.dirty(true)

end

back.update=function(back)

end

back.draw=function(back)

end

back.graphics_maps={

bmaps={

	{ bmap=[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]], },
	{ bmap=[[
G G y y y y G G 
G y y y y G G G 
y y y y G G G G 
y y y G G G G g 
y y G G G G g g 
y G G G G g g g 
G G G G g g g g 
G G G g g g g G 
]], },
	{ bmap=[[
G G g g g g G G 
G g g g g G G G 
g g g g G G G G 
g g g G G G G y 
g g G G G G y y 
g G G G G y y y 
G G G G y y y y 
G G G y y y y G 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 4 4 4 4 4 
5 5 5 4 4 4 4 4 
5 5 5 4 4 4 4 4 
5 5 5 4 4 4 4 4 
5 5 5 4 4 4 4 4 
5 5 5 4 4 4 4 4 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
4 4 4 4 4 5 5 5 
4 4 4 4 4 5 5 5 
4 4 4 4 4 5 5 5 
4 4 4 4 4 5 5 5 
4 4 4 4 4 5 5 5 
4 4 4 4 4 5 5 5 
]], },
	{ bmap=[[
1 3 3 3 3 3 3 3 
1 2 2 2 2 2 2 2 
1 2 2 2 2 2 2 2 
1 2 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
]], },
	{ bmap=[[
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
]], },
	{ bmap=[[
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
y y G G G G g g 
y G G G G g g g 
G G G G g g g g 
G G G g g g g G 
]], },
	{ bmap=[[
5 5 5 4 4 4 4 4 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 4 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
]], },
	{ bmap=[[
4 4 4 4 4 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 4 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
]], },
	{ bmap=[[
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
4 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 3 2 2 3 5 5 
7 3 0 3 3 0 3 5 
]], },
	{ bmap=[[
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
7 7 7 7 7 7 7 4 
5 5 5 5 5 5 5 7 
5 5 3 2 2 3 5 7 
5 3 0 3 3 0 3 7 
]], },
	{ bmap=[[
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
2 2 2 2 2 2 2 2 
1 g G G G G y y 
1 G G G G y y y 
1 G G G y y y y 
1 G G y y y y G 
]], },
	{ bmap=[[
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
F F F F F F F F 
F F F F F F F F 
F F f f f f f f 
F F f f f f f f 
F F f f F F F F 
]], },
	{ bmap=[[
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
F F F F F F F F 
F F F F F F F F 
f f f f f f F F 
f f f f f f F F 
F F F F f f F F 
]], },
	{ bmap=[[
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 2 2 2 2 2 2 
1 1 G G G G g g 
1 1 G G G g g g 
1 1 G G g g g g 
1 1 G g g g g G 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 4 
5 5 5 5 4 4 4 5 
5 5 5 4 4 4 5 4 
5 5 4 4 4 5 4 4 
5 4 4 4 5 4 4 4 
5 4 4 5 4 4 4 4 
5 4 5 4 4 4 4 4 
]], },
	{ bmap=[[
4 4 4 5 4 4 4 3 
4 4 5 4 4 4 4 3 
4 4 4 4 4 5 4 3 
4 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
4 4 5 4 2 2 4 3 
4 5 4 4 1 1 4 3 
5 4 4 4 1 2 4 3 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 4 
5 5 5 5 4 4 4 5 
5 5 5 4 4 4 5 4 
5 5 4 4 4 5 4 4 
5 4 2 2 5 4 4 4 
5 4 1 1 4 4 4 4 
5 4 2 1 4 4 4 4 
]], },
	{ bmap=[[
4 4 4 5 4 4 4 3 
4 4 5 4 4 4 4 3 
4 4 4 4 4 5 4 3 
4 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
4 4 5 4 4 4 4 3 
4 5 4 4 4 4 4 3 
5 4 4 4 4 4 4 3 
]], },
	{ bmap=[[
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
2 2 2 2 2 2 2 2 
]], },
	{ bmap=[[
7 3 0 3 3 0 3 5 
7 5 3 2 2 3 5 5 
7 5 5 5 5 5 5 7 
7 5 3 2 2 3 5 5 
7 3 0 3 3 0 3 5 
7 3 0 3 3 0 3 5 
7 5 3 2 2 3 5 5 
4 7 7 7 7 7 7 0 
]], },
	{ bmap=[[
5 3 0 3 3 0 3 7 
5 5 3 2 2 3 5 7 
7 5 5 5 5 5 5 7 
5 5 3 2 2 3 5 7 
5 3 0 3 3 0 3 7 
5 3 0 3 3 0 3 7 
5 5 3 2 2 3 5 7 
7 0 7 0 7 0 7 4 
]], },
	{ bmap=[[
1 1 y y y y G G 
1 1 y y y G G G 
1 1 y y G G G G 
1 1 y G G G G g 
1 1 G G G G g g 
1 1 G G G g g g 
1 1 G G g g g g 
1 1 G g g g g G 
]], },
	{ bmap=[[
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f f f f f 
]], },
	{ bmap=[[
F F F F f f F F 
F F F F j j F F 
F F F F f j F F 
F F F F f j F F 
F F F F j j F F 
F F F F f f F F 
F F F F f f F F 
f f f f f f F F 
]], },
	{ bmap=[[
1 1 g g g g G G 
1 1 g g g G G G 
1 1 g g G G G G 
1 1 g G G G G y 
1 1 G G G G y y 
1 1 G G G y y y 
1 1 G G y y y y 
1 1 G y y y y G 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
3 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
]], },
	{ bmap=[[
5 4 4 5 4 4 4 4 
5 4 5 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 5 
5 4 4 4 4 4 5 4 
5 4 4 4 4 5 4 4 
5 4 4 4 5 4 4 4 
]], },
	{ bmap=[[
4 4 4 5 1 2 4 3 
4 4 5 4 1 1 4 3 
4 5 4 4 2 2 4 3 
5 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
]], },
	{ bmap=[[
5 4 2 1 4 4 4 4 
5 4 1 1 4 4 4 4 
5 4 2 2 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 5 
5 4 4 4 4 4 5 4 
5 4 4 4 4 5 4 4 
5 4 4 4 5 4 4 4 
]], },
	{ bmap=[[
4 4 4 5 4 4 4 3 
4 4 5 4 4 4 4 3 
4 5 4 4 4 4 4 3 
5 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
]], },
	{ bmap=[[
F F F F F F F F 
F F F F F F F F 
F F f f f f f f 
F F f f f f f f 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
]], },
	{ bmap=[[
F F F F F F F F 
F F F F F F F F 
f f f f f f F F 
f f f f f f F F 
F F F F j j F F 
F F F F f j F F 
F F F F f j F F 
F F F F j j F F 
]], },
	{ bmap=[[
F F F F F F F F 
F F F F F F F F 
F F f f f f f f 
F F f f f f f f 
F F j j F F F F 
F F j f F F F F 
F F j f F F F F 
F F j j F F F F 
]], },
	{ bmap=[[
F F F F F F F F 
F F F F F F F F 
f f f f f f F F 
f f f f f f F F 
F F F F f f F F 
F F F F f f F F 
F F F F f f F F 
F F F F f f F F 
]], },
	{ bmap=[[
7 7 7 5 7 5 5 7 
7 7 1 1 1 1 1 1 
7 5 1 5 5 7 5 5 
7 7 1 5 7 5 5 5 
7 7 1 5 7 5 5 5 
7 5 1 7 5 5 5 5 
7 5 1 5 5 5 5 4 
7 7 1 5 5 5 4 5 
]], },
	{ bmap=[[
5 5 5 5 4 5 5 4 
1 1 1 1 1 1 4 4 
5 5 4 5 5 1 4 5 
5 4 5 5 4 1 5 5 
5 4 5 5 4 1 5 5 
4 5 5 4 4 1 5 5 
5 5 4 4 5 1 5 4 
5 4 4 5 5 1 4 4 
]], },
	{ bmap=[[
1 1 0 0 0 0 0 0 
1 1 1 1 1 1 1 0 
1 1 0 0 0 0 1 0 
1 1 0 0 0 0 1 0 
1 1 1 1 1 1 1 0 
1 1 0 0 0 0 0 0 
1 1 3 3 3 3 3 3 
1 1 4 4 4 4 4 3 
]], },
	{ bmap=[[
F F f f f f f f 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
F F f f F F F F 
]], },
	{ bmap=[[
f f f f f f F F 
F F F F f f F F 
F F F F f f F F 
F F F F j j F F 
F F F F f j F F 
F F F F f j F F 
F F F F j j F F 
F F F F f f F F 
]], },
	{ bmap=[[
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
]], },
	{ bmap=[[
5 4 4 5 4 4 4 4 
5 4 5 4 4 4 4 4 
5 5 4 4 4 4 4 4 
4 3 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
4 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
]], },
	{ bmap=[[
F F f f F F F F 
F F f f f f f f 
F F f f f f f f 
F F F F F F F F 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
]], },
	{ bmap=[[
F F F F f f F F 
f f f f f f F F 
f f f f f f F F 
F F F F F F F F 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
]], },
	{ bmap=[[
7 5 1 5 5 4 4 4 
7 5 1 1 1 1 1 1 
5 4 4 4 4 4 4 4 
3 3 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
]], },
	{ bmap=[[
4 4 4 4 4 1 4 4 
1 1 1 1 1 1 4 4 
4 4 4 4 4 4 4 4 
3 3 3 3 3 3 3 3 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
]], },
	{ bmap=[[
1 1 3 3 3 3 4 3 
1 1 3 3 3 3 4 3 
1 1 4 4 4 4 4 3 
1 1 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
]], },
	{ bmap=[[
3 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
]], },
	{ bmap=[[
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
3 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
3 4 3 3 3 3 4 3 
3 4 3 3 3 3 4 3 
3 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
]], },
	{ bmap=[[
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
3 4 4 4 4 4 4 3 
3 3 3 3 3 3 3 3 
]], },
	{ bmap=[[
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
3 3 3 3 3 3 3 3 
3 4 4 4 4 4 4 3 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
2 1 1 1 1 2 2 2 
1 1 1 1 2 2 2 2 
1 1 1 2 2 2 2 3 
1 1 2 2 2 2 3 3 
1 2 2 2 2 3 3 3 
2 2 2 2 3 3 3 3 
2 2 2 3 3 3 3 2 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
0 3 3 3 3 2 2 2 
0 3 3 3 2 2 2 2 
0 3 3 2 2 2 2 1 
0 3 2 2 2 2 1 1 
0 2 2 2 2 1 1 1 
0 2 2 2 1 1 1 1 
0 2 2 1 1 1 1 2 
]], },
	{ bmap=[[
3 3 3 3 3 3 3 3 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 1 
3 3 2 2 2 2 1 1 
3 2 2 2 2 1 1 1 
2 2 2 2 1 1 1 1 
2 2 2 1 1 1 1 2 
]], },
	{ bmap=[[
2 2 3 3 3 3 2 2 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 1 
y y G G G G g g 
y G G G G g g g 
G G G G g g g g 
G G G g g g g G 
]], },
	{ bmap=[[
0 2 1 1 1 1 2 2 
0 1 1 1 1 2 2 2 
0 1 1 1 2 2 2 2 
0 1 1 2 2 2 2 3 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 3 3 3 3 2 2 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 1 1 1 1 2 2 
2 1 1 1 1 2 2 2 
1 1 1 1 2 2 2 2 
1 1 1 2 2 2 2 3 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 3 3 3 3 2 2 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 2 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 
]], },
	{ bmap=[[
2 2 1 1 1 1 2 2 
2 1 1 1 1 2 2 2 
1 1 1 1 2 2 2 2 
1 1 1 2 2 2 2 3 
4 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 3 2 2 3 5 5 
7 3 0 3 3 0 3 5 
]], },
	{ bmap=[[
2 2 3 3 3 3 2 2 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 1 
7 7 7 7 7 7 7 4 
5 5 5 5 5 5 5 7 
5 5 3 2 2 3 5 7 
5 3 0 3 3 0 3 7 
]], },
	{ bmap=[[
2 2 1 1 1 1 2 2 
2 1 1 1 1 2 2 2 
1 1 1 1 2 2 2 2 
1 1 1 2 2 2 2 3 
g g G G G G y y 
0 G G G G y y y 
0 G G G y y y y 
0 G G y y y y G 
]], },
	{ bmap=[[
0 2 1 1 1 1 2 2 
0 1 1 1 1 2 2 2 
0 1 1 1 2 2 2 2 
0 1 1 2 2 2 2 3 
0 G G G G G y y 
0 G G G G y y y 
0 G G G y y y y 
0 G G y y y y G 
]], },
	{ bmap=[[
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
0 1 1 1 1 1 1 1 
2 2 2 2 2 2 2 2 
]], },
	{ bmap=[[
0 G y y y y G G 
0 y y y y G G G 
0 y y y G G G G 
0 y y G G G G g 
0 y G G G G g g 
0 G G G G g g g 
0 G G G g g g g 
0 G G g g g g G 
]], },
	{ bmap=[[
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
0 1 1 1 1 1 1 0 
0 0 0 0 0 0 0 0 
0 3 3 3 3 3 3 3 
0 4 4 4 4 4 4 3 
]], },
	{ bmap=[[
0 4 3 3 3 3 4 3 
0 4 3 3 3 3 4 3 
0 4 4 4 4 4 4 3 
0 3 3 3 3 3 3 3 
0 0 0 0 0 0 0 0 
0 1 1 1 1 1 1 0 
0 1 0 0 0 0 1 0 
0 1 0 0 0 0 1 0 
]], },
	{ bmap=[[
G G y y y y G G 
G y y y y G G G 
y y y y G G G G 
y y y G G G G b 
y y G G G G b b 
y G G G G b b b 
G G G G b b b b 
G G G b b b b G 
]], },
	{ bmap=[[
G G b b b b G G 
G b b b b G G G 
b b b b G G G G 
b b b G G G G y 
b b G G G G y y 
b G G G G y y y 
G G G G y y y y 
G G G y y y y G 
]], },
	{ bmap=[[
2 2 3 3 3 3 2 2 
2 3 3 3 3 2 2 2 
3 3 3 3 2 2 2 2 
3 3 3 2 2 2 2 1 
y y G G G G b b 
y G G G G b b b 
G G G G b b b b 
G G G b b b b G 
]], },
	{ bmap=[[
2 2 1 1 1 1 2 2 
2 1 1 1 1 2 2 2 
1 1 1 1 2 2 2 2 
1 1 1 2 2 2 2 3 
b b G G G G y y 
0 G G G G y y y 
0 G G G y y y y 
0 G G y y y y G 
]], },
	{ bmap=[[
0 G y y y y G G 
0 y y y y G G G 
0 y y y G G G G 
0 y y G G G G b 
0 y G G G G b b 
0 G G G G b b b 
0 G G G b b b b 
0 G G b b b b G 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
]], },
	{ bmap=[[
F F F F F F F F 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
f f f f f f f f 
]], },
	{ bmap=[[
5 5 4 4 4 4 4 4 
5 5 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 4 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 4 
4 4 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
]], },
	{ bmap=[[
4 4 4 4 4 4 5 5 
4 4 4 4 4 4 5 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 4 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
]], },
	{ bmap=[[
5 5 5 5 5 5 5 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
]], },
	{ bmap=[[
F F F F F F F F 
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
]], },
	{ bmap=[[
F F F F F F F F 
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
]], },
	{ bmap=[[
5 4 4 4 4 4 4 4 
5 4 4 4 4 4 4 4 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 7 
7 5 5 5 5 5 5 5 
7 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
5 5 5 5 5 5 5 5 
]], },
	{ bmap=[[
4 4 4 4 4 4 4 5 
4 4 4 4 4 4 4 5 
5 5 5 5 5 5 5 5 
7 7 7 7 7 7 7 4 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
5 4 4 4 5 4 4 3 
4 4 4 5 4 4 4 3 
]], },
	{ bmap=[[
F f f f f f f f 
F f f f f f f f 
F f f f f f f f 
F F F F F F F F 
F F F F F F F F 
F F f f f f f f 
F F f f f f f f 
F F f f F F F F 
]], },
	{ bmap=[[
f f f f f f f F 
f f f f f f f F 
f f f f f f f F 
F F F F F F F F 
F F F F F F F F 
f f f f f f F F 
f f f f f f F F 
F F F F f f F F 
]], },

},

tmaps={
	{ tmap=[[
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C
4C4D4C4D4C4D4C4D4C4D4C4D4C4D4C4D
3C5952525A3D3C3E3C3E3C3E3C5B5C3D
4E5D56575E4041424344454F4E5F6047
4D16171819481A1A1A1B1C504D1E1F50
2122232425262728292A2B4A212D2E4A
2F303130313233323334354B2F32334B
37383738373837383738373837383738
393A393A393A393A393A393A393A213B
]],	},

},

}

--------------------------------------------------------------------------------
--#start

hardware,main=system.configurator(sysopts)
main_all=all:create()

-- we are in a sandbox and global has probably already been required
-- so we need to force lock globals like so
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
