#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local wzips=require("wetgenes.zips")
local wjson=require("wetgenes.json")

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local V2,V3,V4,M2,M3,M4,Q4=tardis:export("V2","V3","V4","M2","M3","M4","Q4")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,avatar)
	avatar=avatar or {}
	avatar.modname=M.modname

	local gl=oven.gl
	local cake=oven.cake
	local sheets=cake.sheets
	local opts=oven.opts
	local canvas=cake.canvas
	local views=cake.views
	local font=canvas.font
	local flat=canvas.flat


	local framebuffers=oven.rebake("wetgenes.gamecake.framebuffers")

	local wgeoms=oven.rebake("wetgenes.gamecake.spew.geoms")
	local wgeoms_avatar=oven.rebake("wetgenes.gamecake.spew.geoms_avatar")
	local geom=oven.rebake("wetgenes.gamecake.spew.geom")
	local geom_dae=oven.rebake("wetgenes.gamecake.spew.geom_dae")
	local geom_gltf=oven.rebake("wetgenes.gamecake.spew.geom_gltf")

	local gui=oven.rebake(oven.modname..".gui")
	local main=oven.rebake(oven.modname..".main")
	local main_zone=oven.rebake(oven.modname..".main_zone")


-- the available mesh names of an avatar

	avatar.mesh_names=wgeoms_avatar.mesh_names

-- the base part names of an avatar
	avatar.part_names=wgeoms_avatar.part_names

-- the base part names of an avatar and current number of dropdowns for each
	avatar.part_counts=wgeoms_avatar.part_counts

-- the material names of an avatar in texture order
	avatar.material_names=wgeoms_avatar.material_names

-- the tweak part names of an avatar
	avatar.tweak_names=wgeoms_avatar.tweak_names

-- the animation names
	avatar.pose_names=wgeoms_avatar.pose_names

	avatar.soul={
		tweaks={
		},
		parts={
			["hair"]=		{{name="hair_base",flags=3}},
			["head"]=		{{name="head_base",flags=3}},
			["body"]=		{{name="body_belly",flags=3}},
			["hand"]=		{{name="hand_base",flags=3}},
			["foot"]=		{{name="foot_bare",flags=3}},
			["mouth"]=		{{name="mouth_base",flags=3}},
			["nose"]=		{{name="nose_base",flags=3}},
			["ear"]=		{{name="ear_base",flags=3}},
			["eye"]=		{{name="eye_base",flags=3}},
			["eyeball"]=	{{name="eyeball_base",flags=3}},
			["eyebrow"]=	{{name="eyebrow_base",flags=3}},
		},
		materials={
			["white"]=	{
							ramp={0x00bbbbbb,0x80cccccc,0xffdddddd},
						},
			["grey"]=	{
							ramp={0x00777777,0x80888888,0xff999999},
						},
			["black"]=	{
							ramp={0x00000000,0x80000000,0xff000000},
						},
			["skin"]=	{
							ramp={0x00dd8877,0x80ee9988,0xffffaa99},
						},
			["lips"]=	{
							ramp={0x00cc7777,0x80dd8888,0xffee9999},
						},
			["hair"]=	{
							ramp={0x00773333,0x80884444,0xff995555},
						},
			["iris"]=	{
							ramp={0x00333377,0x80444488,0xff555599},
						},
			["eye"]=	{
							ramp={0x00bbbbbb,0x80cccccc,0xffdddddd},
						},
			["head1"]=	{
							ramp={0x00bbbb33,0x80cccc44,0xffdddd55},
						},
			["head2"]=	{
							ramp={0x0033bb33,0x8044cc44,0xff55dd55},
						},
			["body1"]=	{
							ramp={0x0033bbbb,0x8044cccc,0xff55dddd},
						},
			["body2"]=	{
							ramp={0x003333bb,0x804444cc,0xff5555dd},
						},
			["foot1"]=	{
							ramp={0x00bb7733,0x80cc8844,0xffdd9955},
						},
			["foot2"]=	{
							ramp={0x00bb3333,0x80cc4444,0xffdd5555},
						},
			["hand1"]=	{
							ramp={0x00bb7777,0x80cc8888,0xffdd9999},
						},
			["hand2"]=	{
							ramp={0x00773333,0x80884444,0xff995555},
						},
		},
	}

	avatar.filter={}

	avatar.filter.tweaks={}

	avatar.filter.materials={}

	avatar.loads=function()

		wgeoms_avatar.loads()

	end

	avatar.setup=function()

		avatar.mstate={}

	end


	avatar.clean=function()

	end

	avatar.msg=function(m)

	end


	avatar.update=function()

		local player=main_zone.scene:caste("player")[1]
		if player then
			local objs=player.avatar

			objs.pose=gui.datas.get_string("pose")

	--		avatar.frame=avatar.frame+1
			local m1= 90*((main.mouse[1]/main.view.vx)-0.5)
			local m2=-45*((main.mouse[2]/main.view.vy)-0.5)
			local m=M4():rotate(m1,{0,1,0}):rotate(m2,{1,0,0})

			objs.adjust_texture_tweak("eyeball.L",m)
			objs.adjust_texture_tweak("eyeball.R",m)

			objs.update()
		end

	end

	avatar.msg=function(w,m)

