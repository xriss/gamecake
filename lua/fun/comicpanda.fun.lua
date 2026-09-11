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

draws.char4=function(s,x,y)
	local sx,sy=system.components.text.text_tile4x8(s)
	system.components.sprites.list_add({
		t=sy*256+(sx/2) ,
		hx=4 , hy=8 ,
		ox=0 , oy=0 ,
		px=x , py=y , pz=-1 ,
		color=0xffffffff,
	})
end

--------------------------------------------------------------------------------
--#all
-- simple scene setup

all={}
all.meta={__index=all}
all.is="all"

all.sys={}

all.create=function(it)
	return setmetatable( it or {} , all.meta )
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
	
	Doberman Uncut
	
	So called because of my very large floppy ears and loveable nature.
	
]]):match("^%s*(.-)%s*$")

	panda.text_idx=1
	panda.text_wait=0

	all.sys.text.create():setup() -- add an object
	all.sys.panda.create(panda):setup() -- add an object
	all.sys.text.create():setup() -- add an object
	
	for idx=#all.list,1,-1 do -- backwards so safe to remove or add
		all.list[idx]:setup()
	end

	for _,sys in pairs(all.sys) do
		if sys.graphics then
			system.components.tiles.upload_tiles( sys.graphics )
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

panda={}
panda.meta={__index=panda}
panda.is="panda"

all.sys.panda=panda

panda.create=function(it)
	it=setmetatable( it or {} , panda.meta )
	main_all:list_add(it)
	return it
end

panda.setup=function(panda)

	panda.dir=1
	panda.pos=V3(28,164,0)
	panda.frame=0
	panda.walk_frame=1
	panda.text_pos=V3(12,16)

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

	local create_word=panda.all.sys.talk.create_word

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
				panda.text_pos[1]=12
				panda.text_pos[2]=panda.text_pos[2]+16
				panda.text_wait=90
			end
		else
			local word=panda.text:match("^%S*",idx)
			panda.text_idx=idx+#word
			panda.text_wait=2*#word

			if panda.text_pos[1]+#word*4 > 128-12 then -- wrap
				panda.text_pos[1]=12
				panda.text_pos[2]=panda.text_pos[2]+8
			end

			create_word({
				text=word,
				pos_from=panda.pos+V3(0,-8),
				pos_goal=V3(panda.text_pos),
				age_max=60,
			})
			
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

talk={}
talk.meta={__index=talk}
talk.is="talk"

all.sys.talk=talk

talk.create=function(it)
	it=setmetatable( it or {} , talk.meta )
	main_all:list_add(it)
	return it
end

talk.setup=function(talk)

end

talk.update=function(talk)


	local t=0
	if talk.age>=talk.age_max then
		t=1
	elseif talk.age>=0 then
		t=talk.age/talk.age_max
	end
	t=t^0.7
	
	talk.pos=talk.pos_from+(t*(talk.pos_goal-talk.pos_from))

	talk.age=talk.age+1
end

talk.draw=function(talk)

	draws.char4( talk.letter , talk.pos[1] , talk.pos[2] )

end

talk.create_word=function(word)

print(word.text)

	for idx=1,#word.text do
	
		local talk={}
		
		talk.word=word
		talk.letter=word.text:sub(idx,idx)
		talk.age=0
		talk.age_max=word.age_max
		talk.pos_from=V3(word.pos_from)
		talk.pos_goal=V3(word.pos_goal)
		talk.pos_goal[1]=talk.pos_goal[1]+((idx-1)*4)

		talk.pos=V3(talk.pos_from)

		all.sys.talk.create(talk):setup()

	end

end

--------------------------------------------------------------------------------
--#text
-- manage text

text={}
text.meta={__index=text}
text.is="text"

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
