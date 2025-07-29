--
-- (C) 2021 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local zips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local log,dump,display=require("wetgenes.logs"):export("log","dump","display")
local automap=function(it,r) r=r or it for i=1,#it do r[ it[i] ]=i end return r end

local deepcopy=require("wetgenes"):export("deepcopy")

local LINE=function() return debug.getinfo(2).currentline end

local all=require("wetgenes.gamecake.zone.system.all")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local players=M

players.caste="player"

players.uidmap={
	camera=2,
	length=2,
}

players.values={

	pos=V3( 128,-512,128 ),
	vel=V3( 0,0,0 ),
	acc=V3( 0,0,0 ),

	rot=Q4( 0,0,0,1 ),
	ang=V3( 0,0,0 ),

	run=V3( 0,0,0 ),	-- unit vector of run direction

	tweaks=V4( 0,0,0,0 ),

	avatar_time=0,
	avatar_pose="breath",

}
players.types={
	avatar_pose="ignore",
	avatar_time="ignore",
	acc="tween",
	tweaks="tween",
}
-- methods added to system
players.system={}
-- methods added to each item
players.item={}


players.item.set_values=function(player)


	player:set_auto_values()
	player:set("avatar_time",player.avatar.time)
	player:set("avatar_pose",player.avatar.pose)

	player.feet=V3( player.pos[1] , player.pos[2]+(player.body_hover+player.body_radius) , player.pos[3] )

	player:set_body_values()

end

players.item.get_values=function(player)

	player:get_auto_values()

	player.avatar.time=player:get("avatar_time") -- ,player.avatar.anim.length or 0)
	player.avatar.pose=player:get("avatar_pose")
	player.avatar.speed=0
	player.avatar.update_pose()

	player:get_body_values()

-- player direction normal
	player.face=V3(0,0,1)*player.rot
	player.side=V3(1,0,0)*player.rot

-- feet of player
	player.feet=V3( player.pos[1] , player.pos[2]+(player.body_hover+player.body_radius) , player.pos[3] )

end

players.system.setup=function(sys)

	sys.wgeoms_avatar=sys.oven.rebake("wetgenes.gamecake.spew.geoms_avatar")

	sys.wgeoms_avatar.loads()

end

--[[
player.system.draw_hud=function(players)

	local view=sys.oven.cake.views.get()

	players.sheet:batch_start()
	players.sheet:draw(1,0,0,0,view.vy/16,view.vy/16,0)
	players.sheet:batch_stop()
	players.sheet:batch_draw()

end
]]

players.item.get_up=function(player)
	local ups=player.scene.ups -- this is input for this frame acording to network code
	return ups and ups[1] or sys.oven.ups.empty
end

players.item.setup=function(player)
	local sys=player.sys

	player.max_slope=-0.75
	player.min_slope=-1.00
	player.run_speed=2
	player.walk_speed=1

-- shrink fatty a bit so we can fit in 4x4 holes
	local radius  = 2.00 -  0.125
	local capsule = 0.75 + (0.125*2)
	player.body_wide=radius -- belly radius
	player.body_radius=(2*radius+capsule)*0.5 -- half height of capsule
	player.body_hover=1.0 -- hover height above the ground to bottom of capsule

	player.avatar=sys.wgeoms_avatar.avatar({pose="breath"},sys.wgeoms_avatar.random_soul({seed=tostring(math.random())}) )
	player:get_values()

	local world=player:get_singular("kinetic").world

	local shape=world:shape("capsule",radius,capsule)	-- radius , height  so  2*r+h is the height of the capsule
	player.body=world:body( "rigid" , shape , 8*8,  player.pos[1],player.pos[2],player.pos[3]  , 0x0100 )
	player.body.item=player -- backlink for collison

	player.body:ccd(0.9,0.9) -- this *must* be slightly smaller than the real collision which must be a sphere
	player.body:restitution(0.5)
	player.body:friction(0.5)

	player.body:angular_factor(0,0,0) -- I will force rotation

	player:set_body()

	return player
end



--[[
player.item.focus_camera=function(player,camera)

	if player.floor  then
		local d=math.abs(player.floor.dy) -- 0 when exactly on floor
		d=math.min(1,math.max(0,d)) -- clamp
		local s=1-d -- blend
		local pos=V3(player.feet*d)+player.floor.pos*s
		camera.focus:set( pos )
	else
		camera.focus:set( player.feet )
	end
	camera.focus[2]=camera.focus[2]-7
	camera.floor=player.feet[2]

end
]]

