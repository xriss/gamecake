
local wstr=require("wetgenes.string")

local chatdown=require("wetgenes.gamecake.fun.chatdown")	-- conversation trees
local bitdown=require("wetgenes.gamecake.fun.bitdown")		-- ascii to bitmap
local chipmunk=require("wetgenes.chipmunk")					-- 2d physics https://chipmunk-physics.net/


-----------------------------------------------------------------------------
--[[#hardware

select the hardware we will need to run this code, eg layers of 
graphics, colors to use, sprites, text, sound, etc etc.

Here we have chosen the default 320x240 setup.

This also provides a default main function that will upload the 
graphics and call the provided update/draw callbacks.

]]
-----------------------------------------------------------------------------
oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	graphics=function() -- use a function to return a value created later
		return graphics
	end,
	update=function() -- called at a steady 60fps
		if setup then setup() setup=nil end -- call an optional setup function *once*
		entities.call("update")
		local space=entities.get("space")
		if space then -- update space
			space:step(1/(60*2)) -- double step for increased stability, allows faster velocities.
			space:step(1/(60*2))
		end
		local cb=entities.get("callbacks") or {} -- get callback list 
		entities.set("callbacks",{}) -- and reset it
		for _,f in pairs(cb) do f() end
	end,
	draw=function() -- called at actual display frame rate
		entities.call("draw")
	end,
})

-----------------------------------------------------------------------------
--[[#graphics


define all graphics in this global, we will convert and upload to tiles 
at setup although you can change tiles during a game, we try and only 
upload graphics during initial setup so we have a nice looking sprite 
sheet to be edited by artists

]]
-----------------------------------------------------------------------------
graphics={
{0x0000,"_font",0x0340}, -- preserve the 4x8 and 8x8 font area 64x3 tiles
}