--[[
		if m.class=="mouse" then

			if m.keyname=="wheel_add" then
				if m.action==-1 then
					local s=avatar.view_scale[1]*1.10
					avatar.view_scale={s,s,s}
				end
			elseif m.keyname=="wheel_sub" then
				if m.action==-1 then
					local s=avatar.view_scale[1]*0.90
					avatar.view_scale={s,s,s}
				end
			elseif m.keyname=="left" then -- click

				if m.action==1 then		avatar.mstate={"left",m.x,m.y}
				elseif m.action==-1 then	avatar.mstate={}
				end

			elseif m.keyname=="right" then -- click

				if m.action==1 then		avatar.mstate={"right",m.x,m.y}
				elseif m.action==-1 then	avatar.mstate={}
				end

			else

				if m.action==0 then -- drag
					if avatar.mstate[1]=="left" then
						local rs=1/2
						avatar.view_orbit[1]=avatar.view_orbit[1]+(m.x-avatar.mstate[2])*rs
						avatar.view_orbit[2]=avatar.view_orbit[2]+(m.y-avatar.mstate[3])*rs

						while avatar.view_orbit[1] >  180 do avatar.view_orbit[1]=avatar.view_orbit[1]-360 end
						while avatar.view_orbit[1] < -180 do avatar.view_orbit[1]=avatar.view_orbit[1]+360 end

						while avatar.view_orbit[2] >  180 do avatar.view_orbit[2]= 180 end
						while avatar.view_orbit[2] < -180 do avatar.view_orbit[2]=-180 end

						avatar.mstate[2]=m.x
						avatar.mstate[3]=m.y
					elseif avatar.mstate[1]=="right" then

						avatar.view_position:add( V3{m.x-avatar.mstate[2],m.y-avatar.mstate[3],0 } )
						avatar.mstate[2]=m.x
						avatar.mstate[3]=m.y
					end
				end

			end

		end
]]
	end

	avatar.draw=function()
	end

	avatar.rebuild=function(name)

		local player=main_zone.scene:caste("player")[1]
		if player then
			local objs=player.avatar

			objs.rebuild(avatar.soul)
		end

	end

	avatar.random=function(opts)

		local rnd=function(a,b)
			local r=math.random()
			return a + ((b-a)*r)
		end

		local soul={}

		soul.tweaks={}
		soul.parts={}
		soul.materials={}

		for i,name in ipairs( avatar.part_names ) do

			local chance=1.0
			if name=="hat" or name=="eyewear" or name=="beard" or name=="tail" or name=="item" then
				chance=0.1
			end

			local r=math.random()