-- apply controller to player, setting player.acc and player.vel
players.item.update_control=function(player)

	local camera=player:depend("camera")

	local up=player:get_up()

	local l3_set=( up:get("l3_set") ) or false
	local l3_clr=( up:get("l3_clr") ) or false

	local l2_set=( up:get("l2_set") or up:get("mouse_right_set") ) or false
	local l2_clr=( up:get("l2_clr") or up:get("mouse_right_clr") ) or false
	local r2_set=( up:get("r2_set") or up:get("mouse_left_set") ) or false
	local r2_clr=( up:get("r2_clr") or up:get("mouse_left_clr") ) or false
	local by=( up:get("y") ) or false
	local bx=( up:get("x") ) or false
	local ba=( up:get("a") ) or false
	local ba_set=( up:get("a_set") ) or false
	local ba_clr=( up:get("a_clr") ) or false
	local bb=( up:get("b") ) or false
	local bb_set=( up:get("b_set") ) or false
	local bb_clr=( up:get("b_clr") ) or false
	local l2=( up:get("l2") ) or false
	local l3=( up:get("l3") ) or false
	local rx=( up:axis("rx") ) or 0
	local lx=( up:axis("lx") ) or 0
	local ly=( up:axis("ly") ) or 0


	if player.run==V3(0,0,0) then -- we are walking
		camera.move_and_rotate=true

		local move=((lx*camera.playerx) + (ly*camera.playery) )


		if player.floor then -- move up/down as we follow along the floor normal
			local n=player.floor.nrm
			local slope=(n[2]-player.min_slope)/(player.max_slope-player.min_slope)
			if slope<0 then slope=0 end
			if slope>1 then slope=1 end
			slope=slope*slope

			if slope<=0 then -- follow
				move[2]=-(move[2]+(move[1]*n[1]+move[3]*n[3])*n[2])
			else -- pushback
				local push=move:dot(n)*slope
				if push < 0 then
					move=move + (move*push)
				end
			end
		end

		if move:lenlen() > 1 then move:normalize() end -- no go faster than 1
		move=move*player.walk_speed

	-- handle movement with fake gravity in direction of stick
		if move:len()>0.1 then -- need some movement to cause rotation
			local forward=move:angle(player.face,V3(0,1,0))
			player.rot:rrotate( forward/4 , V3(0,-1,0) )
			player.body_active=true
		else
			player.body_active=nil
		end
		player.acc=(move*(15))*( (player.floor and 5) or (player.water and 1) or 1 )

		if l3_set then -- click to start runinng
			player.run=camera.playery*-player.run_speed -- we run in this direction and speed
		end
	else -- we are running
		camera.move_and_rotate=false

		player.run = player.run + ( player.run:normalize(V3()) * ((ly/ -4)) ) -- adjust speed with forward/back
		player.run:product(Q4("y",-lx)) --rotate a little bit with left/right
		camera.direction=camera:get( "direction" ) -- also adjust camera
		camera.direction=camera.direction-lx
		camera:set( "direction" , camera.direction )


		if player.run:len() > player.run_speed then -- max speed
			player.run = player.run:normalize()*player.run_speed
		end

		local move=V3(player.run) -- do not alter run as it is our "target" direction and speed

		if player.floor then -- move up/down as we follow along the floor normal
			local n=player.floor.normal
			move[2]=-(move[2]+(move[1]*n[1]+move[3]*n[3])*n[2])
		end

		local forward=move:angle(player.face,V3(0,1,0))
		player.rot:rrotate( forward/4 , V3(0,-1,0) )
		player.body_active=true

		player.acc=(move*(15))*( (player.floor and 5) or (player.water and 1) or 1 )

		if l3_set or ( player.run:len()<0.9 ) then -- stop runing if we slow down target ( pull back on stick ) or click again
			player.run=V3(0,0,0) -- stop running
		end
	end

--print("set",ba_set,player.floor,player.water)
	if ba_set and (
			( player.floor )--and ( player.vel[2]*player.vel[2] < 8*8 ) )
			or
			player.water
		) then -- jump on release
