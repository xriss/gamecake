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

meta={}
meta.new=function(meta,name,...)
	if meta[name] then return meta[name] end
	meta[name]={}
	meta[name].__index=meta[name]
	meta[name].is=name
	return 	meta[name]
end

local all=meta:new("all")

all.create=function(it)
	return setmetatable( it or {} , all )
end

all.list_add=function(all,it)
	all.list[#all.list+1]=it
	all.names[it.is]=it
	it.all=all -- link back
end

all.setup=function(all)
    all.setup_done=true

	all.list={} -- all objects
	all.names={} -- singleton objects
	
	local panda={}
	panda.text=([[
	
	The name Poopee Pandaa has been generating unwanted attention from a
	certain group of perverts.
	
	To de-escalate this situation you may also refer to me by my old school
	nickname.
	
	DOBERMAN UNCUT
	
	So called because of my very large floppy ears and loveable nature.
	
]]):match("^%s*(.-)%s*$")

	panda.text=([[
	
	You call yourself a traditionalist and yet you refuse to crawl into the
	giant wicker man of your own free will.
	
	Not only would your sacrifice guarantee the harvest but it's also a great
	day out for the kids.
	
	I am beginning to suspect that you might be picking and choosing
	"acceptable" traditions.
	
]]):match("^%s*(.-)%s*$")

	panda.text_idx=1
	panda.text_wait=0

	meta.text.create():setup() -- add an object
	meta.panda.create(panda):setup() -- add an object
	meta.text.create():setup() -- add an object
	
	for idx=#all.list,1,-1 do -- backwards so safe to remove or add
		all.list[idx]:setup()
	end

	for _,it in pairs(meta) do
		if type(it)=="table" then
			if it.graphics then
				system.components.tiles.upload_tiles( it.graphics )
			end
		end
	end

end

all.update=function(all)
	if not all.setup_done then all:setup() end

	for idx=#all.list,1,-1 do -- backwards so safe to remove or add
		all.list[idx]:update()
	end

end

all.draw=function(all)

	for idx=#all.list,1,-1 do -- backwards so safe to remove or add
		all.list[idx]:draw()
	end
end


--------------------------------------------------------------------------------
--#panda
-- manage panda

local panda=meta:new("panda")

panda.create=function(it)
	it=setmetatable( it or {} , panda )
	main_all:list_add(it)
	return it
end

panda.setup=function(panda)

	panda.dir=1
	panda.pos=V3(28,164,0)
	panda.frame=0
	panda.walk_frame=1
	panda.text_pos=V3(13,12)

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

	local create_word=meta.talk.create_word

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
			create_word(ww)
			
			panda.text_pos[1]=panda.text_pos[1]+#word*4
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

local talk=meta:new("talk")

talk.create=function(it)
	it=setmetatable( it or {} , talk )
	main_all:list_add(it)
	return it
end

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

talk.create_word=function(word)

--print(word.text)

--	for idx=1,#word.text do
	
		local talk={}
		
		talk.word=word
--		talk.letter=word.text:sub(idx,idx)
		talk.age=0
		talk.age_max=word.age_max
		talk.pos_from=V3(word.pos_from)
		talk.pos_goal=V3(word.pos_goal)
--		talk.pos_goal[1]=talk.pos_goal[1]+((idx-1)*4)
--		talk.pos_from[1]=talk.pos_from[1]+((idx-1)*8)

		talk.pos=V3(talk.pos_from)

		meta.talk.create(talk):setup()

--	end

end

--------------------------------------------------------------------------------
--#text
-- manage text

local text=meta:new("text")

text.create=function(it)
	it=setmetatable( it or {} , text )
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
--#back
-- manage back

local back=meta:new("back")

back.create=function(it)
	it=setmetatable( it or {} , back )
	main_all:list_add(it)
	return it
end

back.setup=function(back)

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
j j 1 j 1 1 j j 
j j j 1 1 j j i 
i i j j 1 j i j 
j i i 1 j i i i 
j i j j 1 j i j 
i i j 1 j j i i 
j j j 1 1 j j j 
i j 1 1 j 1 j i 
]], },
	{ bmap=[[
j i j j 1 j i j 
i i j 1 j j i i 
j j j 1 1 j j j 
i j 1 1 j 1 j i 
j j 1 j 1 1 j j 
j j j 1 1 j j i 
i i j j 1 j i j 
j i i 1 j i i i 
]], },
	{ bmap=[[
3 4 4 3 0 1 1 0 
3 3 3 3 0 0 0 0 
0 0 0 0 3 3 3 3 
0 1 1 0 3 4 4 3 
0 0 0 0 3 3 3 3 
3 3 3 3 0 0 0 0 
3 4 4 3 0 1 1 0 
3 3 3 3 0 0 0 0 
]], },
	{ bmap=[[
0 0 0 0 3 3 3 3 
0 1 1 0 3 4 4 3 
0 0 0 0 3 3 3 3 
3 3 3 3 0 0 0 0 
3 4 4 3 0 1 1 0 
3 3 3 3 0 0 0 0 
0 0 0 0 3 3 3 3 
0 1 1 0 3 4 4 3 
]], },
	{ bmap=[[
0 0 0 0 3 3 3 3 
3 3 3 3 0 0 0 0 
3 4 4 3 0 1 1 0 
3 3 3 3 0 0 0 0 
0 0 0 0 3 3 3 3 
0 1 1 0 3 4 4 3 
0 0 0 0 3 3 3 3 
3 3 3 3 0 0 0 0 
]], },
	{ bmap=[[
f f f f f f f f 
3 3 3 3 0 0 0 0 
3 4 4 3 0 1 1 0 
3 3 3 3 0 0 0 0 
0 0 0 0 3 3 3 3 
0 1 1 0 3 4 4 3 
0 0 0 0 3 3 3 3 
3 3 3 3 0 0 0 0 
]], },

},

tmaps={
	{ tmap=[[
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
01020102010201020102010201020102
06060606060606060606060606060606
03030303030303030303030303030303
04040404040404040404040404040404
05050505050505050505050505050505
03030303030303030303030303030303
]],	},

},

}

--------------------------------------------------------------------------------
--#start

hardware,main=system.configurator(sysopts)
main_all=all.create()

-- we are in a sandbox and global has probably already been required
-- so we need to force lock globals like so
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