--print( name, r , chance , r < chance )

			if r < chance then -- object

				local poss={}

				for _,v in ipairs( avatar.mesh_names ) do

					local s=v[1]
					if s:sub(1,#name)==name then
						if v.group==1 then
							poss[#poss+1]=v[1]
						end
					end
				end

				if #poss>0 then
					local i=math.random(1,#poss)
					soul.parts[name]={{name=poss[i],flags=3}}
				else
					soul.parts[name]={}
				end

			else
				soul.parts[name]={}
			end

		end

		if soul.parts.hat[1] then soul.parts.hair[1]={name="hair_base",flags=3} end -- smaller hair if we have a hat
		if soul.parts.item[1] then soul.parts.item[1].flags=math.random(1,2) end -- only 1 item in a random hand

		local cmap={
			0x11cccccc,
			0x11888888,
			0x11000000,
			0x11ee9988,
			0x11dd8888,
			0x11884444,
			0x11444488,
			0x11cccccc,
			0x11cc4444,
			0x11cc8844,
			0x11cccc44,
			0x1144cc44,
			0x114444cc,
			0x1144cccc,
			0x11cc8888,
			0x11884444,
		}
		for i,name in ipairs( avatar.material_names ) do

			local r,g,b,a=pack.argb8_b4(cmap[i])

			r=r+rnd(-64,64)
			g=g+rnd(-64,64)
			b=b+rnd(-64,64)
			a=a+rnd(-16,64)

			local ramp=avatar.simp_to_ramp(r,g,b,a)

			soul.materials[name]={ ramp=ramp }

		end

		for i,name in ipairs( avatar.tweak_names ) do
			local tweak={}
			soul.tweaks[name]=tweak
			tweak.scale={}
			tweak.scale[1]    =rnd(0.95,1.05)
			tweak.scale[2]    =rnd(0.95,1.05)
			tweak.scale[3]    =rnd(0.95,1.05)
			tweak.rotate={}
			tweak.rotate[1]   =rnd(-3,3)
			tweak.rotate[2]   =rnd(-3,3)
			tweak.rotate[3]   =rnd(-3,3)
			tweak.translate={}
			tweak.translate[1]=rnd(-0.003,0.003)
			tweak.translate[2]=rnd(-0.003,0.003)
			tweak.translate[3]=rnd(-0.003,0.003)

			if name=="eye" or name=="ear" or name=="mouth" or name=="cheek" or name=="boob" then
				tweak.scale[1]    =rnd(0.75,1.25)
				tweak.scale[2]    =rnd(0.75,1.25)
				tweak.scale[3]    =rnd(0.75,1.25)
			end

			if name=="body" or name=="item" or name=="hat" or name=="hair" or name=="eyeball" then
				tweak.scale[1]    =1
				tweak.scale[2]    =1
				tweak.scale[3]    =1
				tweak.rotate[1]   =0
				tweak.rotate[2]   =0
				tweak.rotate[3]   =0
				tweak.translate[1]=0
				tweak.translate[2]=0
				tweak.translate[3]=0
			end
		end

		avatar.soul=soul
		gui.data_load("all")
	end


	avatar.load=function(filename)

local dofixup=false
local fixup={
	hair="hair_base",
	eye="eye_base",
	foot="foot_bare",
	ear="ear_base",
	eyebrow="eyebrow_base",
	eyeball="eyeball_base",
	head="head_base",
	hand="hand_base",
	nose="nose_base",
	mouth="mouth_base",
}

		local fp=io.open(filename,"r")

		if not fp then
			gui.show_request({
				lines={
					"Could not open file",
					filename,
				},
				ok=function()end,
			})
		else

			local s=fp:read("*all")
			fp:close()
			avatar.soul=wjson.decode(s)

			if avatar.soul.parts then
				for n,v in pairs(avatar.soul.parts) do
					if type(v)=="string" then -- promote to table
						v=fixup[v] or v
						avatar.soul.parts[n]={{name=v,flags=3}}
						dofixup=true
					else
						for i=1,#v do
							if type(v[i])=="string" then
								v[i]={name=v[i],flags=3}
								dofixup=true
							end
						end
					end

				end
				local hair={}
				if not avatar.soul.parts.hair then
					if avatar.soul.parts.hair1 then hair[#hair+1]=avatar.soul.parts.hair1[1] end
					if avatar.soul.parts.hair2 then hair[#hair+1]=avatar.soul.parts.hair2[1] end
					if avatar.soul.parts.hair3 then hair[#hair+1]=avatar.soul.parts.hair3[1] end
					avatar.soul.parts.hair=hair
				end
			end

			if dofixup then
				avatar.soul.tweaks={}
				for n,v in pairs(avatar.soul.materials) do
					if v.diffuse then -- old diffuse
						local m={}
						m.ramp=avatar.simp_to_ramp(v.diffuse[1]*256,v.diffuse[2]*256,v.diffuse[3]*256,16)
						avatar.soul.materials[n]=m
					end
				end
			end
			avatar.soul.tweaks=avatar.soul.tweaks or {}

			gui.data_load("all")

		end
	end

	avatar.save=function(filename,overwrite)

		if not overwrite then

			local fp=io.open(filename,"r")
			if fp then
				fp:close()

				gui.show_request({
					lines={
						"File already exists",
						filename,
						"Overwrite?",
					},
					yes=function() avatar.save(filename,true) end,
					no=function()end,
				})

			else
				overwrite=true
			end



		end

		if overwrite then -- do save

			local fp=io.open(filename,"w")
			if not fp then
				gui.show_request({
					lines={
						"Could not open file",
						filename,
					},
					ok=function()end,
				})
			else
				if not fp:write(wjson.encode(avatar.soul,{pretty="\t"})) then
					gui.show_request({
						lines={
							"Could not write to file",
							filename,
						},
						ok=function()end,
					})
				end
				fp:close()

--				local objs=main_zone.scene.get("player").avatar
--				objs.map:save(filename..".png")
			end

		end

	end


	avatar.simp_to_ramp=wgeoms_avatar.simp_to_ramp
	avatar.ramp_to_simp=wgeoms_avatar.ramp_to_simp
	avatar.ramp_is_simp=wgeoms_avatar.ramp_is_simp

	return avatar
end