--		print("jump",player.vel)

		player.body_active=true
		player.vel[2]=-50 -- player.run and -50 or -30
		if player.floor and player.floor.body and player.floor.body.item and player.floor.body.item.pos then -- push object we are standing on?
			player.floor.body.item:impulse_body( player.vel*(-0.5) , player.floor.pos - player.floor.body.item.pos )
			player:get_singular("kinetic").watch=player.floor.body.item
		end

	end

	if l2_set or r2_set then
		player.prep={count=0}
	end

	if l2_clr or r2_clr then
		player.prep=nil
	end

	if bb_clr then

		local sides={4,6,8,12,20}
		sides=sides[ player.sys:get_rnd(#sides) ]
		player:set("sides",sides)
		local color=player.sys:get_rnd()
		local shade=player.sys:get_rnd(0.25,0.75)

		local pos=player.pos+V3(0,-4,0)+(player.face*4)
		local vel=(player.face+V3(0,-1,0))*32
		local size=player.sys:get_rnd(1,7)
		size={size,size,size}
		local solid=player.scene:create{"solid",pos=pos,size=size,sides=sides,vel=vel,shade=shade,color=color}
	end

end

players.item.update=function(player)

--	player.floor={gy=0}

	player:get_values()

	local camera=player:depend("camera")



	local best_hit_fraction=1
	player.floor=nil
	local tw={0.3,0.3}
	local nrm=V3()
	for i=1,5 do -- check for floor distance

		local b=player.feet
		if     i==1 then	b=b
		elseif i==2 then	b=b+( (V3( 1,0, 1)*player.rot) * (player.body_wide*0.5) )
		elseif i==3 then	b=b+( (V3( 1,0,-1)*player.rot) * (player.body_wide*0.5) )
		elseif i==4 then	b=b+( (V3(-1,0,-1)*player.rot) * (player.body_wide*0.5) )
		elseif i==5 then	b=b+( (V3(-1,0, 1)*player.rot) * (player.body_wide*0.5) )
		end

		local world=player:get_singular("kinetic").world
		local l=(player.body_hover)*2
		local test=world:ray_test({
			ray={
				V3{b[1],b[2]-l*0.5,b[3]},
				V3{b[1],b[2]+l*0.5,b[3]},
			},
			cmask=0x00ff,
		})
		if test.hit then
			nrm=nrm+V3(test.hit.normal)

			local dy=(test.hit.fraction*l)-(l*0.5) -- how much we need to move to be perfectly on the floor
--			if dy<-0.6 then dy=-0.6 end
			local pbest=function(i,n)
				if not tw[i] then tw[i]=n end
				if n<tw[i] then tw[i]=n end
			end
			if     i==1 then	pbest(1,dy) ; pbest(2,dy)
			elseif i==2 then	pbest(1,dy)
			elseif i==3 then	pbest(1,dy)
			elseif i==4 then	pbest(2,dy)
			elseif i==5 then	pbest(2,dy)
			end -- tweak feets

			if test.hit.fraction<0.75 and test.hit.fraction<best_hit_fraction then
				best_hit_fraction=test.hit.fraction
				player.floor=test.hit
				test.hit.length=(test.hit.fraction*l) -- length of ray
				test.hit.dy=test.hit.length-l*0.5 -- how much we need to move to be perfectly on the floor
				test.hit.pos=V3{b[1],b[2]-(l*0.5)+test.hit.length,b[3]} -- floor hit position
				if test.hit.dy<0 then
--					test.hit.gy=test.hit.dy*600*(1.0-math.pow(math.max(0,1.0-test.hit.fraction*2.0),1.0))
					test.hit.gy=test.hit.dy*50
				else
					test.hit.gy=test.hit.dy*50
				end
				test.hit.body=world.bodies[test.hit.body_ptr]

--			print( test.hit.body and test.hit.body.item and test.hit.body.item.caste )
			end
		end
	end
	if player.floor then player.floor.nrm=nrm:normalize() end

	player.tweaks=V4( tw[1] or 0, tw[2] or 0, 0,0 )

--	local up=srecaps.ups( player.zip.up )
--	print("UP",player.up,up)

	player:update_control()

	local v=V3(player.acc) ; v[2]=0
	if v:lenlen() >= 5*5 then
		player.avatar.speed=1/16
--		if player.avatar.pose=="breath" then player.avatar.time=0 end
		player.avatar.pose="walk"
		if player.floor or player.water then
			player.avatar.speed=(1/16)*v:len()/32 -- this speed stops feet from slipping while animating, maybe?
		end
		if player.avatar.speed>4/16 then player.avatar.speed=4/16 end -- walk looks bad any faster than 4x
	else
		player.avatar.speed=player.scene.ticklength
		player.avatar.pose="breath"
	end
--print(LINE(),player.avatar.pose,v:lenlen())

	local water_height=0xffff
	if player.scene.systems.water then
		water_height=player.scene.systems.water:get_water_height(player.pos)
	end
	local submerged=player.feet[2]-water_height
	display( water_height , submerged )
	if submerged>0 then -- float in water

		player.water=submerged
		player.body:damping(31/32,0) -- water friction
		player.acc[2]=player.acc[2] - ((submerged)*32) -- use gravity to float

	else

		player.water=false
		if player.floor then
			player.body:damping(1023/1024,0) -- floor friction
		else
			player.body:damping(1/2,0) -- air friction
		end

	end

	if player.floor then

		player.acc[2]=player.acc[2] + player.floor.gy*4
		player.body:damping(1023/1024,0) -- floor friction

		local n=player.floor.nrm
		local slope=(n[2]-player.min_slope)/(player.max_slope-player.min_slope)
		if slope<0 then slope=0 end
		if slope>1 then slope=1 end
		slope=slope*slope

		if slope>0 then -- slide
			local push=100*slope
			if player.run~=V3(0,0,0) then push=push*4 end -- we are runing

			local d=V3(player.floor.normal)*push
			d[2]=d[2]+push
			player.acc=player.acc+d
		end

	else
		player.acc[2]=player.acc[2]+100 -- default gravity
	end

	if player.prep then
		local prep=player.prep
		prep.count=prep.count+(1/60)

-- Face towards aiming point
		local aim=V3( -camera.mtx[ 9] , -camera.mtx[10] , -camera.mtx[11] ) -- cam nrm
		local a=aim:angle(player.face,V3(0,1,0))
		player.rot:rrotate( 1/4 * a , V3(0,-1,0) ):normalize()
	end


--[[
	if l2_clr or r2_clr then
		local smash={}
		player.smash=smash
		if     r2_clr then smash.info=B.hits.slash
		elseif l2_clr then smash.info=B.hits.smash
		end
		smash.count=1/16
		smash.pos=V3(player.feet + player.face*6 + V3(0,-3,0) )
		smash.rot=V3(player.rot)



		local world=player.scene.systems.physics.world
		local shape=world:shape("box",smash.info.siz[1],smash.info.siz[2],smash.info.siz[3])
		local ghost=world:body( "ghost" , shape , 0,  smash.pos[1],smash.pos[2],smash.pos[3] )
		local overlaps=ghost:overlaps()
		print(#overlaps)
		for i,body in ipairs(overlaps) do
			if body.item then
				print(body.item and body.item.caste)
				if body.item.caste=="tree" or body.item.caste=="rock" then
					local timbers=scene.systems.timber
					local it=body.item
					local boot
					local hit_pos=V3(body.item.pos)
					local hit_up=V3(0,-1,0)*it.rot
					if body.item.caste=="tree" then
						boot={"timber",mode="tree"}
						local p=hit_pos+V3(0,-it.siz[2]*0.5-it.siz[1]-3,0)*it.rot
						boot.pos=V3(p)
						boot.rot=Q4(it.rot)
						boot.siz=V3(it.siz)

						-- delete item from sync history and world
						body.item:purge()
					end
					if body.item.caste=="rock" then
						boot={"timber",mode="rock"}
						local g=body.item.geom:duplicate()
						boot.pos=V3( g:find_center() )
						g:adjust_position(-boot.pos[1],-boot.pos[2],-boot.pos[3]) -- center on our position
						boot.zip={}
						boot.zip.geoms=g:segmentris() -- create segments
						for _,g in ipairs(boot.zip.geoms) do
							for i,v in ipairs(g.verts) do
								v[4]=1 v[5]=1 v[6]=1
								v[7]=0.5
								v[8]=5/8
							end
						end

						local r=g:max_radius()/4
						local g=geom.box( {}, { {-r,-r,-r} , {r,r,r} } )
						for i,v in ipairs(g.verts) do
							v[4]=1 v[5]=1 v[6]=1
							v[7]=0.5
							v[8]=7/8
						end
--						g:adjust_position(unpack(it.pos))
						table.insert(boot.zip.geoms,1,g )

						-- delete item from sync history and world
						body.item:purge()
					end
					if boot then
						local timber=timbers:create(boot)
						local up=hit_up*400
						timber.body:impulse( up[1] , up[2] , up[3] )
					end
				else
					if body.item.do_smash then
						body.item:do_smash(smash.pos,player.face)
					end
				end
			end
		end
		ghost:destroy()

		player.prep=nil

	end
	if player.smash then
		if player.smash.count then
			player.smash.count=player.smash.count-(1/60)
		end
	end
]]

--	player:body_save()


-- this will need a camera per player
--[[
	player.aim=nil
	do
		local world=player.scene.systems.physics.world
		local cp=V3(  camera.mtx[13] ,  camera.mtx[14] ,  camera.mtx[15] ) -- cam pos
--		local cy=V3( -camera.mtx[ 5] , -camera.mtx[ 6] , -camera.mtx[ 7] ) -- cam nrm
		local cn=V3( -camera.mtx[ 9] , -camera.mtx[10] , -camera.mtx[11] ) -- cam nrm
		local ch
		local cc=""
--		cp=cp+cy*1
		local test=world:ray_test({
			ray={
				cp,
				cp+cn*500,
			},
			cmask=0x00ff,
		})
		if test.hit then
			player.aim=test.hit -- player is aiming at something
			test.hit.dir=cn -- direction of cast ray
			test.hit.pos=cp+(cn*(test.hit.fraction*500))
			test.hit.body=world.bodies[test.hit.body_ptr]
			cc=test.hit.body and test.hit.body.item and test.hit.body.item.caste

			display( test.hit.pos , cc )
		end
	end

]]

-- player.pos[2]=-3.6

	-- apply speed here so we can save it and update does not change it
	player.avatar.time=(player.avatar.time+player.avatar.speed)%(player.avatar.anim.length or 1)
	player.avatar.speed=0

	player:set_body()
--	player:set_values()

--print( player.pos , player.vel , player.acc )

--print( player.body:gravity() )


--	scene.systems.build:update_player(player)

--	print( "PV", player.vel )

end


players.item.update_kinetic=function(player)

	player:get_body()
	player:set_values()

end

players.item.set_body=function(player)

	all.item.set_body(player)

	player.body:gravity( (player.acc[1]) , (player.acc[2]) , (player.acc[3]) )

end

players.item.get_body=function(player)

	all.item.get_body(player)

	display( player.pos , player.rot )
	display( player.vel , player.ang )

	-- feet of player
	player.feet=V3( player.pos[1] , player.pos[2]+(player.body_hover+player.body_radius) , player.pos[3] )

end


players.item.draw=function(player)

	player:get_values()

	player.avatar.update()

--local t1=player:get("avatar_time")
--print(player.avatar.time,t1,player.scene.tween,player.avatar.anim.length)

	local gl=player.gl

--	if not player:depend("camera") then return end

	gl.PushMatrix()
	gl.Translate(player.feet)
	gl.Rotate(player.rot)

	local m1=M4():translate(0,player.tweaks[1],0)
	local m2=M4():translate(0,player.tweaks[2],0)
	player.avatar.adjust_texture_tweak("foot.L",m1)
	player.avatar.adjust_texture_tweak("foot.R",m2)
	player.avatar.adjust_texture_tweak("toes.L",m1)
	player.avatar.adjust_texture_tweak("toes.R",m2)

	player.avatar.draw({})

	gl.PopMatrix()

--[[
	if player.smash then
		gl.PushMatrix()
		gl.Translate(player.smash.pos)
		gl.Rotate(player.smash.rot)
		gl.Color(1,1,1,0.5)
		geom.draw(player.smash.info.g,"gamecake_shader?CAM",function(p)
		end)
		gl.Color(1,1,1,0.25)
		gl.PopMatrix()
		if player.smash.count<0 then -- so we always draw at least once
			player.smash=nil
		end
	end
]]

end