-- load a single sprite
graphics.load=function(idx,name,data)
	local found
	for i,v in ipairs(graphics) do
		if v[2]==name then
			found=v
			break
		end
	end
	if not found then -- add new graphics
		graphics[#graphics+1]={idx,name,data}
	else
		found[1]=idx
		found[2]=name
		found[3]=data
	end
end	

-- load a list of sprites
graphics.loads=function(tab)
	for i,v in ipairs(tab) do
		graphics.load(v[1],v[2],v[3])
	end
end

-----------------------------------------------------------------------------
--[[#entities


	entities.reset()
	
empty the list of entites to update and draw


	entities.caste(caste)

get the list of entities of a given caste, eg "bullets" or "enemies"


	entities.add(it,caste)
	entities.add(it)

add a new entity of caste or it.caste if caste it missing to the list 
of things to update 


	entities.remove(it)

Remove an entity from its caste table.


	entities.call(fname,...)

for every entity call the function named fname like so it[fname](it,...)


	entities.count(caste)

Return the count of the number of entities in a given caste.


	entities.get(name)

get a value previously saved, this is an easy way to find a unique 
entity, eg the global space but it can be used to save any values you 
wish not just to bookmark unique entities.


	entities.set(name,value)

save a value by a unique name


	entities.manifest(name,value)

get a value previously saved, or initalize it to the given value if it 
does not already exist. The default value is {} as this is intended for 
lists.


]]
-----------------------------------------------------------------------------
entities={} -- a place to store everything that needs to be updated

-- clear the current data
entities.reset=function()
	entities.data={}
	entities.info={}
end

-- get items for the given caste
entities.caste=function(caste)
	caste=caste or "generic"
	if not entities.data[caste] then entities.data[caste]={} end -- create on use
	return entities.data[caste]
end

-- add an item to this caste
entities.add=function(it,caste)
	caste=caste or it.caste -- probably from item
	caste=caste or "generic"
	local items=entities.caste(caste)
	items[ #items+1 ]=it -- add to end of array
	return it
end

-- remove an item
entities.remove=function(it)
	caste=it.caste or "generic"
	for i=1,#entities.data[caste] do -- find it
		local v=entities.data[caste][i]
		if v==it then
			table.remove(entities.data[caste],i) -- remove it
			return v
		end
	end
end

-- call this functions on all items in every caste
entities.call=function(fname,...)
	local count=0
	for caste,items in pairs(entities.data) do
		for idx=#items,1,-1 do -- call backwards so item can remove self
			local it=items[idx]
			if it[fname] then
				it[fname](it,...)
				count=count+1
			end
		end			
	end
	return count -- number of items called
end

-- get/set info associated with this entities
entities.get=function(name)       return entities.info[name]						end
entities.set=function(name,value)        entities.info[name]=value	return value	end
entities.manifest=function(name,empty)
	if not entities.info[name] then entities.info[name]=empty or {} end -- create empty
	return entities.info[name]
end
entities.count=function(caste) local c=entities.data[caste] return c and #c or 0 end

-- also reset the entities right now creating the initial data and info tables
entities.reset()

-----------------------------------------------------------------------------
--[[#entities.systems

A global table for entity systems to live in.

	entities.systems_call(fname,...)
	
Call the named function on any systems that currently exist. For 
instance entities.systems_call("load") is used at the bottom 
of this file to prepare graphics of registered systems.

]]
-----------------------------------------------------------------------------
entities.systems={}
entities.systems_call=function(fname,...)
	for n,v in pairs(entities.systems) do
		if type(v)=="table" then
			if v[fname] then v[fname](v,...) end
		end
	end
end

-----------------------------------------------------------------------------
--[[#entities.systems.space

	space = entities.systems.space.setup()

Create the space that simulates all of the physics.

]]
-----------------------------------------------------------------------------
entities.systems.space={
setup=function()

	local space=entities.set("space", chipmunk.space() ) -- create space
	
	space:gravity(0,0)
	space:damping(0.5)
	space:sleep_time_threshold(1)
	space:idle_speed_threshold(10)

	-- run all arbiter space hooks that have been registered
	entities.systems_call("space")

	return space
end,
}


-----------------------------------------------------------------------------
--[[#entities.systems.player

	player = entities.systems.player.add(idx)

Add a player

]]
-----------------------------------------------------------------------------
entities.systems.player={

load=function() graphics.loads{

-- 4 x 16x32
{nil,"player_ship",[[
. . . . . . . . 
. . . . . . . . 
. . . 7 . . . . 
. . 7 7 7 . . . 
. . . 7 . . . . 
. 7 7 7 7 7 . . 
. 7 . 7 . 7 . . 
. . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_player={} -- when a player collides with something
		arbiter_player.presolve=function(it)
			if it.shape_a.player and it.shape_b.invader then
				it.shape_a.player.bang =it.shape_b.invader
				it.shape_b.invader.bang=it.shape_a.player
			end
			return false
		end
	space:add_handler(arbiter_player,space:type("player"))

end,


add=function(i)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local player=entities.add{caste="player"}

	player.idx=i
	player.score=0
	

	player.color={r=1/8,g=6/8,b=1/8,a=1}
	player.frame=0
	player.frames={ names.player_ship.idx+0 }
	
	player.body=space:body(1,math.huge)
	player.body:position(160,220)
	player.body:velocity(0,0)
	
	player.scale=4
						
	player.shape=player.body:shape("box",-3*player.scale,-2*player.scale,2*player.scale,3*player.scale,0)
	player.shape:friction(1)
	player.shape:elasticity(0)
	player.shape:collision_type(space:type("player"))
	player.shape.player=player

	player.remove=function()

		entities.remove(player)

		if player.shape then space:remove(player.shape) player.shape=nil end
		if player.body  then space:remove(player.body)  player.body=nil  end

	end

	player.update=function()
		local up=ups(0) -- the controls for this player
		
		local px,py=player.body:position()
		local vx,vy=player.body:velocity()
		local s=4

		if up.button("fire_set") or
			( up.button("left") and up.button("right_set") ) or
			( up.button("right") and up.button("left_set") ) then
		
			if up.button("mouse_left_set") then player.mouse=true else player.mouse=false end
			
			if entities.count("missile")==0 then
			
				entities.systems.missile.add(px-2,py-8,0,-256)

			end

		end

--		if up.button("up")    then if vy>0 then vy=0 end vy=vy-s end
--		if up.button("down")  then if vy<0 then vy=0 end vy=vy+s end
		if up.button("left")  then if vx>0 then vx=0 end vx=vx-s end
		if up.button("right") then if vx<0 then vx=0 end vx=vx+s end
		
		if player.mouse then
			local mx=up.axis("mx")
			if     mx < px then if vx>0 then vx=0 end vx=vx-s
			elseif mx > px then if vx<0 then vx=0 end vx=vx+s
			end
		end

		if px<0  +12 and vx<0 then vx=0 end
		if px>320-8  and vx>0 then vx=0 end

		player.body:velocity(vx,vy)

		if player.bang then
			entities.systems.bang.add({px=px,py=py})
			entities.get("score").gameover=240
			player.remove()
		end

	end

	player.draw=function()

			local px,py=player.body:position()
			local t=player.frames[1]
			
			system.components.sprites.list_add({t=t,h=8,px=px,py=py,s=player.scale,color=player.color})			

	end
	
	return player
end,
}


-----------------------------------------------------------------------------
--[[#entities.systems.invader

an invader

]]
-----------------------------------------------------------------------------
entities.systems.invader={

load=function() graphics.loads{

-- 4 x 16x32
{nil,"invader_ship",[[
. . . . . . . . 
. . . . . . . . 
. . 7 7 7 7 . . 
. . . 7 7 . . . 
. . 7 7 7 7 . . 
. . 7 . . 7 . . 
. . . . . . . . 
. . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_invader={} -- trigger things
	space:add_handler(arbiter_invader,space:type("invader"))


	local arbiter_invader_aura={} -- when a player collides with something
		arbiter_invader_aura.presolve=function(it)
			if it.shape_a.invader_aura and it.shape_b.invader_aura then
				return true
			end
			return false
		end
	space:add_handler(arbiter_invader_aura,space:type("invader_aura"))

end,


add=function(x,y)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local invader=entities.add{caste="invader"}

	invader.idx=i
	invader.score=0
	

	invader.color={r=1/8,g=6/8,b=1/8,a=1}
	invader.frame=0
	invader.frames={ names.invader_ship.idx+0 }
	
	invader.body=space:body(1,math.huge)
	invader.body:position(x,y)
	invader.body:velocity(0,0)
	
	invader.scale=4
						
	invader.shape=invader.body:shape("box",-2*invader.scale,-2*invader.scale,2*invader.scale,2*invader.scale,0)
	invader.shape:friction(0)
	invader.shape:elasticity(0)
	invader.shape:collision_type(space:type("invader"))
	invader.shape.invader=invader

-- used to keep invaders away from each other
	invader.shape2=invader.body:shape("box",-3*invader.scale,-3*invader.scale,3*invader.scale,3*invader.scale,0)
	invader.shape2:friction(0)
	invader.shape2:elasticity(0)
	invader.shape2:collision_type(space:type("invader_aura"))
	invader.shape2.invader_aura=true


	invader.remove=function()

		entities.remove(invader)

		if invader.shape2 then space:remove(invader.shape2) invader.shape2=nil end
		if invader.shape  then space:remove(invader.shape)  invader.shape=nil  end
		if invader.body   then space:remove(invader.body)   invader.body=nil   end

	end

	invader.update=function()
		
		local px,py=invader.body:position()

		local vx,vy=invader.body:velocity()
		vx=(vx*3+invader.horde.vx)/4
		vy=(vy*3+invader.horde.vy)/4
		invader.body:velocity(vx,vy)
		
		if py<  0   then invader.horde.hit_top=true end

		if px<  0+8 then invader.horde.hit_left=true end
		if px>320-8 then invader.horde.hit_right=true end
		
		if invader.bang then
			invader.remove()
			entities.get("score").inc(invader.horde.score)
		end

		if py>240 then
			invader.bang={}
			for i,v in ipairs(entities.caste("player")) do -- should only be one
				invader.bang=v
				v.bang=invader
			end
			entities.systems.bang.add({px=px,py=py})
		end

	end

	invader.draw=function()

			local px,py=invader.body:position()
			local t=invader.frames[1]
			
			system.components.sprites.list_add({t=t,h=8,px=px,py=py,s=invader.scale,color=invader.color})			

	end
	
	return invader
end,
}

-----------------------------------------------------------------------------
--[[#entities.systems.horde

The invading horde

]]
-----------------------------------------------------------------------------
entities.systems.horde={

sound=function()

	local sfx=system.components.sfx

	sfx.render{
		name="move1",
		fwav="square",
		frequency="C1",
		volume=1.0,
		duty=0.3,
		adsr={
			1.0,
			0.0,0.0,0.0,0.2
		},
    }

	sfx.render{
		name="move2",
		fwav="square",
		frequency="G1",
		volume=1.0,
		duty=0.2,
		adsr={
			1.0,
			0.0,0.0,0.0,0.2
		},
    }

end,

add=function(cx,cy,cs)

	local horde=entities.add{caste="horde"}
	
	horde.state="right"
	horde.wait=0

	horde.vx=0
	horde.vy=0
	
	horde.moved=0
	horde.noise="move1"
	
	horde.score=cs-2
	if horde.score<1 then horde.score=1 end

	if cx>11 then cx=11 cs=cs+1 end -- 12x7 fills the entire screen
	if cy>7  then cy=7  cs=cs+1 end

	for y=1,cy do
		for x=1,cx do

			local invader=entities.systems.invader.add(x*24,y*24)
			
			invader.horde=horde

		end	
	end

	horde.remove=function()
		entities.remove(horde)
	end
	
	horde.update=function()
	
		local sfx=system.components.sfx


		horde.moved=horde.moved+math.abs(horde.vx) -- +math.abs(horde.vy)
		if horde.moved > 1024 then
			horde.moved=0
			sfx.play(horde.noise,1,1)
			if horde.noise=="move1" then horde.noise="move2" else horde.noise="move1" end
		end
	
		if entities.count("invader")==0 then -- alldead
			horde.remove()
			entities.systems.horde.add(cx+1,cy+1,cs+1)
			return
		end

		local count=entities.count("invader")
		if count<1 then count=1 end
		local speed=cs*64/count
	
		if     horde.state=="left" then
			horde.vx=-speed
			horde.vy=0
			if horde.hit_left or horde.hit_top then
				horde.state="down_right"
				horde.wait=60
			end
		elseif horde.state=="right" then
			horde.vx=speed
			horde.vy=0
			if horde.hit_right or horde.hit_top then
				horde.state="down_left"
				horde.wait=60
			end
		elseif horde.state=="down_left" or horde.state=="down_right"  then
			horde.vx=0
			horde.vy=16
			
			horde.wait=horde.wait-1
			
			if horde.wait<=0 then
				if horde.state=="down_left"  then horde.state="left" end
				if horde.state=="down_right" then horde.state="right" end
			end

		end
		
		horde.hit_top=false
		horde.hit_left=false
		horde.hit_right=false

	end

	return horde
end,

}

-----------------------------------------------------------------------------
--[[#entities.systems.missile

a missile

]]
-----------------------------------------------------------------------------
entities.systems.missile={

load=function() graphics.loads{

-- 4 x 16x32
{nil,"missile",[[
. . . . . . . . 
. . . . . . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . 7 7 . . . 
. . . . . . . . 
. . . . . . . . 
]]},

}end,

space=function()

	local space=entities.get("space")

	local arbiter_missile={} -- trigger things
		arbiter_missile.presolve=function(it)
			if it.shape_a.missile and it.shape_b.invader then
				it.shape_a.missile.bang=it.shape_b.invader
				it.shape_b.invader.bang=it.shape_a.missile
			end
			return false
		end
	space:add_handler(arbiter_missile,space:type("missile"))

end,

sound=function()

	local sfx=system.components.sfx

	sfx.render{
		name="shot",
		fwav="whitenoise",
		frequency="C5",
		volume=1.0,
		duty=0.5,
		adsr={
			1.0,
			0.0,0.0,0.0,0.5
		},
		fm={
			frequency=16,
			fwav="square",
			duty=0.5,
			ffreq=function(it)
				local t1=-35*35
				return function(m,t)
					return sfx.bitsynth.c5+t*t1
				end
			end,
		},
    }

end,

add=function(px,py,vx,vy)

	local sfx=system.components.sfx

	sfx.play("shot",1,1)

	local names=system.components.tiles.names
	local space=entities.get("space")

	local missile=entities.add{caste="missile"}

	missile.color={r=1/8,g=6/8,b=1/8,a=1}
	missile.frame=0
	missile.frames={ names.missile.idx+0 }
	
	missile.body=space:body(1,math.huge)
	missile.body:position(px,py)
	missile.body:velocity(vx,vy)
	
	missile.scale=2
						
	missile.shape=missile.body:shape("box",-1*missile.scale,-2*missile.scale,1*missile.scale,2*missile.scale,0)
	missile.shape:friction(0)
	missile.shape:elasticity(0)
	missile.shape:collision_type(space:type("missile"))
	missile.shape.missile=missile

	missile.remove=function()

		entities.remove(missile)

		if missile.shape then space:remove(missile.shape) missile.shape=nil end
		if missile.body  then space:remove(missile.body)  missile.body=nil  end

	end
	
	missile.update=function()
		
		local px,py=missile.body:position()

		missile.body:velocity(vx,vy) -- keep velocity

		if py<0 or py>240 then
		
			missile.remove()

		end
		
		if missile.bang then
			entities.systems.bang.add({px=px,py=py})
			missile.remove()
		end
		
	end

	missile.draw=function()

		local px,py=missile.body:position()
		local t=missile.frames[1]
		
		system.components.sprites.list_add({t=t,h=8,px=px,py=py,s=missile.scale,color=missile.color})

	end
	
	return missile
end,
}


-----------------------------------------------------------------------------
--[[#entities.systems.bang

a bang

]]
-----------------------------------------------------------------------------
entities.systems.bang={

load=function() graphics.loads{

-- 4 x 16x32
{nil,"bang",[[
. . . 7 7 . . . 
. 7 7 7 7 7 7 . 
. 7 7 7 7 7 7 . 
7 7 7 7 7 7 7 7 
7 7 7 7 7 7 7 7 
. 7 7 7 7 7 7 . 
. 7 7 7 7 7 7 . 
. . . 7 7 . . . 
]]},

}end,

sound=function()

	local sfx=system.components.sfx

	sfx.render{
		name="explode",
		fwav="square",
		frequency="C5",
		volume=1.0,
		duty=0.5,
		adsr={
			1.0,
			0.0,0.0,0.0,0.6
		},
		fm={
			frequency=5*5,
			fwav="triangle",
			duty=0.5,
			ffreq=function(it)
				local f1=sfx.bitsynth.C5
				local f2=sfx.bitsynth.E5
				local f3=sfx.bitsynth.G5
				local t1=-50*50
				local f=function(m,t)
					local t2=t*t1
					if m<0 then return f2+((f1-f2)*-m)+t2 end
					return f2+((f3-f2)*m)+t2
				end
				return f
			end,
		},
    }

end,


space=function()

end,


add=function(it)

	local sfx=system.components.sfx

	sfx.play("explode",1,1)

	local names=system.components.tiles.names
	local space=entities.get("space")

	it.caste="bang"
	local bang=entities.add(it)

	bang.color={r=1/8,g=6/8,b=1/8,a=1}
	bang.frame=0
	bang.frames={ names.bang.idx+0 }

	bang.px=bang.px or 160
	bang.py=bang.py or 120

	bang.scale=bang.scale or 6
	bang.life=bang.life or 12
	
	for i,v in ipairs(entities.caste("invader")) do
		if v.body then
			local px,py=v.body:position()
			local dx=bang.px-px
			local dy=bang.py-py
			local dd=dx*dx + dy*dy
			if dd < 80*80 and dd>0 then
				local d=math.sqrt(dd)
				local nx,ny=dx/d,dy/d
				local vx,vy=v.body:velocity()
				vx=vx-(nx*256)
				vy=vy-(ny*256)
				v.body:velocity(vx,vy)
			end
		end
	end

	bang.remove=function()

		entities.remove(bang)

	end
	
	bang.update=function()

		bang.life=bang.life-1
		
		bang.scale=bang.scale+1
		bang.color.a=bang.life/12
		bang.color.r=bang.color.a*(1/8)
		bang.color.g=bang.color.a*(6/8)
		bang.color.b=bang.color.a*(1/8)
		
		if bang.life<=0 then
			bang.remove()
		end	
	end

	bang.draw=function()

		local t=bang.frames[1]
		
		system.components.sprites.list_add({t=t,h=8,px=bang.px,py=bang.py,s=bang.scale,color=bang.color})			

	end
	
	return bang
end,
}

-----------------------------------------------------------------------------
--[[#entities.systems.score

The score

]]
-----------------------------------------------------------------------------
entities.systems.score={

add=function()
		
	local score=entities.set("score",entities.add{caste="score"})


	score.number=0
	score.highest=0


	score.reset=function(num)
		score.number=num or 0
	end

	score.inc=function(num)
		if entities.count("player")>0 then -- check we are still alive
			score.number=score.number + num
			if score.number > score.highest then score.highest=score.number end
		end
	end

	score.update=function()

		if score.start then

			local up=ups(0) -- the controls for this player
			
		if up.button("fire_clr") or
			( up.button("left") and up.button("right_clr") ) or
			( up.button("right") and up.button("left_clr") ) then

				-- use a setup function so we are called outside of update loop
				setup=function()
					-- remove all old stuff
					for _,name in ipairs{"horde","invader","missile","player"} do
						local tab=entities.caste(name)
						for i=#tab,1,-1 do tab[i].remove() end
					end
					-- add in all new stuff
					entities.systems.player.add(0)	
					entities.systems.horde.add(6,3,3)
					-- reset state
					score.number=0
					score.start=nil
				end
			end

		elseif score.gameover then
		
			score.gameover=score.gameover-1
			
			if score.gameover<=0 then
			
				score.gameover=nil
				score.start=true
			
			end
			
		end

	end
	
	score.draw=function()

		local ctext=system.components.text


		local s="Score : "..score.number
		ctext.text_print(s,(80-#s)/2,1,31,0)
		
		if score.start then
		
			local s="HighScore : "..score.highest
			ctext.text_print(s,(80-#s)/2,10,system.ticks%32,0)

			local s="Press FIRE to start!"
			ctext.text_print(s,(80-#s)/2,15,system.ticks%32,0)

		elseif score.gameover then
		
			local s="!GAME!OVER!"
			ctext.text_print(s,(80-#s)/2,15,system.ticks%32,0)

		end
		
--		ctext.dirty(true)
	
	end


	return score
end,

}

-----------------------------------------------------------------------------
--[[#entities.systems.stars

The stars

]]
-----------------------------------------------------------------------------
entities.systems.stars={

add=function(cx,cy)

	local stars=entities.add{caste="stars"}

	stars.vx=1/8
	stars.vy=2

	local ccopper=system.components.copper
	ccopper.shader_name="fun_copper_stars"		
	ccopper.shader_uniforms.scroll={0,0,0,0}

	stars.update=function()
	
		local player=entities.caste("player")[1]
		if player then
			if player.body then
				local px,py=player.body:position()
				stars.vx=((px-120)/120)
			end
		end
		ccopper.shader_uniforms.scroll[1]=ccopper.shader_uniforms.scroll[1]+stars.vx
		ccopper.shader_uniforms.scroll[2]=ccopper.shader_uniforms.scroll[2]+stars.vy

	end

	return stars
end,

}
-- The GLSL handler will pickup the #shader directive and use all the code following it until the next #shader directive.
--[=[
#shader "fun_copper_stars"

#ifdef VERTEX_SHADER

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 color;

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
varying vec4  v_color;
 
void main()
{
    gl_Position = projection * vec4(a_vertex, 1.0);
	v_texcoord=a_texcoord;
	v_color=color;
}


#endif
#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif


varying vec2  v_texcoord;
varying vec4  v_color;

uniform vec4      scroll;

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)

void mainImage( out vec4 fragColor, in vec2 fragCoord );

void main(void)
{
    vec2 uv=v_texcoord;
    uv.y=iResolution.y-uv.y;
    mainImage(gl_FragColor,uv);
}

// Cellular noise ("Worley noise") in 3D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.
// https://github.com/stegu/webgl-noise

// Modulo 289 without a division (only multiplications)
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
vec3 mod7(vec3 x) {
  return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Permutation polynomial: (34x^2 + x) mod 289
vec3 permute(vec3 x) {
  return mod289((34.0 * x + 1.0) * x);
}

// Cellular noise, returning F1 and F2 in a vec2.
// 3x3x3 search region for good F2 everywhere, but a lot
// slower than the 2x2x2 version.
// The code below is a bit scary even to its author,
// but it has at least half decent performance on a
// modern GPU. In any case, it beats any software
// implementation of Worley noise hands down.

vec2 cellular(vec3 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 1.0 // smaller jitter gives more regular pattern

	vec3 Pi = mod289(floor(P));
 	vec3 Pf = fract(P) - 0.5;

	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);

	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);

	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);

	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);

	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod7(floor(p11*K))*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod7(floor(p12*K))*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;

	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod7(floor(p13*K))*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;

	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod7(floor(p21*K))*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;

	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod7(floor(p22*K))*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;

	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod7(floor(p23*K))*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;

	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod7(floor(p31*K))*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;

	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod7(floor(p32*K))*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;

	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod7(floor(p33*K))*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;

	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;

	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;

	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;

	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;

	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;

	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;

	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;

	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;

	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;

	vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
	vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
	vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
	vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
	vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
	vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
	vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
	vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
	vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

	// Cheat and sort out only F1
	vec3 d1 = min(min(d11,d12), d13);
	vec3 d2 = min(min(d21,d22), d23);
	vec3 d3 = min(min(d31,d32), d33);
	vec3 d = min(min(d1,d2), d3);
	d.x = min(min(d.x,d.y),d.z);
	return vec2(sqrt(d.x)); // F1 duplicated, no F2 computed
}

// main
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

// plasma background
	float f=cellular( vec3(fragCoord+(scroll.xy/32.0),iGlobalTime*3.0)/16.0 ).x;
	vec3 color=vec3(
		(0.0+ f*f*2.0)/32.0 ,
		(0.0+ f*f*2.0)/32.0 ,
		(2.0+ f  *2.0)/32.0 );


	for(float i=1.0;i<=4.0;i+=1.0 )
	{
		float speed=i/8.0;
		f=1.0-cellular( vec3(fragCoord+vec2(i*19.0,0.0)+(scroll.xy*speed),i*19.0)/32.0 ).x;
		f=pow(f,8.0-i);
		f=max(f-0.75,0.0)*4.0;
		color+=vec3(f)*vec3(0.5,0.5,1.0);
	}

	
	fragColor = vec4( color , 1.0 );
}



#endif

#shader
-- end of GLSL code
//]=]

-----------------------------------------------------------------------------
--[[#setup

Called once to setup things in the first update loop after hardware has 
been initialised.

]]
-----------------------------------------------------------------------------
setup_start=function()

	ups(1).touch="left_fire_right" -- enable touch buttons

	entities.systems_call("sound")

	entities.systems.stars.add()
	entities.systems.score.add().start=true

	entities.systems.space.setup()
--	entities.systems.player.add(0)
	
	entities.systems.horde.add(6,3,3)
	
end
setup=setup_start

-- copy images from systems into graphics table
entities.systems_call("load")



