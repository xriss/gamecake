-- 
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it. 
-- 

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")

-- gonna render some sounds
local djon=require("djon")
local bitsynth=require("wetgenes.gamecake.fun.bitsynth")
local bitsynth_task=require("wetgenes.gamecake.fun.bitsynth_task")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wstr=require("wetgenes.string")

oven.opts.fun="" -- back to menu on reset				

sysopts={
	update=function() update() end, -- call global update
	draw=function() draw() end, -- call global draw

	mode="swordstone", -- select basic text setup on a 256x128 screen using the swanky32 palette.
	lox=256,loy=128, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
	layers=4,
	hardware={ -- modify hardware so we have sprites and move the text layer
		{
			component="copper",
			name="copper",
			autosize="lohi",
			layer=1,
		},
		{
			component="tilemap",
			name="map",
			tiles="tiles",
			tile_size={8,8},
			layer=2,
			autosize="lohi",
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
			autosize="lohi",
		},
	},
	icon=[[
. . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . 
. . . . . Y Y O O Y Y O O o o . . . . 
. . . . . Y Y O O Y Y O O o o . . . . 
. . . Y Y O O o o O O o o r r r r . . 
. . . Y Y O O o o O O o o r r r r . . 
. . . s s s s F F F F F F f f f f . . 
. . . s s s s F F F F F F f f f f . . 
. . . Y Y Y Y Y Y Y Y Y Y o o o o . . 
. . . Y Y Y Y Y Y Y Y Y Y o o o o . . 
. . . s s s s F F F F F F f f f f . . 
. . . s s s s F F F F F F f f f f . . 
. . . o o o o O O O O O O r r r r . . 
. . . o o o o O O O O O O r r r r . . 
. . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . 
]],
}

--------------------------------------------------------------------------------
--
--#sounds

-- render some sounds ( the editor saves djon so cutnpaste here )
sound_data=djon.load([[

{
 sounds = [
  {
   adsr = [ 1 0 0 0 0.2 ]
   duty = 0.5
   fm = { duty=0.5 ffreq=[ `freq_range` 261.630000000001 293.66 329.630000000001 1024 ] frequency=16 fwav=`sine` }
   frequency = D4
   fwav = square
   name = jump
   name_idx = 1
   volume = 0.5
  }
 ]
}

]])


hardware,main=system.configurator(sysopts)

systems={}

all=require("wetgenes.gamecake.zone.flat.all"):import()
systems[#systems+1]=all

kinetic=require("wetgenes.gamecake.zone.flat.kinetic"):import()
systems[#systems+1]=kinetic


all.create_scene=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or require("wetgenes.gamecake.zone.scene").create()
	scene.create_scene=all.create_scene
	scene.oven=oven

	-- do level after players so we can more easily focus
	scene.sortby.level=-1001
	scene:sortby_update()
	
	scene.require_search={
		"wetgenes.gamecake.zone.flat.",
		"",
	}
	for _,it in pairs(systems) do  -- static data and functions for each system
		scene.infos[it.caste]=it
	end

	scene.do_clean=function(scene)
		scene:systems_cocall("clean")
		scene:call("destroy")
		scene:reset()
		scene:reset_ticks()
--		scene.infos.all.scene.initialize(scene)
	end
	
	scene.do_boot_level=function(scene,level)
		local boots={
			{"kinetic",
				LengthUnitsPerMeter=16,
				step=(1/16), -- amount of time to step each frame
				substeps=16, -- number of substeps
				defaults=kinetics.defaults,
				bits=kinetics.bits,
			},
			{"level",idx=level},
--			{"fauna_panda",sname="fauna_panda",pos={192,32,0}},
		}
		scene:creates(boots)
	end
	
	scene.do_setup=function(scene)
		scene:systems_cocall("setup")
		scene:do_boot_level(1)
	end

	scene.do_level=function(scene,level)
		scene:call("destroy")
		scene:reset()
		scene:reset_ticks()
		scene:do_boot_level(level)
	end
	
	scene.infos.all.scene.initialize(scene)

	return scene

end

local main_setup=function()

	for _,snd in ipairs(sound_data.sounds) do
		if snd.name then
			system.components.sfx.render(snd)
		end
	end
	
	-- reset tiles
    local ctiles=system.components.tiles
	ctiles.reset_tiles()
	ctiles.upload_default_font_4x8()
	ctiles.upload_default_font_8x8()
	ctiles.upload_default_font_8x16()

	-- create global scene
	global.scene=all.create_scene()
	scene:do_setup()

	-- add overshade
	overshade:setup()
end
setup=main_setup

update=function()
	if setup then setup=setup() end -- call setup once
	overshade:update()
	scene:do_update()
end

draw=function()
	overshade:draw()
	scene:do_draw()
end

--------------------------------------------------------------------------------
--
--#kinetics
--
kinetics={}

kinetics.defaults={ -- world defaults when creating box2d objects
	world={
		gravity={0,400}, -- default gravity
		sleepThreshold=0*16, -- *16 meter adjust
		hitEventThreshold = 0.0*16, -- any hit please
		restitutionThreshold = 0.0*16, -- bouncy
		contactSpeed = 16*16,
--		maximumLinearSpeed = 400*16,
	},
	body={
		sleepThreshold=1, -- meter adjust
--        linearDamping=1/64,
--        angularDamping=1/64,
		gravityScale=0, -- disable gravity by default
	},
	shape={
		density=1/256,
		material_friction=0.5,
		material_restitution=0.5,
--						material_rollingResistance=0.5,
	},
	joint={
	},
}
kinetics.bits={ -- named collision bit masks
	player={		group=0,
					bits=0x0000000000100,
					mask=0x0000000fffeff,
	},
	floater={		group=0,
					bits=0x0000000010000,
					mask=0x0000000ffffff,
	},
	fauna_egg={		group=0,
					bits=0x0000000010000,
					mask=0x0000000ffffff,
	},
	fauna_slim={	group=0,
					bits=0x0000000010000,
					mask=0x0000000ffffff,
	},
	fauna_trench={	group=0,
					bits=0x0000000010000,
					mask=0x0000000ffffff,
	},
	fruit={			group=0,
					bits=0x0000000010000,
					mask=0x00000000001ff,
	},
	gib={			group=0,
					bits=0x0000001000000,
					mask=0x00000000000ff,
	},
	junk={			group=0,
					bits=0x0000000010000,
					mask=0x0000000ffffff,
	},
}

-- find out what we might be standing on
-- ray translation is expected to be y only and origin is body origin
-- we will ignore "bad" shapes and fill in the length of the ray
kinetics.cast_feet=function(world,ray)
	local hits=world:cast(ray)
	local pass={}

	 -- force meta and calc ray values
	ray.origin=V2(ray.origin)

	for _,hit in ipairs(hits) do
		if hit.shape then
			hit.ray=ray
--			hit.closest=V2(hit.shape:convert(ray.origin[1],ray.origin[2],"closest"))
			hit.point=V2(hit.point)
			hit.normal=V2(hit.normal)
			hit.delta=hit.point-ray.origin
			hit.floor=hit.delta[2] -- delta distance to floor
			if		hit.floor>=2 and 
					hit.normal[2]<0.5 and
					math.abs(hit.delta[1])<(ray.radius-1) then -- filter out bad hits
--print(hit.delta,hit.normal)
				return hit
			end
		end
	end
end

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
	if not it.pz then it.pz=px+py end	-- auto pz
	local map=system.components.map
	it.px=it.px+map.window_px-map.px	-- auto map position
	it.py=it.py+map.window_py-map.py
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

draws.char16=function(s,x,y)
	local sx,sy=system.components.text.text_tile8x16(s)
	system.components.sprites.list_add({
		t=sy*256+sx ,
		hx=8 , hy=16 ,
		ox=0 , oy=0 ,
		px=x+1 , py=y+1 , pz=0 ,
		color=0xff000000,
	})
	system.components.sprites.list_add({
		t=sy*256+sx ,
		hx=8 , hy=16 ,
		ox=0 , oy=0 ,
		px=x , py=y , pz=-1 ,
		color=0xffffffff,
	})
end

draws.string16=function(s,x,y)
	local l=#s
	for i=1,l do
		local c=s:sub(i,i)
		draws.char16(c,x+((i-1)*8),y)
	end
end

draws.string16_in_map=function(s,x,y)
	local map=system.components.map
	x=x+map.window_px-map.px	-- auto map position
	y=y+map.window_py-map.py
	draws.string16(s,x,y)
end


draws.char4=function(s,x,y)
	local sx,sy=system.components.text.text_tile4x8(s)
	system.components.sprites.list_add({
		t=sy*256+(sx/2) ,
		hx=4 , hy=8 ,
		ox=0 , oy=0 ,
		px=x+1 , py=y+1 , pz=0 ,
		color=0xff000000,
	})
	system.components.sprites.list_add({
		t=sy*256+(sx/2) ,
		hx=4 , hy=8 ,
		ox=0 , oy=0 ,
		px=x , py=y , pz=-1 ,
		color=0xffffffff,
	})
end

draws.string4=function(s,x,y)
	local l=#s
	for i=1,l do
		local c=s:sub(i,i)
		draws.char4(c,x+((i-1)*4),y)
	end
end

draws.string4_in_map=function(s,x,y)
	local map=system.components.map
	x=x+map.window_px-map.px	-- auto map position
	y=y+map.window_py-map.py
	draws.string4(s,x,y)
end

draws.integer_to_string_with_commas=function(n)
	local sign
	n=math.floor(n) -- force int
	if n<0 then -- remove negative
		sign="-"
		n=-n
	end
	local s=tostring( n )
	local t={}
	for i=1,#s,3 do
		table.insert(t,1,s:sub(-i-2,-i))
	end
	s=table.concat(t,",")
	if sign then return sign..s end
	return s
end

--------------------------------------------------------------------------------
--
--#overshade

overshade={}

overshade.setup=function(overshade)
    local screen=system.components.screen

	overshade.buttons={
		{ "bl" , 16/32 , 22/32 ,  5.0/32 , "up" },
		{ "bl" , 10/32 , 16/32 ,  5.0/32 , "left" },
		{ "bl" , 22/32 , 16/32 ,  5.0/32 , "right" },
		{ "bl" , 16/32 , 10/32 ,  5.0/32 , "down" },

--		{ "br" , 16/32 , 22/32 ,  5.0/32 , "y" , name="Y" },
		{ "br" , 10/32 , 16/32 ,  5.0/32 , "b" , name="B" },
--		{ "br" , 22/32 , 16/32 ,  5.0/32 , "x" , name="X" },
		{ "br" , 16/32 , 10/32 ,  5.0/32 , "a" , name="A" },
	}
    overshade.touches={}
	
	overshade.DATA_MAX=#overshade.buttons
    overshade.last_touch=0

    overshade.stick={} -- fill this up with left/right/up/down state
    overshade.stick.state={left=-1,right=-1,up=-1,down=-1}

	overshade.enable={"fun_overshade?DATA_MAX="..overshade.DATA_MAX.."&hax="..tostring(overshade),overshade.uniforms}
	overshade:update()
end

overshade.update=function(overshade)
    local screen=system.components.screen
    
    overshade.hx=screen.raw_hx
    overshade.hy=screen.raw_hy
    
    -- shortest edge
    overshade.hh=overshade.hx < overshade.hy and overshade.hx or overshade.hy
    
    overshade.margin=0
    overshade.margin_min=-10/32
    overshade.margin_max=10/32
    
	local stick=overshade.stick
	local stick_state_change=function(newstate)
		local bname={
			left="lx0",
			right="lx1",
			up="ly0",
			down="ly1",
		}
		for i,name in ipairs{"left","right","up","down"} do
			if stick.state[name] ~= newstate[name] then
				stick.state[name] = newstate[name]
				oven.ups.msg({
					time=oven.time(),
					class="button",
					action=stick.state[name],
					button=bname[ name ],
					upidx=1,
				})
			end
		end
	end

	local s=overshade.hh/2
	for i,button in ipairs(overshade.buttons) do
		local base
		local flip
		if     button[1]=="bl" then
			base=V2(0,overshade.hy)
			flip=V2(s,-s)
		elseif button[1]=="br" then
			base=V2(overshade.hx,overshade.hy)
			flip=V2(-s,-s)
		elseif button[1]=="tl" then
			base=V2(0,0)
			flip=V2(s,s)
		elseif button[1]=="tr" then
			base=V2(overshade.hx,0)
			flip=V2(-s,s)
		end
		button.pos=base+((overshade.margin+V2(button[2],button[3]))*flip)
		button.siz=s*button[4]
		if     button[5]=="left"  then	stick[1]=button ; button.stick=stick
		elseif button[5]=="right" then	stick[2]=button ; button.stick=stick
		elseif button[5]=="up"    then	stick[3]=button ; button.stick=stick
		elseif button[5]=="down"  then	stick[4]=button ; button.stick=stick
		end
	end
	
	-- merge stick
	stick.xpos=stick[1] and stick[2] and (stick[1].pos+stick[2].pos)/2 or V2()
	stick.ypos=stick[3] and stick[4] and (stick[3].pos+stick[4].pos)/2 or V2()
	stick.pos=(stick.xpos+stick.ypos)/2

	for _,m in ipairs( ups(1).msgs() ) do -- a cache of msgs can be found here
--DUMP(m)
	
		local id=0
		local pos=V2()
		local action=0

		if m.class=="touch" then
			id=m.id+1
			action=m.action
			pos=V2(m.x,m.y)
--[[
		elseif m.class=="mouse" then
			if m.keyname=="left" or m.keyname=="mouse" then -- only left mouse clicks
				id="mouse"
				action=m.action
			end
			pos=V2(m.x,m.y)
]]
		end

		if id~=0 then -- ignore unknown id
			local touch=overshade.touches[id]
			if action==1 then -- first touch creates
				if not touch then touch={} ; overshade.touches[id]=touch end
				local best_dist=math.huge
				local best_button
				for i,button in ipairs(overshade.buttons) do
					if not button.stick then -- sticks are special
						local d=(pos-button.pos):len()
						if d<best_dist then
							best_dist=d
							best_button=button
						end
					end
				end
				local d=(pos-stick.pos):len()
				if d<best_dist then
					best_dist=d
					best_button=stick
				end
				if best_button then
					touch.button=best_button -- link button to touch
					touch.button.touch=touch -- link touch to button
					touch.start=V2(pos)
				end
				if touch.button.name then
					oven.ups.msg({
						time=oven.time(),
						class="padkey",
						value=1,
						name=touch.button.name,
						id=1,
					})
				end
			end
			if touch then
--print(id,action,pos,touch)
			    overshade.last_touch=oven.time()

				touch.pos=V2(pos)
				
				if touch.button==stick then -- special stick
					local state={left=-1,right=-1,up=-1,down=-1}
					local m=(touch.pos-stick.pos)/(overshade.hh/8) -- normaliseish
					if m[1]<-0.25 then state.left=1  end
					if m[1]> 0.25 then state.right=1 end
					if m[2]<-0.25 then state.up=1    end
					if m[2]> 0.25 then state.down=1  end
					stick_state_change(state)
				end
				
				if action==-1 then -- last touch removes
					-- send virtual button presses back into ups
					if touch.button.name then
						oven.ups.msg({
							time=oven.time(),
							class="padkey",
							value=-1,
							name=touch.button.name,
							id=1,
						})
					end
					touch.button.touch=nil
					overshade.touches[id]=nil
					if touch.button==stick then -- special stick release
						local state={left=-1,right=-1,up=-1,down=-1}
						stick_state_change(state)
					end
				end
			end
		end
--		print(id,act,x,y)
	end

	local age=(oven.time()-overshade.last_touch)
	if age>10 then
		overshade.fade=0
	else
		if age<0 then
			overshade.fade=1
		else
			overshade.fade=1-((age)/10)
		end
	end

	if overshade.fade==0 then
		screen.overshade=nil
	else
		screen.overshade=overshade.enable
	end
end

overshade.draw=function(overshade)
end

overshade.uniforms=function(p)
	for i,button in ipairs(overshade.buttons) do
		local touch=button.touch
		local pos=button.pos
		if touch then
			pos=touch.start -- move button to click pos
		end
		if overshade.stick.state[ button[5] ]==1 then -- stick buttons
			touch=overshade.stick
		end
		local idx=p:uniform("data["..(i-1).."]")
		gl.Uniform4f( idx,   pos[1], pos[2], button.siz, touch and 1 or 0 )
	end

	gl.Uniform4f( p:uniform("fade") , overshade.fade,0,0,0 )

end

-- Include GLSL code inside a comment
-- The GLSL handler will pickup the #shader directive and use all the code following it until the next #shader directive.
--[=[
#shader "fun_overshade"

#ifdef VERTEX_SHADER

attribute vec3 a_vertex;
attribute vec2 a_texcoord;

varying vec2  v_texcoord;
 
void main()
{
	gl_Position = vec4(a_vertex, 1.0);
	v_texcoord = a_texcoord;
}


#endif

#ifdef FRAGMENT_SHADER

#if defined(GL_FRAGMENT_PRECISION_HIGH)
precision highp float; /* really need better numbers if possible */
#endif

varying vec2  v_texcoord;

uniform vec4 fade;

uniform vec4 data[DATA_MAX];

void check(inout vec4 best,vec4 dat)
{
	vec2 dd=v_texcoord.xy-dat.xy;
	float dl=length(dd);
	if( dl<dat.z )
	{
		if( dl<best.w )
		{
			float c=dl/dat.z;
			if(dat.w==0.0)
			{
				if(c>(4.0/8.0))
				{
					float f=pow(1.0-min(1.0,max(0.0,(abs(c-(6.0/8.0))*8.0))),1.0/4.0)*0.25;
					best=vec4( f , f , f , dl );
				}
			}
			else
			{
				float f=pow(1.0-min(1.0,max(0.0,((c-(6.0/8.0))*8.0))),1.0/4.0)*0.25;
				best=vec4( f , f , f , dl );
			}
		}
	}
}

void main(void)
{
	vec4 best=vec4( 0.0 , 0.0 , 0.0 , 1.0/0.0 );

// unrolled loop

	#if 0<DATA_MAX
		check(best, data[0] );
	#endif
	#if 1<DATA_MAX
		check(best, data[1] );
	#endif
	#if 2<DATA_MAX
		check(best, data[2] );
	#endif
	#if 3<DATA_MAX
		check(best, data[3] );
	#endif
	#if 4<DATA_MAX
		check(best, data[4] );
	#endif
	#if 5<DATA_MAX
		check(best, data[5] );
	#endif
	#if 6<DATA_MAX
		check(best, data[6] );
	#endif
	#if 7<DATA_MAX
		check(best, data[7] );
	#endif

	gl_FragColor=vec4( best.xyz , 0.0 )*fade.x;
}

#endif

#shader
]=]

--------------------------------------------------------------------------------
--
--#players

players={}
systems[#systems+1]=players
-- methods added to system
players.system={}
-- methods added to each item
players.item={}

players.caste="player"

-- players are hard linked to these other entities by uid
-- note slot 1 is reserved for a parent which we do not have so it is empty
-- which also means these other entities will back link to us via slot 1
players.uidmap={
	camera=2,
	hud=3,
	hold=4,
	length=4,
}

players.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
	acc=V2( 0,0 ),
	idx=1,
	mode="none",
	side=1,
	foot=8,
	onfloor=0,
	flap=0,
	jump_debounce=0,
	holdtime=0,
	walk=0,
	score=0,
	idle=0,
	spawn=V2( 0,0 ),
}

players.types={
	pos="tween",
	rot="tween",
}

players.graphics={

{nil,"ply1",[[
. . . . . . . . . . . . . . . . 
. . . . . . r r r R . . . . . . 
. . . . r r r r r r R R . . . . 
. . . r 7 7 r 7 7 r r R R . . . 
. . r 7 0 0 7 0 0 7 r r R R . . 
. . r 7 0 0 7 0 0 7 r r R R . . 
. r r r 7 7 r 7 7 r r r R R R . 
. r r r r r r r r r r R R R R . 
. r r 7 r r r 7 r r r R R R R . 
. R r r 7 7 7 r r r R R R R R . 
. . R r r r r r r R R R R R . . 
. . R R r r r r R R R R R R . . 
. . . R R R R R R R R R R . . . 
. . . . R R R R R R R R . . . . 
. . . . . . R R R R . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply1_egg",[[
. . r r r r . . 
. r O O r r r . 
r O Y o O r r r 
r O o o O r r r 
r r O O r r R r 
r r r r r R R r 
. r r r R R r . 
. . r r r r . . 
]]},

{nil,"ply2_egg",[[
. . G G G G . . 
. G d d G G G . 
G d C D d G G G 
G d D D d G G G 
G G d d G G g G 
G G G G G g g G 
. G G G g g G . 
. . G G G G . . 
]]},

{nil,"ply1_hand",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . r O r r R 
. . . . . . . . . . . r O r r R 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply1_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . R R . . R R . . . . . 
. . . . . O O . . O O . . . . . 
. . . . r r r . r r r . . . . . 
]]},

{nil,"ply1_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . R R . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . R R . . . . . . . . . . . R R R R . . . . . . 
. . . . . R R . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . R R . . . . . . . . . . . R R R R . . . . . . 
. . . . . O O . . R R . . . . . . . . . R R . . . . R R . . . . . . . . . R R . . O O . . . . . . . . . . . R R R R . . . . . . 
. . . . r r r . . O O . . . . . . . . . O O . . . . O O . . . . . . . . . O O . r r r . . . . . . . . . . . O O O O . . . . . . 
. . . . . . . . r r r . . . . . . . . r r r . . . r r r . . . . . . . . r r r . . . . . . . . . . . . . . r r r r r . . . . . . 
]],4},

{nil,"ply2",[[
. . . . . . . . . . . . . . . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G 7 0 G 7 0 G g g . . . . 
. . . G 7 7 G 7 7 G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G 7 G 7 G 7 G g g . . . . 
. . . G G 7 G 7 G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . G G G G G G G g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g g g g g g g g . . . . 
. . . g g g g g g g g g . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply2_hand",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . g G d G G . 
. . . . . . . . . . g G d G . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

{nil,"ply2_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . g g . . g g . . . . . 
. . . . . g g . . g g . . . . . 
. . . . . G G . . G G . . . . . 
. . . . . d d . . d d . . . . . 
. . . . G G G . G G G . . . . . 
]]},

{nil,"ply2_walk",[[
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . g g . . g g . . . . . . . . . g g . . . . g g . . . . . . . . . g g . . g g . . . . . . . . . . . g g g g . . . . . . 
. . . . . G G . . g g . . . . . . . . . g g . . . . g g . . . . . . . . . g g . . G G . . . . . . . . . . . g g g g . . . . . . 
. . . . . d d . . G G . . . . . . . . . G G . . . . G G . . . . . . . . . G G . . d d . . . . . . . . . . . G G G G . . . . . . 
. . . . G G G . . d d . . . . . . . . . d d . . . . d d . . . . . . . . . d d . G G G . . . . . . . . . . . d d d d . . . . . . 
. . . . . . . . G G G . . . . . . . . G G G . . . G G G . . . . . . . . G G G . . . . . . . . . . . . . . G G G G G . . . . . . 
]],4},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
players.system.setup=function(sys)

	 system.components.tiles.upload_tiles( players.graphics )

end

players.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
players.item.setup=function(player)
	player:get_values()

	player:setup_kinetic()
	player:set_values()
end

players.item.setup_kinetic=function(player)
	if player.body then return end -- only create once
	local world=player:get_singular("kinetic").world
	player.body=world:body({
		type="dynamic",
		transform={player.pos[1],player.pos[2],player.rot*(math.pi*2)},
	})
	if player.mode=="spawn" or player.mode=="die" then
		-- no collision when spawning
	else
		player.shape=player.body:shape({
			shape="circle",
			radius=4,
			material_friction=0.0, -- slide on walls
			filter="player",
			enableHitEvents=true,
			uid=player.uid,
		})
	end
	player:set_body() -- set positon etc
end

players.item.clean_kinetic=function(player)
	if not player.body then return end -- auto clean
	player.body:destroy()
	player.body=nil
	player.shape=nil
end

players.item.clean=function(player)
	player:clean_kinetic()
end

players.item.update=function(player)

	player:get_values()
	player:setup_kinetic() -- might need to recreate body
	
	local kinetic=player:get_singular("kinetic") -- we will do kinetic things
	local world=kinetic.world
	local level=player:get_singular("level") -- only one level is active at a time

	local up=player.scene.ups[player.idx] or player.sys.oven.ups.empty

	local rx=( up:axis("rx") ) or 0
	local ry=( up:axis("ry") ) or 0
	local lx=( up:axis("lx") ) or 0
	local ly=( up:axis("ly") ) or 0
	local ba_now=( up:get("a") or up:get("a_set") ) or false
	local ba_set=( up:get("a_set") ) or false
	local ba_clr=( up:get("a_clr") ) or false

	local bb_now=( up:get("b") or up:get("b_set") ) or false
	local bb_set=( up:get("b_set") ) or false
	local bb_clr=( up:get("b_clr") ) or false

	local grav=level:get_gravity(player.pos,player.vel)

	player.idle=player.idle+1
	local ll=lx*lx+ly*ly
	if ll>0.25 or bb_set or bb_now or bb_clr or ba_set or ba_now or ba_clr then
		player.idle=0 -- ticks since last controller input
	end

	local foot_touch -- set this if our feet touched something ( not the level )
	local feet=kinetics.cast_feet(world,{ -- this is part of the collision so run it here
		closest=true, -- just want the closest hit
		origin=player.pos,
		points={0,0},
		radius=4,
		translation={0,8},
		filter_categoryBits=0x00000100,
		filter_maskBits=0x00ffffff,
	})
	if feet then
		foot_touch=scene:find_uid(feet.shape.uid) -- remember foot touch for later
	end

	local event_touch=function(it,event)
		if player.mode~="none" then return end -- only if player is in this state

		if it.flag_touch then
			it:flag_touch(player,event)
		end

		if it.caste=="fauna_slim" then -- kill us on touch
			player.mode="die"
			it:do_stun(player,event) -- also kill them
		end
	end

	-- check events for anyone we touched
	for event in kinetic.events:iterate(player.uid) do
		if event.is=="contact_hit" then
			local it=scene:find_uid(
				(event.shapeA.uid == player.uid) and
				event.shapeB.uid or
				event.shapeA.uid ) -- we hit the one that is is not us
			if it then
				event_touch(it,event)
			end
		end
	end
	if foot_touch then -- also did we touched with our feet earlier
		event_touch(foot_touch,{is="foot"})
	end

if player.mode=="spawn" then -- we are spawning but not really here yet

	local aim=V2( lx+(rx*256) , (-1/256)+ly+(ry*256) )
	aim:normalize()

	local dest=player.spawn + aim*4 -- move to choose launch direction
	
	player.pos=player.pos + ( dest -player.pos )*0.25 -- aim


	if ba_set and ( player.idx==1 or player.idle<16*10 ) then -- hide p2 after 10 secs
		player.mode="none"
		player:clean_kinetic()
		player:setup_kinetic()

		player.vel=aim*200

		system.components.sfx.play("jump",1,0.5)
	end

elseif player.mode=="egg" then

	player.acc=grav -- apply gravity

	if ba_set then
		player.mode="none"
		player:clean_kinetic()
		player:setup_kinetic()
	end
	
elseif player.mode=="die" then

	if player.shape then -- stop colliding with anything (start of death)
		player:clean_kinetic()
		player:setup_kinetic()

		player.vel[2]=player.vel[2]-200
	end
	
	player.vel=player.vel+(grav/16) -- 0 mass so we have to apply gravity

	if player.pos[2] > 512 then -- fell way off of screen
	
		player.mode="spawn"
		
		player:clean_kinetic()
		player:setup_kinetic()

		player.pos=V2()+player.spawn
		player.rot=0
		player.vel=V2()
		player.ang=0
		player.acc=V2()
		player.onfloor=0

	end

else
	
	if player.idle==0 then -- flap arms while moving
		player.flap=(player.flap+1)%4
	else
		player.flap=2
	end

	player.acc=V2( 0 ,0) -- reset force
	local va=lx*400  -- velocity we want to achieve
	if va then -- apply left/right movement
		if va<0 and player.vel[1]>0 then player.vel[1]=0 end -- quick turn
		if va>0 and player.vel[1]<0 then player.vel[1]=0 end -- quick turn
		local vb=va-player.vel[1] -- diff from current velocity
		player.acc[1]=player.acc[1]+(vb) -- apply force to make us move at requested speed
	end

	if lx*lx > 1/16 then
		player.walk=player.walk+1
		if player.walk>4 then player.walk=1 end
	else
		player.walk=0
	end
	
	player.vel[1]=player.vel[1]*12/16 --  dampen horizontal velocity
	player.vel[2]=player.vel[2]*14/16 --  dampen vertical velocity
	
	if lx>0 and player.side<0 then  -- turn right
		player.side= 1
		player.holdtime=math.floor(player.holdtime/2)
	end
	if lx<0 and player.side>0 then -- turn left
		player.side=-1
		player.holdtime=math.floor(player.holdtime/2)
	end

--	player.pos=player.pos+player.vel

	local footspeed=0.5
	if feet then

		local d=feet.floor -- Y distance to floor
		local o=player.vel[2] -- original velocity
		local v=((d-9)) -- distance to where we want to be

		if ly>0.25 then -- crouch
			v=v+3
		elseif ly<-0.25 then -- tiptoe
			v=v-2
		end

		player.vel[2]=player.vel[2]*8/16 -- decay bouncy-ness
		if     v>= 0.5 then
			player.acc[2]=footspeed*128
		elseif v<=-0.5 then
			player.acc[2]=footspeed*-256
		else
			player.acc[2]=v*16
		end

		player.foot=d
		if player.foot<7  then player.foot=7  end
		if player.foot>11 then player.foot=11 end
--		player.acc[2]=player.acc[2]+a --  hover

		player.onfloor=4

	else
		if player.foot>11 then player.foot=11 end
		if player.foot<11 then player.foot=player.foot+footspeed end

		player.acc:add(grav) -- gravity
	end

	if player.jump_debounce>0 then -- may press jump a few frames before touching floor
		player.jump_debounce=math.max(0,player.jump_debounce-1)
	elseif player.jump_debounce<0 then -- force minimum time between jumps
		player.jump_debounce=math.min(0,player.jump_debounce+1)
	end
	if player.jump_debounce<0 then player.onfloor=0 end -- just jumped reset floor for a few frames
	if ba_set and player.jump_debounce>=0 then player.jump_debounce=4 end -- start jump now or in the next few frames
	if ba_now and player.jump_debounce>=0 and player.jump_debounce<=1 then player.jump_debounce=1 end -- continue jump only this frame
	if player.jump_debounce>0 and player.vel[2]>-32 then -- start jump
		if player.onfloor>0 then -- can jump
			player.onfloor=0
			player.jump_debounce=-4
			player.vel[2]=-180 -- reset starting force
			system.components.sfx.play("jump",1,0.5)
		end
	end


	local aim=V2( (player.side/128)+lx+(rx*256) , (1/256)+ly+(ry*256) )
	aim:normalize()
	local hold_pos=player.pos+V2(0,-8)+(aim*8)


	local hold=player:depend("hold")
	local hold_len -- how well held 0 is best
	if bb_set and hold then -- throw
		player:depend("hold",0)
		hold:depend("held",0)
		local m=((player.holdtime-8)/16)
		if m>1 then m=1 end -- full speed after 1.5 seconds
		if m<0 then m=0 end -- must hold for at least 0.5 seconds before we can throw
--		local aim=V3( (player.side/128)+lx+(rx*256) , (1/256)+ly+(ry*256) , 0 )
--		aim:normalize()
		hold:set_value("vel",aim*(400*m))
		hold:set_value("danger",16*8)
		hold:setup_kinetic_reshape()
	end
	if bb_set and not hold then -- pickup
		player.holdtime=0
		local v1=V2(-24,-16)
		local v2=V2( 24, 24)

		local overlaps = world:overlap({
			origin=player.pos,
			lowerBound=v1,
			upperBound=v2,
			filter_categoryBits=0xffffffff,
			filter_maskBits=0xffffffff,
		})
		local best_d=math.huge
		local best
		for _,shape in ipairs(overlaps.shapes) do
			-- level shapes will not have a uid
			local it=scene:find_uid(shape.uid)
			if it and it.caste=="junk" then -- can only pickup junk
				local d=hold_pos:distance(it.pos)
				if d<best_d then
					best_d=d
					best=it
				end
			end
		end
		if best then -- pick this up
			hold=best
			player:depend("hold",hold.uid)
			hold:depend("held",player.uid)
			hold:set_value("danger",0)
			hold:setup_kinetic_reshape()
		end
	end
	if hold then
		player.holdtime=player.holdtime+1
		
		if math.abs(lx)>0.5 then -- run to speed up spin timer
			player.holdtime=player.holdtime+2
		end

		hold:get_value("pos")
		hold:get_value("vel")
		local p=hold_pos - hold.pos
		hold_len=p:len() -- distance
		if hold_len>40 then -- too far so just drop
			player:depend("hold",0)
			hold:depend("held",0)
			hold:setup_kinetic_reshape()
		else
			hold.vel:add( p*8 )
			hold.vel:scale(1/2)
			hold:set_value("vel",hold.vel)
			local m=((player.holdtime-8)/16)
			if m>1 then m=1 end -- full speed after 1.5 seconds
			if m<0 then m=0 end -- must hold for at least 0.5 seconds before we can throw
			hold:set_value("ang",m*2*player.side)
		end
	end

--RINT( player.body:convert(10,10,"point_local_to_world") )

end

	player:set_values()

end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
players.item.draw=function(player)

	player:get_values()

	local p=V3( player.pos[1] , player.pos[2], player.pos[1]+player.pos[2] )

	if player.mode=="spawn" then

		local level=player:get_singular("level")
		local t=level:get("time")
		local s=1
		if ( player.idx==1 or player.idle<16*10 ) then -- hide p2 after 10secs of inactivity
			if 1==((t*16)%2) then -- blink
				draws.sprite{ n="ply"..player.idx.."_egg" , p=p , sy=s,sx=-player.side*s }
			end
		end
	elseif player.mode=="egg" then

		draws.sprite{ n="ply"..player.idx.."_egg" , p=p , sx=-player.side }

	elseif player.mode=="die" then

		draws.sprite{ n="ply"..player.idx , p=p , sx=-player.side , sy=-1 }

	else

		local f=math.abs(player.flap-2)
		draws.sprite{ n="ply"..player.idx          , p=p , sx=-player.side }
		draws.sprite{ n="ply"..player.idx.."_hand" , p=p+V3(0,f,-1) , sx=-player.side }

		if player.walk==0 then
			draws.sprite{ n="ply"..player.idx.."_feet" , p=p+V3(0,player.foot-8,-1) , sx=-player.side }
		else
			draws.sprite{ n="ply"..player.idx.."_walk" , p=p+V3(0,player.foot-8,-1) , sx=-player.side , i=player.walk }
		end

	end

end


--------------------------------------------------------------------------------
--
--#scores
--
-- These are very simple objects that just display a number and do not interact
-- with anything so no physics collision no special system setup and no need to
-- clean anything up.

scores={}
systems[#systems+1]=scores
-- methods added to system
scores.system={}
-- methods added to each item
scores.item={}

scores.caste="score"

scores.uidmap={
	length=1,
}

scores.values={
	pos=V2( 0,0 ),
	vel=V2( 0,0 ),

	num=0,
	age=0,
}

scores.types={
	pos="tween",
}


scores.graphics={

}

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
scores.item.setup=function(score)
	score:get_values()

	score:set_values()
end


scores.item.update=function(score)
	score:get_values()
	
	score.vel=score.vel+V2(0,-1)
	score.pos=score.pos+score.vel
	
	if score.pos[2] < -64 then -- off of top of screen
		score:mark_deleted()
	end

	score:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times draws.sprite updates for different results
scores.item.draw=function(score)

	score:get_values()

	local s=draws.integer_to_string_with_commas( score.num )
	draws.string4_in_map(s,score.pos[1]-(#s*2),score.pos[2]-4)

end


--------------------------------------------------------------------------------
--
--#floaters

floaters={}
systems[#systems+1]=floaters
-- methods added to system
floaters.system={}
-- methods added to each item
floaters.item={}

floaters.caste="floater"

floaters.uidmap={
	fauna=2,
	length=2,
}

floaters.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
	acc=V2( 0,0 ),

	touch={},
	sname="fauna_slim",
	spin=0.5,
}

floaters.types={
	pos="tween",
	rot="tween",
}


floaters.graphics={

{nil,"floater_bubble",[[
. . G G G G . . 
. G G d d G G . 
G G d d d d G G 
G G G G G G G G 
G G G G G G G G 
G G g g g g G G 
. G G g g G G . 
. . G G G G . . 
]]},


}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
floaters.system.setup=function(sys)
	 system.components.tiles.upload_tiles( floaters.graphics )
end

floaters.system.clean=function(sys)
end


-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
floaters.item.setup=function(floater)
	floater:get_values()

	floater:setup_kinetic()
	floater:set_values()
end


floaters.item.setup_kinetic=function(floater)
	if floater.body then return end -- already done
	local world=floater:get_singular("kinetic").world
	floater.body=world:body({
		type="dynamic",
		transform={floater.pos[1],floater.pos[2],floater.rot*(math.pi*2)},
	})
	floater.shape=floater.body:shape({
		shape="circle",
		radius=6,
		filter="floater",
		uid=floater.uid,
	})
	floater:set_body()
end

floaters.item.clean_kinetic=function(floater)
	if not floater.body then return end -- already done
	floater.body:destroy()
	floater.body=nil
	floater.shape=nil
end

floaters.item.clean=function(floater)
	floater:clean_kinetic()
end


-- flag functions set values for use in the next update
floaters.item.flag_touch=function(floater,touch,event)
	if touch.caste=="player" then
		floater:set_value("touch",{uid=touch.uid})
	end
end

floaters.item.update=function(floater)
	floater:get_values()
	floater:setup_kinetic() -- might need to recreate body

	local level=floater:get_singular("level") -- only one level is active at a time
	local wind=level:get_wind(floater.pos)

	if floater.touch.uid then
		local hit=scene:find_uid(floater.touch.uid)
		if hit  then -- we are stomped so burst fauna
			local fauna=floater:depend("fauna")
			if fauna then
		 		if fauna:mark_deleted() then -- remove fauna
		 		
		 			-- these need to be live counts
		 			local number_of_fauna_slim=scene:get_number_of("fauna_slim")
		 			local number_of_fruits=scene:get_number_of("fruit")
	
		 			floater:mark_deleted()
					for i=1,16 do
--						local v=V2( hit.vel[1]*2+(100*((hit.sys:get_rnd()-0.5)*2)) ,
--									hit.vel[2]*2+(-100*hit.sys:get_rnd()) )
						local v=V2( floater.sys:get_rnd(-1000,1000)/10 , floater.sys:get_rnd(-1000,1000)/10 )
						local boots={
							{"gib",sname="gib_green",size=4,pos=floater.pos+(v/16),vel=v},
						}
						scene:creates(boots)
					end

					local r=hit.sys:get_rnd(1,100) -- percent chance
					if r<=75 then r=1 elseif r<=95 then r=2 else r=3 end -- 1,2 or 3 drops

					for i=1,r do -- 1-3 fruits
--						local v=V2( hit.vel[1]*2+(100*((hit.sys:get_rnd()-0.5)*2)) ,
--									hit.vel[2]*2+(-100*hit.sys:get_rnd()) )
						local v=V2( floater.sys:get_rnd(-1000,1000)/10 , floater.sys:get_rnd(-1000,1000)/10 )
						local f=math.min(8,number_of_fauna_slim+i) -- maximum fruit
						local boots={
							{"fruit",sname="fruit_"..f,pos=floater.pos+(v/16),vel=v,score=(2^(f-1))*100},
						}
						scene:creates(boots)
					end
					
				end
			end	
			return
		end
		floater.touch={} -- remove hit as it has been dealt with
	end

	floater.acc:set(wind*32)

	floater.spin=floater.spin+(1/16)

	floater:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times draws.sprite updates for different results
floaters.item.draw=function(floater)

	floater:get_values()

	local fauna=floater:depend("fauna")
	if fauna then -- draw fauna in this floater
		local p=V3( floater.pos[1] , floater.pos[2], floater.pos[1]+floater.pos[2] )
		draws.sprite{ n=fauna.sname.."_float"  , p=p , sx=fauna.side ,rz=floater.spin, }
	end
end




--------------------------------------------------------------------------------
--
--#fauna_eggs

fauna_eggs={}
systems[#systems+1]=fauna_eggs
-- methods added to system
fauna_eggs.system={}
-- methods added to each item
fauna_eggs.item={}

fauna_eggs.caste="fauna_egg"

fauna_eggs.uidmap={
	held=1,
	length=1,
}

fauna_eggs.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
--	acc=V2( 0,0 ),
	idx=1,
	sname="fauna_egg",
	egg="fauna_slim",
	hatch=16*5,
}

fauna_eggs.types={
	pos="tween",
	rot="tween",
}


fauna_eggs.graphics={

{nil,"fauna_egg",[[
. . G G G G . . 
. G G d d G G . 
G G d d d d G G 
G G G G G G G G 
G G G G G G G G 
G G g g g g G G 
. G G g g G G . 
. . G G G G . . 
]]},


}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fauna_eggs.system.setup=function(sys)
	 system.components.tiles.upload_tiles( fauna_eggs.graphics )
end

fauna_eggs.system.clean=function(sys)
end

-- we have no system storage but can perform holistic updates on our items here.
fauna_eggs.system.update=function(sys)
	local list_eggs=scene:caste("fauna_egg")
	local list_players=scene:caste("player")
--	print(#list_eggs)
	local best_egg,best_player
	local best_d=math.huge
	for i,egg in ipairs(list_eggs) do
		if not egg:get("deleted") and not egg:depend("held") then -- live egg not held
			for p,player in ipairs(list_players) do
				if not player:get("deleted") then -- live player
					local mode=player:get_value("mode")
					if mode=="none" then -- ignore unhatched players
						local p1=player:get_value("pos")
						local p2=egg:get_value("pos")
						local d=p1:distance(p2)
						if d<best_d then
							best_d=d
							best_egg=egg
							best_player=player
						end
					end
				end
			end
		end
	end
	-- only the egg closest to a player will try and hatch
	if best_egg and best_player then
		best_egg:update_hatch(best_player)
	end
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fauna_eggs.item.setup=function(fauna)
	fauna:get_values()

	fauna:setup_kinetic()
	fauna:set_values()
end


fauna_eggs.item.setup_kinetic=function(fauna)
	if fauna.body then return end -- already done
	local world=fauna:get_singular("kinetic").world
	fauna.body=world:body({
		type="dynamic",
		transform={fauna.pos[1],fauna.pos[2],fauna.rot*(math.pi*2)},
		gravityScale=1, -- use world gravity
	})
	fauna.shape=fauna.body:shape({
		shape="circle",
		radius=4,
		filter="fauna_egg",
		uid=fauna.uid,
	})
	fauna:set_body()
end

fauna_eggs.item.clean_kinetic=function(fauna)
	if not fauna.body then return end -- already done
	fauna.body:destroy()
	fauna.body=nil
	fauna.shape=nil
end

fauna_eggs.item.clean=function(fauna)
	fauna:clean_kinetic()
end

fauna_eggs.item.update_hatch=function(fauna,player)
	fauna:get_values()
	
	fauna.hatch=fauna.hatch-1
	if fauna.hatch<=0 then
		fauna:mark_deleted()
		scene:creates({
			{
				"fauna_slim",
				pos={fauna.pos[1],fauna.pos[2],0},
				vel={fauna.sys:get_rnd(-20,20),fauna.sys:get_rnd(-20,20),0},
			},
		})
	else
		if math.sqrt(fauna.hatch*2)%1==0 then -- jiggle
			local jump=V2(fauna.sys:get_rnd(-10,10),fauna.sys:get_rnd(-60,-30))
			fauna.vel=fauna.vel+jump
		end
	end

	fauna:set_values()
end

fauna_eggs.item.update=function(fauna)
	fauna:get_values()
	fauna:setup_kinetic() -- might need to recreate body

	local level=fauna:get_singular("level") -- only one level is active at a time

-- use global gravity
--	local grav=level:get_gravity(fauna.pos,fauna.vel)
--	fauna.acc:set(grav) -- gravity

	fauna:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times draws.sprite updates for different results
fauna_eggs.item.draw=function(fauna)

	fauna:get_values()

	local p=V3( fauna.pos[1] , fauna.pos[2], fauna.pos[1]+fauna.pos[2] )
	draws.sprite{ n=fauna.sname  , p=p , sx=fauna.side }

end


--------------------------------------------------------------------------------
--
--#fauna_slims

fauna_slims={}
systems[#systems+1]=fauna_slims
-- methods added to system
fauna_slims.system={}
-- methods added to each item
fauna_slims.item={}

fauna_slims.caste="fauna_slim"

fauna_slims.uidmap={
	floater=1,
	length=1,
}

fauna_slims.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
	acc=V2( 0,0 ),
	idx=1,
	side=1,
	foot=0,
	onfloor=0,
	floor_uid=0,
	sname="fauna_slim",
	floor_uid=0,
	mode="none",

-- brain values

	brain_thunk=0,

}

fauna_slims.types={
	pos="tween",
	rot="tween",
}


fauna_slims.graphics={


{nil,"fauna_slim",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . d d d G . . . . . . 
. . . . d d d d d d G G . . . . 
. . . d d d d d d d d G G . . . 
. . d d 7 0 d 7 0 d d d G G . . 
. . d d 0 0 d 0 0 d d d G G . . 
. d d d d d d d d d d d G G G . 
. d d d d d d d d d d G G G G . 
. G G d d d d d d d G G G G G . 
. . G G G G G G G G G G G G . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},


{nil,"fauna_slim_float",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . d d d G . . . . . . 
. . . . d d d d d d G G . . . . 
. . . d d d d d d d d G G . . . 
. . d d d d d d d d d d G G . . 
. . d d 0 0 d 0 0 d d d G G . . 
. d d d d d d d d d d d G G G . 
. d d d d d d d d d d G G G G . 
. G G d d d d d d d G G G G G . 
. . G G G G G G G G G G G G . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},


{nil,"fauna_slim_feet",[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . G G G G G G G G G G G G . . 
. . . G G G G G G G G G G . . . 
. . . . . G G G G G G G . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]]},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fauna_slims.system.setup=function(sys)

	 system.components.tiles.upload_tiles( fauna_slims.graphics )

end

fauna_slims.system.clean=function(sys)
end


-- turn into a floater
fauna_slims.item.do_stun=function(fauna,touch,event)

	if fauna:depend("floater") then -- we are floater, do nothing
		return
	end

	local floater=scene:create({"floater",
		sname=fauna.sname, pos=fauna.pos, vel=fauna.vel+V2(0,-50), })

	-- link us to floater
	floater:depend("fauna",fauna.uid)
	fauna:depend("floater",floater.uid) -- we are turned into a floater

	fauna:clean_kinetic() -- remove body

end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fauna_slims.item.setup=function(fauna)
	fauna:get_values()

	fauna:setup_kinetic()
	fauna:set_values()
end


fauna_slims.item.setup_kinetic=function(fauna)
	if fauna.body then return end -- already done
	local world=fauna:get_singular("kinetic").world
	fauna.body=world:body({
		type="dynamic",
		transform={fauna.pos[1],fauna.pos[2],fauna.rot*(math.pi*2)},
		enableSleep=false,
		gravityScale=0,
		motionLocks_angularZ=true,
	})
	if not fauna:depend("floater") then
		fauna.shape=fauna.body:shape({
			shape="box",
			halfWidth=6,
			halfHeight=4,
			filter="fauna_slim",
			uid=fauna.uid,
		})
	end
	fauna:set_body()
end

fauna_slims.item.clean_kinetic=function(fauna)
	if not fauna.body then return end -- already done
	fauna.body:destroy()
	fauna.body=nil
	fauna.shape=nil
end

fauna_slims.item.clean=function(fauna)
	fauna:clean_kinetic()
end

fauna_slims.item.update=function(fauna)

	if fauna:depend("floater") then -- we are floater, do nothing
		return
	end

	fauna:get_values()
	fauna:setup_kinetic() -- might need to recreate body

	local world=fauna:get_singular("kinetic").world
	local level=fauna:get_singular("level") -- only one level is active at a time

	local grav=level:get_gravity(fauna.pos,fauna.vel)

	local foot_touch -- set this if our feet touched something ( not the level )
	local feet=kinetics.cast_feet(world,{ -- this is part of the collision so run it here
		closest=true, -- just want the closest hit
		origin=fauna.pos,
		points={0,0},
		radius=4,
		translation={0,7},
		filter_categoryBits=0x00000100,
		filter_maskBits=0x00ffffff,
	})
	local footspeed=0.5
	if feet then
		foot_touch=scene:find_uid(feet.shape.uid) -- remember foot touch for later

		local floor=feet.floor-4 -- distance to where we want to be

--[[
		fauna.vel[2]=fauna.vel[2]*8/16 -- decay bouncy-ness
		if     floor>= 0.5 then
			fauna.acc[2]=footspeed*128
		elseif floor<=-0.5 then
			fauna.acc[2]=footspeed*-256
		else
			fauna.acc[2]=floor*16
		end
]]

		fauna.foot=floor
		if fauna.foot<0 then fauna.foot=0 end
		if fauna.foot>3 then fauna.foot=3 end

		fauna.onfloor=1

	else
		if fauna.foot>3 then fauna.foot=3 end
		if fauna.foot<3 then fauna.foot=fauna.foot+footspeed end

	end

	fauna.acc:set(grav) -- gravity

--	fauna.vel[1]=fauna.vel[1]*12/16 --  dampen horizontal velocity
--	fauna.vel[2]=fauna.vel[2]*14/16 --  dampen vertical velocity


	fauna.brain_thunk=fauna.brain_thunk-1
	if fauna.brain_thunk<=0 then
		fauna.brain_thunk=fauna.sys:get_rnd(16,64)
		
		if fauna.onfloor > 0 then -- we can jump

			fauna.vel:set( V2( fauna.sys:get_rnd(-10,-100)*fauna.side , fauna.sys:get_rnd(-20,-200) ) )
			
			if fauna.sys:get_rnd(1,100)>=50 then -- flip direction 50%
				fauna.side=fauna.side*-1
			end
		end

--		brain.jump=V2(fauna.sys:get_rnd(-180,180),fauna.sys:get_rnd(-10,-160))
--		brain.move[1]=brain.jump[1]<0 and -1 or 1

	end

--rint( fauna.vel , fauna.pos , grav)

	fauna:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
fauna_slims.item.draw=function(fauna)

	if fauna:depend("floater") then -- we are floater, do nothing
		return
	end

	fauna:get_values()

	local p=V3( fauna.pos[1] , fauna.pos[2], fauna.pos[1]+fauna.pos[2] )
	
	draws.sprite( { n=fauna.sname , p=p, sx=fauna.side } )
	draws.sprite( { n=fauna.sname.."_feet" , p=p+V3(0,fauna.foot,8) , sx=fauna.side } )

end


--------------------------------------------------------------------------------
--
--#fauna_trenchs

fauna_trenchs={}
systems[#systems+1]=fauna_trenchs
-- methods added to system
fauna_trenchs.system={}
-- methods added to each item
fauna_trenchs.item={}

fauna_trenchs.caste="fauna_trench"

fauna_trenchs.uidmap={
	length=0,
}

fauna_trenchs.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
	acc=V2( 0,0 ),
	idx=1,
	side=1,
	foot=8,
	onfloor=0,
	jump=0,
	flap=0,
	sname="fauna_trench",
	thunk=0,
	floor_uid=0,
	slap=0,
}

fauna_trenchs.types={
	pos="tween",
	rot="tween",
}

fauna_trenchs.graphics={

{nil,"fauna_trench",[[
. . . . . . . . . F F F F F f . . . . . . . . . 
. . . . . . . . . F F F F F f . . . . . . . . . 
. . . . . . F F F F F F F F F F F f . . . . . . 
. . . . . . f f f f f f f f f f f f . . . . . . 
. . . . . . . d d d d d d d d G G . . . . . . . 
. . . . . . d d 7 0 d 7 0 d d d G G . . . . . . 
. . . . . . d d 0 0 d 0 0 d d d G G . . . . . . 
. . . . . F d d d d d d d d d F F F F . . . . . 
. . . . . f F F d d d d d F F f f f f . . . . . 
. . . . . f f F F F d F F F f f f f f . . . . . 
. . . . f f f f f F G F f f f f f f f f . . . . 
. . . . f f f F F F g F F F f f j f f f . . . . 
. . . . f f F F F F g F F F F f j f f f . . . . 
. . . . f f f F F F g F F F f f j f f f . . . . 
. . . . f f f f F F f F F f f f j f f f . . . . 
. . . . F F f f f F s F f f f f j F F F . . . . 
. . . . F F f f F F f F F f f f j F F F . . . . 
. . . . . . f f F F f F F f f f f f . . . . . . 
. . . . . . f f F F g F F f f f f f . . . . . . 
. . . . . . f f F F g F F f f f f f . . . . . . 
. . . . . . f f F F g F F f f f f f . . . . . . 
. . . . . . f f F F G F F f f f f f . . . . . . 
. . . . . . f f F F d F F f f f f f . . . . . . 
. . . . . . G G G G G G G G G G G G . . . . . . 
]],3},

{nil,"fauna_trench_feet",[[
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . . . . . . . . . 
. . . . . . G G G G G G G G G G G G . . . . . . 
. . . . . . G G G G G G G G G G G G . . . . . . 
. . . . . . G G G G G G G G G G G G . . . . . . 
. . . . . . G G G G G G G G G G G G . . . . . . 
. . . . . . . G G G G G G G G G G . . . . . . . 
. . . . . . . . . G G G G G G G . . . . . . . . 
]]},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fauna_trenchs.system.setup=function(sys)

	 system.components.tiles.upload_tiles( fauna_trenchs.graphics )

end

fauna_trenchs.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fauna_trenchs.item.setup=function(fauna)
	fauna:get_values()

	fauna:setup_kinetic()
	fauna:set_values()
end


fauna_trenchs.item.setup_kinetic=function(fauna)
	if fauna.body then return end -- already done
	local world=fauna:get_singular("kinetic").world
	fauna.body=world:body({
		type="dynamic",
		transform={fauna.pos[1],fauna.pos[2],fauna.rot*(math.pi*2)},
	})
	fauna.shape=fauna.body:shape({
		shape="circle",
		radius=4,
		filter="fauna_trench",
		uid=fauna.uid,
	})
	fauna:set_body()
end

fauna_trenchs.item.clean_kinetic=function(fauna)
	if not fauna.body then return end -- already done
	fauna.body:destroy()
	fauna.body=nil
	fauna.shape=nil
end

fauna_trenchs.item.clean=function(fauna)
	fauna:clean_kinetic()
end

fauna_trenchs.item.update_brain=function(fauna,brain)

	fauna.thunk=fauna.thunk-1
	if fauna.thunk<=0 then
		fauna.thunk=fauna.sys:get_rnd(8,32)
		brain.jump=V2(fauna.sys:get_rnd(-180,180),fauna.sys:get_rnd(-10,-160))
		brain.move[1]=brain.jump[1]<0 and -1 or 1
	end
end

fauna_trenchs.item.update=function(fauna)
	fauna:get_values()
	fauna:setup_kinetic() -- might need to recreate body
	
--	local up=fauna.scene.ups[fauna.idx] or fauna.sys.oven.ups.empty

	local level=fauna:get_singular("level") -- only one level is active at a time

	local brain={}
	brain.move=V2(0,0)
	brain.jump=nil
	fauna:update_brain(brain)

	local grav=level:get_gravity(fauna.pos,fauna.vel)

	fauna.acc=V2( 0 ,0) -- reset force
	local va -- velocity we want to achieve
	if fauna.onfloor>0 or fauna.jump>0 then -- when on floor
		va=brain.move[1]*512
	else -- when in air
		va=brain.move[1]*256
	end
	if va then -- apply left/right movement
		if va<0 and fauna.vel[1]>0 then fauna.vel[1]=0 end -- quick turn
		if va>0 and fauna.vel[1]<0 then fauna.vel[1]=0 end -- quick turn
		local vb=va-fauna.vel[1] -- diff from current velocity
		fauna.acc[1]=fauna.acc[1]+(vb) -- apply force to make us move at requested speed
	end
	
	fauna.vel[1]=fauna.vel[1]*12/16 --  dampen horizontal velocity
	fauna.vel[2]=fauna.vel[2]*14/16 --  dampen vertical velocity
	
	if brain.move[1]<0 then fauna.side= 1 end
	if brain.move[1]>0 then fauna.side=-1 end

--	fauna.pos=fauna.pos+fauna.vel

	local footspeed=0.25
	local footbase=3
	local world=fauna:get_singular("kinetic").world
	local feet=kinetics.cast_feet(world,{
		origin=fauna.pos,
		points={0,0},
		radius=4,
		translation={0,6},
		filter_categoryBits=0x00010000,
		filter_maskBits=0x00ffffff,
	})
	if feet then

		local d=feet.floor -- Y distance to floor
		local o=fauna.vel[2] -- original velocity
		local v=((d-(footbase+2))) -- distance to where we want to be
		local a=v*8 -- force to adjust velocity by

		fauna.foot=d-(footbase+2)
		if fauna.foot<0  then fauna.foot=0  end
		if fauna.foot>3 then fauna.foot=3 end
		fauna.acc[2]=fauna.acc[2]+a --  hover

		fauna.onfloor=4



	else

		fauna.floor_uid=0
	
		if fauna.foot>3 then fauna.foot=3 end
		if fauna.foot<3 then fauna.foot=fauna.foot+footspeed end

		fauna.acc:add(grav) -- gravity
	end

	if fauna.onfloor>0 and fauna.jump<=0 then -- meep meep jump	
		if brain.jump then
			fauna.onfloor=0
			fauna.jump=4
			fauna.acc[2]=0
			fauna.vel:add(brain.jump) --[2]=-120
		end
	end
	if fauna.onfloor>0 then fauna.onfloor=fauna.onfloor-1 end

	if fauna.jump>0 then -- jump higher while button is held down
		fauna.onfloor=0 -- no foot grab while jumping
		fauna.jump=fauna.jump-1 -- continue jump
	end


	fauna:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
fauna_trenchs.item.draw=function(fauna)

	fauna:get_values()

	local p=V3( fauna.pos[1] , fauna.pos[2]-3, fauna.pos[1]+fauna.pos[2] )
	local f=math.abs(fauna.flap-2)
	draws.sprite{ n=fauna.sname          , p=p , sx=fauna.side }
	draws.sprite{ n=fauna.sname.."_feet" , p=p+V3(0,fauna.foot,8) , sx=fauna.side }

end

--------------------------------------------------------------------------------
--
--#fruits

fruits={}
systems[#systems+1]=fruits
-- methods added to system
fruits.system={}
-- methods added to each item
fruits.item={}

fruits.caste="fruit"

fruits.uidmap={
	length=0,
}

fruits.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
--	acc=V2( 0,0 ),
	sname="fruit_1",
	age=0,
	score=100,
	touch={},
}

fruits.types={
	pos="tween",
	rot="tween",
}


fruits.graphics={

{nil,"fruit_1",[[
. . . . . . . . 
. . Y Y Y . . . 
. Y o o o o . . 
Y o 7 7 7 o O . 
Y o 7 o o o O . 
Y o 7 7 7 o O . 
. o o o o O . . 
. . O O O . . . 
]]},

{nil,"fruit_2",[[
. . d . . G . . 
. . . d G . . . 
. . r R r R . . 
. r R r R r R . 
. R r R r R r . 
. r R r R r R . 
. . r R r R . . 
. . . r R . . . 
]]},

{nil,"fruit_3",[[
G . g . . . . . 
. G F . . . . . 
. . F F F R R . 
. . F . R r r R 
. R R . R r R R 
R r r R . R R . 
R r R R . . . . 
. R R . . . . . 
]]},

{nil,"fruit_4",[[
. . . . . . . . 
. . . Y Y Y . . 
. . Y o o o Y . 
. Y o y y y o Y 
. Y o y . . y y 
Y o y . . . . 1 
Y y y . . . . 1 
. y . . . . . . 
]]},

{nil,"fruit_5",[[
. . . . . R . . 
. . . R R 1 R . 
. . R 1 R R R . 
. R R R R 1 R . 
d R R 1 R R R . 
G G R R R 1 R . 
. d d G G R R . 
. . . d d d G . 
]]},

{nil,"fruit_6",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. Y O Y O o . . 
Y O o O o r r . 
s s F F F f f . 
o o O O O r r . 
]]},

{nil,"fruit_7",[[
. . . . . . . . 
. . . . . . . . 
. Y O Y O o . . 
Y O o O o r r . 
s s F F F f f . 
Y Y Y Y Y o o . 
s s F F F f f . 
o o O O O r r . 
]]},

{nil,"fruit_8",[[
. . I I I I . . 
. I B B B B I . 
I B d d d d B I 
B d Y Y Y Y Y B 
d Y O O O O Y d 
Y O O R R O O Y 
O O R R R R O O 
O R R . . R R O 
]]},

{nil,"fruit_9",[[
. . . . . . . . 
. R R . R R . . 
R O r R O r R . 
R r R R r R R . 
R R R R R R R . 
. R R R R R . . 
. . R R R . . . 
. . . R . . . . 
]]},


}


-- flag functions set values for use in the next update
fruits.item.flag_touch=function(fruit,touch,event)
	local oldtouch=fruit:get_value("touch")
	if oldtouch.uid==0 or oldtouch.uid==-1 then -- replace with real uid
		fruit:set_value("touch",{uid=touch and touch.uid or -1})
	end
end


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fruits.system.setup=function(sys)

	 system.components.tiles.upload_tiles( fruits.graphics )

end

fruits.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fruits.item.setup=function(fruit)
	fruit:get_values()
	
	fruit:setup_kinetic()
	fruit:set_values()
end

fruits.item.setup_kinetic_reshape=function(fruit)
	if fruit.shape then
		fruit.shape:destroy()
		fruit.shape=nil
	end
	fruit.shape=fruit.body:shape({
		shape="box",
		halfWidth=3,
		halfHeight=3,
		material_friction=1.0,
		filter="fruit",
		uid=fruit.uid,
	})
end
fruits.item.setup_kinetic=function(fruit)
	if fruit.body then return end -- already done
	local world=fruit:get_singular("kinetic").world
	fruit.body=world:body({
		type="dynamic",
		transform={fruit.pos[1],fruit.pos[2],fruit.rot*(math.pi*2)},
		gravityScale=1, -- use world gravity
	})
	fruit:setup_kinetic_reshape()
	fruit:set_body()
end

fruits.item.clean_kinetic=function(fruit)
	if not fruit.body then return end -- already done
	fruit.body:destroy()
	fruit.body=nil
	fruit.shape=nil
end

fruits.item.clean=function(fruit)
	fruit:clean_kinetic()
end


fruits.item.update=function(fruit)
	fruit:get_values()
	fruit:setup_kinetic() -- might need to recreate body

	fruit.age=fruit.age+1
	
	if fruit.touch.uid~=0 then
		local touch=scene:find_uid(fruit.touch.uid)
		fruit.touch.uid=0

		if touch and touch.caste=="player" then -- pickup
			if fruit.age>=16 then
				if fruit:mark_deleted() then
					local score=touch:get_value("score")
					touch:set_value("score",score+fruit.score)
					local boots={
						{"score",num=fruit.score,pos=fruit.pos},
					}
					scene:creates(boots)
				end
			end
		end

	end
	
	if fruit.age>=(16*10) then
		fruit:mark_deleted()
	end

	fruit:set_values()

end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
fruits.item.draw=function(fruit)

	fruit:get_values()

	local p=V3( fruit.pos[1] , fruit.pos[2], fruit.pos[1]+fruit.pos[2] )
	
	if fruit.age>=(16*7) or fruit.age<=(16*1) then -- flash when old or young
		if 1==fruit.age%2 then -- blink
			draws.sprite{ n=fruit.sname          , p=p  }
		end
	else
		draws.sprite{ n=fruit.sname          , p=p  }
	end

end

--------------------------------------------------------------------------------
--
--#gibs

gibs={}
systems[#systems+1]=gibs
-- methods added to system
gibs.system={}
-- methods added to each item
gibs.item={}

gibs.caste="gib"

gibs.uidmap={
	length=0,
}

gibs.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
--	acc=V2( 0,0 ),
	sname="gib_green",
	size=4,
	age=0,
	touch=0,
}

gibs.types={
	pos="tween",
	rot="tween",
}


gibs.graphics={


{nil,"gib_green",[[
. . g g g g . . . . . . . . . . . . . . . . . . . . . . . . . . 
. g d d G g g . . . g g g g . . . . . . . . . . . . . . . . . . 
g d d d d G g g . g g d G g g . . . . g g . . . . . . . . . . . 
g d d d d G G g . g d d d G g . . . g d G g . . . . . d G . . . 
g G d d G G G g . g G d G G g . . . g G G g . . . . . G g . . . 
g G G G G G G g . g g G G g g . . . . g g . . . . . . . . . . . 
. g g G G G g . . . g g g g . . . . . . . . . . . . . . . . . . 
. . g g g g . . . . . . . . . . . . . . . . . . . . . . . . . . 
]],4},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
gibs.system.setup=function(sys)

	 system.components.tiles.upload_tiles( gibs.graphics )

end

gibs.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
gibs.item.setup=function(gib)
	gib:get_values()
	
	gib:setup_kinetic()
	gib:set_values()
end

gibs.item.setup_kinetic_reshape=function(gib)
	if gib.shape then
		gib.shape:destroy()
		gib.shape=nil
	end
	if gib.size>0 then
		gib.shape=gib.body:shape({
			shape="circle",
			radius=gib.size,
			material_friction=1.0,
			filter="gib",
			enableHitEvents=true,
			uid=gib.uid,
		})
	end
end
gibs.item.setup_kinetic=function(gib)
	if gib.body then return end -- already done
	local world=gib:get_singular("kinetic").world
	gib.body=world:body({
		type="dynamic",
		transform={gib.pos[1],gib.pos[2],gib.rot*(math.pi*2)},
		gravityScale=1, -- use world gravity
	})
	gib:setup_kinetic_reshape()
	gib:set_body()
end

gibs.item.clean_kinetic=function(gib)
	if not gib.body then return end -- already done
	gib.body:destroy()
	gib.body=nil
	gib.shape=nil
end

gibs.item.clean=function(gib)
	gib:clean_kinetic()
end


gibs.item.update=function(gib)
	gib:get_values()
	gib:setup_kinetic() -- might need to recreate body
	
	local bounce=false

	gib.age=gib.age+1
	if gib.age>64 then bounce=true end
	

	local kinetic=gib:get_singular("kinetic")
	-- what to do when we touch
	local event_touch=function(it,event)
		if gib.age>8 then
			if not it or it.caste~="gib" then -- no self touch
				bounce=true
			end
		end
	end

	-- check events for anyone we touched
	for event in kinetic.events:iterate(gib.uid) do
		if event.is=="contact_hit" then
			local it=scene:find_uid(
				(event.shapeA.uid == gib.uid) and
				event.shapeB.uid or
				event.shapeA.uid ) -- we hit the one that is is not us
			event_touch(it,event) -- it may be nil, when hitting level
		end
	end


	if bounce then
		if gib.size>1 then
			gib.size=gib.size-1
			gib:setup_kinetic_reshape()
			gib.age=0
		else
			gib:mark_deleted()
		end
	end

	gib:set_values()

end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
gibs.item.draw=function(gib)

	gib:get_values()

	local p=V3( gib.pos[1] , gib.pos[2], gib.pos[1]+gib.pos[2] )
	draws.sprite{ n=gib.sname          , p=p , i=5-gib.size }

end


--------------------------------------------------------------------------------
--
--#junks

junks={}
systems[#systems+1]=junks
-- methods added to system
junks.system={}
-- methods added to each item
junks.item={}

junks.caste="junk"

junks.uidmap={
	held=1,
	length=1,
}

junks.values={
	pos=V2( 0,0 ),
	rot=0,
	vel=V2( 0,0 ),
	ang=0,
--	acc=V2( 0,200 ),
	sname="junk_box",
	danger=0,
}

junks.types={
	pos="tween",
	rot="tween",
}


junks.graphics={


{nil,"junk_box",[[
4 4 4 4 4 4 4 4 
4 3 3 3 3 3 3 4 
4 3 4 4 4 4 3 4 
4 3 3 4 4 3 3 4 
4 3 4 4 4 4 3 4 
4 3 4 3 3 4 3 4 
4 3 3 3 3 3 3 4 
4 4 4 4 4 4 4 4 
]]},

{nil,"junk_box_danger",[[
4 4 4 4 4 4 4 4 
4 3 3 3 3 3 3 4 
4 3 Y Y Y Y 3 4 
4 3 3 Y Y 3 3 4 
4 3 Y Y Y Y 3 4 
4 3 Y 3 3 Y 3 4 
4 3 3 3 3 3 3 4 
4 4 4 4 4 4 4 4 
]]},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
junks.system.setup=function(sys)

	 system.components.tiles.upload_tiles( junks.graphics )

end

junks.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
junks.item.setup=function(junk)
	junk:get_values()
	
	junk:setup_kinetic()
	junk:set_values()
end

junks.item.setup_kinetic_reshape=function(junk)
	local held=junk:depend("held")
	local danger=junk:get_value("danger")

	if junk.shape then
		junk.shape:destroy()
		junk.shape=nil
	end
	junk.shape=junk.body:shape({
		shape="box",
		halfWidth=4,
		halfHeight=4,
		material_friction=1.0,
		filter=( held or danger>0 ) and "player" or "junk",
		enableHitEvents=true,
		uid=junk.uid,
	})
end
junks.item.setup_kinetic=function(junk)
	if junk.body then return end -- already done
	local world=junk:get_singular("kinetic").world
	junk.body=world:body({
		type="dynamic",
		transform={junk.pos[1],junk.pos[2],junk.rot*(math.pi*2)},
		isBullet=true, -- we will throw junk at stuff
		gravityScale=1, -- use world gravity
 	})
	junk:setup_kinetic_reshape()
	junk:set_body()
end

junks.item.clean_kinetic=function(junk)
	if not junk.body then return end -- already done
	junk.body:destroy()
	junk.body=nil
	junk.shape=nil
end

junks.item.clean=function(junk)
	junk:clean_kinetic()
end


junks.item.update=function(junk)
	junk:get_values()
	junk:setup_kinetic() -- might need to recreate body
	
	if junk.danger>0 then -- limit danger time
		junk.danger=junk.danger-1
		local len=junk.vel:len()
		if len<2 then -- limit if stopped moving
			junk.danger=0
			junk:set_value("danger",0)
			junk:setup_kinetic_reshape()
		end
	end

	local kinetic=junk:get_singular("kinetic")
	-- what to do when we touch
	local event_touch=function(it,event)

		if it.do_stun and junk.danger>0 then -- can stun
			it:do_stun(junk,event)
		end

	end
	-- check events for anyone we touched
	for event in kinetic.events:iterate(junk.uid) do
		if event.is=="contact_hit" then
			local it=scene:find_uid(
				(event.shapeA.uid == junk.uid) and
				event.shapeB.uid or
				event.shapeA.uid ) -- we hit the one that is is not us
			if it then
				event_touch(it,event)
			end
		end
	end
	
	junk:set_values()
end

-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
junks.item.draw=function(junk)

	junk:get_values()

	local p=V3( junk.pos[1] , junk.pos[2], junk.pos[1]+junk.pos[2] )
	
	if junk.danger>0 then
		draws.sprite{ n=junk.sname.."_danger" , p=p , rz=junk.rot , }
	else
		draws.sprite{ n=junk.sname          , p=p , rz=junk.rot , }
	end

end

--------------------------------------------------------------------------------
--
--#fauna_pandas

fauna_pandas={}
systems[#systems+1]=fauna_pandas
-- methods added to system
fauna_pandas.system={}
-- methods added to each item
fauna_pandas.item={}

fauna_pandas.caste="fauna_panda"

fauna_pandas.uidmap={
	length=0,
}

fauna_pandas.values={
	pos=V2( 0,0 ),
	sname="",
	thunk=0,
}

fauna_pandas.types={
	pos="tween",
}


fauna_pandas.graphics={

{nil,"fauna_panda",[[
. . 0 0 0 0 . . . . . . . . . . . 0 0 0 0 . . . 
. 0 0 1 1 0 0 . . 6 6 6 6 6 . . 0 0 1 1 0 0 . . 
. 0 1 0 0 0 0 6 6 6 6 6 6 6 6 6 0 0 0 0 1 0 . . 
. 0 1 0 0 0 6 6 6 6 6 6 6 6 6 6 6 0 0 0 1 0 . . 
. 0 0 0 0 6 6 0 0 0 6 6 6 0 0 0 6 6 0 0 0 0 . . 
. . 0 0 6 6 0 0 6 0 0 6 0 0 6 0 0 6 6 0 0 . . . 
. . . . 6 6 0 0 0 0 6 6 6 0 0 0 0 6 6 . . . . . 
. . . 6 6 6 6 0 0 6 6 6 6 6 0 0 6 6 6 6 . . . . 
. . . 6 6 6 6 6 6 6 0 0 0 6 6 6 6 6 6 6 . . . . 
. . . 6 6 6 6 6 6 6 6 0 6 6 6 6 6 6 6 6 . . . . 
. . . . 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 . . . . . 
. . . . 6 6 6 0 6 6 0 0 0 6 6 0 6 6 6 . . . . . 
. . . . . 6 6 6 0 0 6 6 6 0 0 6 6 6 . . . . . . 
. . . . . . 0 6 6 6 6 6 6 6 6 6 0 . . . . . . . 
. . . . . 0 0 0 0 6 6 6 6 6 0 0 0 0 . . . . . . 
. . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . 
. . . 0 0 0 0 0 0 0 6 6 6 0 0 0 0 0 0 0 . . . . 
. . 0 1 0 0 0 0 6 6 6 6 6 6 6 0 0 0 0 1 0 . . . 
. . 0 1 1 0 . 6 6 6 0 6 0 6 6 6 . 0 1 1 0 . . . 
. . . 0 0 . . 6 6 6 6 0 6 6 6 6 . . 0 0 . . . . 
. . . . . . . 0 0 0 6 0 6 0 0 0 . . . . . . . . 
. . . . . . 0 1 0 1 0 . 0 1 0 1 0 . . . . . . . 
. . . . . . 0 0 1 0 0 . 0 0 1 0 0 . . . . . . . 
. . . . . . . 0 0 0 . . . 0 0 0 . . . . . . . . 
]]},

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
fauna_pandas.system.setup=function(sys)

	 system.components.tiles.upload_tiles( fauna_pandas.graphics )

end

fauna_pandas.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
fauna_pandas.item.setup=function(fauna_panda)
	fauna_panda:get_values()

	fauna_panda:set_values()
end

fauna_pandas.item.update_brain=function(fauna_panda,brain)

	fauna_panda.thunk=fauna_panda.thunk-1
	if fauna_panda.thunk<=0 then
		fauna_panda.thunk=fauna_panda.sys:get_rnd(16*8,16*16)
		scene:creates({
			{
				"fauna_egg",
				pos={fauna_panda.pos[1],fauna_panda.pos[2],0},
				vel={fauna_panda.sys:get_rnd(-20,20),0},
			},
		})
	end


end

fauna_pandas.item.update=function(fauna_panda)
	fauna_panda:get_values()
	
	local brain={}
	fauna_panda:update_brain(brain)
	
	fauna_panda:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
fauna_pandas.item.draw=function(fauna_panda)

	fauna_panda:get_values()

	local p=V3( fauna_panda.pos[1] , fauna_panda.pos[2]-3, fauna_panda.pos[1]+fauna_panda.pos[2] )
	draws.sprite{ n=fauna_panda.sname          , p=p }

end

--------------------------------------------------------------------------------
--
--#levels

levels={}
systems[#systems+1]=levels
-- methods added to system
levels.system={}
-- methods added to each item
levels.item={}

levels.caste="level"

levels.uidmap={
	length=0,
}

levels.values={
	pos=V2( 0,0 ),
	focus=V2( 0,0 ),
	idx=1,
	time=0,
	complete=0,
	start=0,
}

levels.types={
	pos="tween",
	focus="tween",
	time="tween",
}


levels.graphics={

{nil,"char_empty",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},

{nil,"char_black",[[
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 
]]},

{nil,"char_solid",[[
r r r r r r r R 
r r r r r r r R 
r r r r r r r R 
R R R R R R R R 
r r r R r r r r 
r r r R r r r r 
r r r R r r r r 
R R R R R R R R 
]]},

{nil,"char_sign",[[
. . . F f . . . 
s s s s s s s s 
s 0 1 0 1 0 1 s 
s 1 0 1 0 1 0 s 
s s s s s s s s 
. . . F f . . . 
. . . F f . . . 
. . . F f . . . 
]]},


}

levels.combine_legends=function(...)
	local legend={}
	for _,t in ipairs{...} do -- merge all
		for n,v in pairs(t) do -- shallow copy, right side values overwrite left
			legend[n]=v
		end
	end
	return legend
end

levels.legend={
	[0]={ name="char_empty",	},

	[". "]={ name="char_empty",				},
	["< "]={ name="char_empty",				dir_left=1,  },
	["> "]={ name="char_empty",				dir_right=1, },
	["^ "]={ name="char_empty",				dir_up=1,    },
	["v "]={ name="char_empty",				dir_down=1,  },
	["00"]={ name="char_black",				solid=1, dense=1, },		-- black border
	["0 "]={ name="char_solid",				solid=1, dense=1, },		-- empty border
	["P1"]={ spawn="player", player=1,  },
	["P2"]={ spawn="player", player=2,  },
	["S1"]={ spawn="egg", egg="slim", },
	["J1"]={ spawn="junk", junk="box", },
}

levels.infos={}

levels.infos[1]={
legend=levels.combine_legends(levels.legend,{
	["Ta"]={ name="char_sign",				text="Welcome to the Dungeon, we got fun and games." },
	["Tb"]={ name="char_sign",				text="We got everything you want, honey, we got the Memes." },
	["Tc"]={ name="char_sign",				text="Congratulations on the coyote JUMP." },
	["Td"]={ name="char_sign",				text="You may coyote JUMP in the air after walking off a platform." },
	["T1"]={ name="char_sign",				text="Press Button A or . to JUMP in." },
	["T2"]={ name="char_sign",				text="Use Left Stick or WASD to move." },
	["T3"]={ name="char_sign",				text="MOVE down to crouch down." },
	["T4"]={ name="char_sign",				text="Press Button B or / to GRAB object, press GRAB again to throw it." },
	["T5"]={ name="char_sign",				text="Throw power is shown by rotation speed, run for it to speed up." },
	["T6"]={ name="char_sign",				text="Aim throw with Right Stick or Cursor Keys." },
	["T7"]={ name="char_sign",				text="Throw object at Slim Slimy to stun him." },
	["T8"]={ name="char_sign",				text="Stomp stunned Slim Slimy to finish him." },
}),
title="Test.",
map=[[
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 J1. . . Tb. . . Ta. . . . . . . . . . . > . . . . . . . . . . . . . . . . . . . . v . . . . 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . 0 0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 > . . . . . . . . . . . . . ^ . . . 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 J1. Tc. . . . . . . . . . . . . . . 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 0 0 0 0 0 . . . . . . . . . . . . . 0 . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . Td. . . . 0 . . . . . . . . . . . . . . . . . . 0 . . . < . . . . 0 
0 . . . . . . . . . . . . 0 0 0 . . . 0 . . . . . . . . . . . . . . . . . . 0 . . . . . . . , 0 
0 P1P2. . . . . . . . . . 0 0 0 . . . 0 . ^ . . . J1. . . . . . . . . . . . 0 . . . ^ . . . . 0 
0 . T2. . . . . . . . T1. 0 0 0 . T3. . . . T4. . 0 . . T5. . T6. . T7. . T80 . . . . . . . S10 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
]],
}

levels.infos[2]={
legend=levels.combine_legends(levels.legend,{
}),
title="Test.",
map=[[
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 . . . . . > v . . . . . . . . . . . . . . . . v < . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . J1. . . . . . . . . . . . . . . . . . J1. . . . . 0 
0 0 0 0 0 0 0 . . . . . . . . . . . . . . . . . . 0 0 0 0 0 0 0 
0 . . . J1. . . . . . . . . . . . . . . . . . . . . . J1. . . 0 
0 . . . . . . . . . . . . . S1. . S1. . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . 0 0 0 0 0 0 . . . . . . . . . . . . 0 
0 . . . . . . . . . . > ^ < . . . . > ^ < . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . S1. . . . . . . . . . S1. . . . . . . . . 0 
0 . . . . . . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . . . . 0 
0 . . . . . . > ^ < . . . . . . . . . . . . > ^ < . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . S1. . . . . . . . . . . . . . . . . . S1. . . . . 0 
0 . . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . 0 
0 ^ . > ^ < . . . . . . . . . . . . . . . . . . . . > ^ < . . 0 
0 P1. . . . . . . . . . . . . . . . . . . . . . . . . . . P2. 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
]],
}

levels.infos[3]={
legend=levels.combine_legends(levels.legend,{
}),
title="Test.",
map=[[
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 > . . . . . . . . . . . . , . . . . . . . . . . . . . . . < 0 
0 P1. . . . . . . . . . . . . . . . . . . . . . . . . . . P2. 0 
0 . . . . . . . . . > . . . . . . . . . . < . . . . . . . . . 0 
0 0 0 0 0 0 0 . . . . . . . . . . . . . . . . . . 0 0 0 0 0 0 0 
0 . . . . . . . . . . . . . . v v . . . . . . . . . . . . . . 0 
0 . . . . . . . S10 0 . . . . . . . . . . 0 0 S1. . . . . . . 0 
0 . . . . . < 0 0 0 0 0 0 . J1. . J1. 0 0 0 0 0 0 > . . . . . 0 
0 . . . . . . . . . . . . . S1. . S1. . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . J1< > J1. . . . . . . . . . . . . 0 
0 . . ^ . . . . . . . . . . S1. . S1. . . . . . . . . . ^ . . 0 
0 . . . . . . . . . . . . 0 0 0 0 0 0 . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 S1. . . . . . . . . . . . . < > . . . . . . . . . . . . . S10 
0 0 0 0 0 0 0 0 0 0 0 0 0 . . . . . . 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 . . . . . . . . . . J1. . . . . . . . J1. . . . . . . . . . 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 > . . . . > . . . . . > . . . . . . < . . . . . < . . . . < 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
]],
}

levels.infos[4]={
legend=levels.combine_legends(levels.legend,{
}),
title="Test.",
map=[[
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . J1. . . . . 0 
0 . . v . . . . . . . . . . . < . . . . . . . . . < . . . . . 0 
0 S1. . . . . . . . . . . . . . . . . . . . . . . . . . . . S10 
0 0 . . . . . . . . . . . . . . . . . . . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . S10 
0 . . . . J1. . . T1. . . . T2. . . T3. . . . . . . . . . . S10 
0 . . . 0 0 0 0 . 0 0 0 0 . 0 0 . 0 0 0 . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 S1. . . . . . . . . . . . . . . . . . . . . . . . . . . . S10 
0 0 . v . . . . . . . . . . . . . . S1. . . . . . ^ . . 0 0 0 0 
0 P1. . . . . . . . . . . . . . . 0 0 . . . . . . . . . . . P20 
0 . . . . J1. . . . . . . . . . S10 . . . . . . . . . . . . S10 
0 . . . 0 0 0 0 . . . . . . . . 0 0 . . . . . . . . . . 0 0 0 0 
0 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 0 
0 S1. . . . . . . . . . . . . . 0 0 0 . . . . . . . . . . . S10 
0 0 0 0 . . . . . . J1. . . . . . . . . . . 0 0 0 . . . 0 0 0 0 
0 . . > . . . . . 0 0 0 . . . > . . . . . . . . . . ^ . . . . 0 
0 . . . . . . . . . . . . . . . S1. . . . . . . . . . . . . . 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
]],
}

-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
levels.system.setup=function(sys)

	 system.components.tiles.upload_tiles( levels.graphics )
	 
end

levels.system.clean=function(sys)
end

levels.item.clean=function(level)
	if level.static_body then
		level.static_body:destroy()
		level.static_body=nil
	end
end
-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
levels.item.setup=function(level)
	level:get_values()

	local names=system.components.tiles.names
	local world=level:get_singular("kinetic").world
	local kinetic=level:get_singular("kinetic")
	level.static_body=world:body({
		type="static",
	})
	
	local info=levels.infos[level.idx]

	local tilemap={}
	for n,v in pairs( info.legend ) do -- build tilemap from legend
		if v.name then -- convert name to tile
			tilemap[n]=names[v.name]
		end
	end
	local hx,hy=bitdown.pix_size(info.map)
--print("mapsize",hx,hy)
	system.components.map.autosize=nil -- turn off autosize
	system.components.map.screen_resize(hx*8,hy*8)
	system.components.map.tilemap_grd:clear(0)
	system.components.map.dirty(true)

	bitdown.tile_grd( info.map, tilemap, system.components.map.tilemap_grd  ) -- draw into the screen (tiles)

	local map=bitdown.pix_tiles(  info.map,  info.legend )
	level.map=map
	level.spawns={} -- find spawn tiles easily
	local get_tile=function(x,y,name)
		local t=map[y] and map[y][x]
		if name then return t and t[name] else return t end
	end
	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			if tile.spawn and tile.idx then
				level.spawns[tile.spawn.."_"..tile.idx]=tile
			elseif tile.spawn then
				level.spawns[tile.spawn]=tile
			end
			local shape
			if tile.solid then -- merge outside edges of solid cells to create smoother collisions
				if (not get_tile(x,y-1,"solid") or not get_tile(x,y+1,"solid") ) then -- try to drag across
					local otile=get_tile(x-1,y)
					if otile and otile.shape and otile.solid==tile.solid then -- drag across
						if otile.shape[4]-otile.shape[2] == 8 then -- check size
							shape=otile.shape
							shape[3]=shape[3]+8
						end
					end
				elseif (not get_tile(x-1,y,"solid") or not get_tile(x+1,y,"solid") ) then -- try to drag down
					local otile=get_tile(x,y-1)
					if otile and otile.shape and otile.solid==tile.solid then -- drag across
						if otile.shape[3]-otile.shape[1] == 8 then -- check size
							shape=otile.shape
							shape[4]=shape[4]+8
						end
					end
				end
				if not shape then -- start new shape
					shape={x*8,y*8,(x+1)*8,(y+1)*8}
				end
				tile.shape=shape
			end
		end
	end

	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			local shape=tile.shape
			tile.shape=nil
			if shape and shape[1]==x*8 and shape[2]==y*8 then -- shape must start at this cell
--print("shape",unpack(shape))
				level.static_body:shape({
					shape="box",
					halfWidth=(shape[3]-shape[1])/2,
					halfHeight=(shape[4]-shape[2])/2,
					center={(shape[3]+shape[1])/2,(shape[4]+shape[2])/2},
					material_friction=tile.solid*0.5,
					material_restitution=tile.solid*0.5,
				})
			end
			if tile.text then
				tile.text_lines=wstr.smart_wrap(tile.text,(256-16)/8)
			end
			tile.dir_left  = tile.dir_left  or math.huge
			tile.dir_right = tile.dir_right or math.huge
			tile.dir_up    = tile.dir_up    or math.huge
			tile.dir_down  = tile.dir_down  or math.huge
		end
	end

	local get_side_tiles=function(x,y)
		local stiles={}
		local push=function(t)
			if not t then return end
			stiles[#stiles+1]=t
		end
		push(map[y-1] and map[y-1][x])
		push(map[y]   and map[y][x-1])
		push(map[y]   and map[y][x+1])
		push(map[y+1] and map[y+1][x])
		return stiles
	end

	repeat
		local done=true
		for y,line in pairs(map) do
			for x,tile in pairs(line) do
				for _,side in ipairs( get_side_tiles(x,y) ) do
					for _,name in ipairs( { "dir_left" , "dir_right" , "dir_up" , "dir_down" } ) do
						if side[name]>tile[name]+1 then
							side[name]=tile[name]+1
							done=false
						end
					end
				end
			end
		end
	until done

	for y,line in pairs(map) do
		for x,tile in pairs(line) do
			-- calc the wind direction normal for this tile
			tile.dir=V2(
					(1/tile.dir_right)-(1/tile.dir_left),
					(1/tile.dir_down)-(1/tile.dir_up)
				):normalize()
				
			local pos={ (tile.x or 0)*8+4 , (tile.y or 0)*8+4 , 0 }
			if tile.spawn=="egg" then -- spawn an egg
				scene:creates({
					{
						"fauna_egg",
						egg=tile.egg,
						pos=pos,
					},
				})
			elseif tile.spawn=="junk" then -- spawn junk
				scene:creates({
					{
						"junk",
						junk=tile.junk,
						pos=pos,
					},
				})
			elseif tile.spawn=="player" then -- spawn players
				scene:creates({
					{"hud",idx=tile.player,depends={player=3}},
					{"camera",idx=tile.player,depends={player=3}},
					{"player",idx=tile.player,pos=pos,spawn=pos,depends={camera=2,hud=1},mode="spawn",},
				})
			end
		end
	end

-- set background color
	local it=system.components.copper
	it.shader_name="fun_copper_back_y5"
	it.shader_uniforms.cy0={ 0.25 , 0    , 0.25 , 1   }
	it.shader_uniforms.cy1={ 0.125, 0    , 0.25 , 1   }
	it.shader_uniforms.cy2={ 0.125, 0.125, 0.25 , 1   }
	it.shader_uniforms.cy3={ 0    , 0.125, 0.25 , 1   }
	it.shader_uniforms.cy4={ 0    , 0.25 , 0.25 , 1   }

	it.shader_uniforms.cy0={ 1/2 , 0   , 1/2 , 1   }
	it.shader_uniforms.cy1={ 1/4 , 0   , 1/2 , 1   }
	it.shader_uniforms.cy2={ 1/4 , 1/4 , 1/2 , 1   }
	it.shader_uniforms.cy3={ 0   , 1/4 , 1/2 , 1   }
	it.shader_uniforms.cy4={ 0   , 1/2 , 1/2 , 1   }

	level.time=0
	level.start=0

	level:set_values()
end

levels.item.update=function(level)
	level:get_values()

	level.time=level.time+(1/16) -- yes we are updating at 16fps, but can draw at 60 or whatevs.
	

	local fauna=0
	local lists={
		scene:caste("fauna_egg"),
		scene:caste("fauna_slim"),
		scene:caste("fruit"),
	}
	for _,list in ipairs(lists) do
		for i,it in ipairs(list) do
			if not it:get("deleted") then -- live
				fauna=fauna+1
				break
			end
		end
	end
	if fauna==0 then -- all mosters dead
		level.complete=level.complete+1
	end
	
	level.start=level.start+1
	
	if level.complete>(64+16) then -- delete everything and restart
	
		local idx=level.idx+1
		if not levels.infos[idx] then idx=1 end
		scene:do_level(idx)
		return
		
	end


	level:set_values()
end

levels.item.get_wind=function(level,pos)
	local t=level:get_tile_by_pos(pos)
	return V2( t and t.dir )
end

levels.item.get_gravity=function(level,pos,vel)
	if vel[2]>16 then return V2(0,600) end -- fall faster
	return V2(0,400)
end

levels.item.get_tile_by_idx=function(level,idx,name)
	local x=idx%0x100
	local y=math.floor(idx/0x100)%0x100
	return level:get_tile_by_xy(x,y,name)
end

levels.item.get_tile_by_xy=function(level,x,y,name)
	local t=level.map[y] and level.map[y][x]
	if name then return t and t[name] else return t end
end


levels.item.get_tile_by_pos=function(level,pos)
	local x=math.floor((pos[1])/8)
	local y=math.floor((pos[2])/8)
	return level:get_tile_by_xy(x,y)
end

levels.item.each_tile_near=function(level,pos,near)

--	local done={}
	local list={}
	local push=function(t)
		if not t then return end
--		if done[t] then return end
--		done[t]=true
		list[#list+1]=t
	end

	local bx=math.floor((pos[1])/8)
	local by=math.floor((pos[2])/8)

	push( level:get_tile_by_xy(bx,by) )

	for n=1,near do
		for x=-n,n,1 do
			push( level:get_tile_by_xy(bx+x,by-n) )
			push( level:get_tile_by_xy(bx+x,by+n) )
		end
		for y=-n+1,n-1,1 do
			push( level:get_tile_by_xy(bx-n,by+y) )
			push( level:get_tile_by_xy(bx+n,by+y) )
		end
	end

	return ipairs(list)
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
levels.item.draw=function(level)

end

--------------------------------------------------------------------------------
--
--#cameras

cameras={}
systems[#systems+1]=cameras
-- methods added to system
cameras.system={}
-- methods added to each item
cameras.item={}

cameras.caste="camera"

cameras.uidmap={
	player=1,
	length=1,
}

cameras.values={
	pos=V2( 0,0 ),
	focus=V2( 0,0 ),
	slide=V2( 0,-(32*32) ),
	idx=1,
	tile=0x0000,
}

cameras.types={
	pos="tween",
	slide="tween",
	focus="tween",
}


cameras.graphics={

}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
cameras.system.setup=function(sys)

	 system.components.tiles.upload_tiles( cameras.graphics )

end

cameras.system.clean=function(sys)
end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
cameras.item.setup=function(camera)
	camera:get_values()

	camera:set_values()
end

cameras.item.update=function(camera)
	if camera.idx~=1 then return end -- only 1 hud

	camera:get_values()
	
	local level=camera:get_singular("level") -- only one level is active at a time
	level:get_values()
	

	local player=camera:depend("player")
	local hud=player:depend("hud")

	camera.focus:mix(player.pos+player.vel,1/16) -- smooth ?
	
	local map=system.components.map
	local screen=system.components.screen

	if map.window_hx<=screen.hx then -- center
		camera.pos[1]=-(screen.hx-map.window_hx)/2
	else
		camera.pos[1]=camera.focus[1]-( screen.hx/2 )
		if camera.pos[1]<0 then camera.pos[1]=0 end
		local m=map.window_hx-screen.hx
		if camera.pos[1]>m then camera.pos[1]=m end
	end

	local shy=(screen.hy-hud.pos[2])
	if map.window_hy<=shy then -- center
		camera.pos[2]=-(shy-map.window_hy)/2
	else
		camera.pos[2]=camera.focus[2]-( shy/2 )
		if camera.pos[2]<0 then camera.pos[2]=0 end
		local m=map.window_hy-(shy-hud.pos[1])
		if camera.pos[2]>m then camera.pos[2]=m end
	end

	camera.slide[2]=0
	if level.start<16 then
		local t=(16-level.start)
		camera.slide[2]=-(t*t)
	end

	if level.complete>0 then
		if level.complete>64 then
			local t=(level.complete-64)
			camera.slide[2]=t*t
		end
	end



	camera:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
cameras.item.draw=function(camera)
	if camera.idx~=1 then return end -- only 1 hud

	local map=system.components.map
	local screen=system.components.screen
	local level=camera:get_singular("level") -- only one level is active at a time

	camera:get_values()

	if camera.pos[1]<0 then
		map.window_px=math.floor(-camera.pos[1])
		map.px=0
	else
		map.window_px=0
		map.px=math.floor(camera.pos[1])
	end
	
	if camera.pos[2]<0 then
		map.window_py=math.floor(-camera.pos[2])
		map.py=0
	else
		map.window_py=0
		map.py=math.floor(camera.pos[2])
	end

	map.window_px=map.window_px+math.floor(camera.slide[1])
	map.window_py=map.window_py+math.floor(camera.slide[2])

end

--------------------------------------------------------------------------------
--
--#huds

huds={}
systems[#systems+1]=huds
-- methods added to system
huds.system={}
-- methods added to each item
huds.item={}

huds.caste="hud"

huds.uidmap={
	player=1,
	length=1,
}

huds.values={
	pos=V2( 0,0 ),
	focus=V2( 0,0 ),
	idx=1,
	dst=V2( 0,0 ),
	tile_idx=0,
	tile_time=0,
}

huds.types={
	pos="tween",
	dst="tween",
	focus="tween",
}


huds.graphics={

{nil,"char_life",[[
. . . . . . . . 
. . . 1 1 . . . 
. . . 1 1 . . . 
. . . . . . . . 
. . 1 . . 1 . . 
. . . 1 1 . . . 
. . . 1 1 . . . 
. . 1 . . 1 . . 
]]},


}


-- the system has no state values but can still perform generic actions
-- eg allocate shared resources for later use
huds.system.setup=function(sys)

	 system.components.tiles.upload_tiles( huds.graphics )

end

huds.system.clean=function(sys)
end

huds.system.draw=function(sys)

    local ctext=system.components.text
    local bg=24
	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color

end

-- state values are cached into the item for easy access on a get
-- and must be set again if they are altered so setup and updates
-- must begin and end with a get and a set
huds.item.setup=function(hud)
	hud:get_values()

	hud:set_values()
end

local TEXT_DELAY_WAIT=48
huds.item.update=function(hud)
	if hud.idx~=1 then return end -- only 1 hud

	hud:get_values()
	
	local player=hud:depend("player")
	
	local level=hud:get_singular("level") -- only one level is active at a time
	
	local show_tile
	local v=player:get("vel")
		for _,t in level:each_tile_near( player.pos + V2(0,0), 1 ) do
			if t.name=="char_sign" then
				show_tile=t
				break
			end
		end

	if show_tile and hud.tile_idx~=show_tile.idx then -- new text
		hud.tile_idx=show_tile.idx
		if hud.tile_time>TEXT_DELAY_WAIT then
			hud.tile_time=TEXT_DELAY_WAIT
		end
	end

	if show_tile and player.idle>0 then
		hud.tile_time=hud.tile_time+1
		if hud.tile_time > #show_tile.text_lines*32+TEXT_DELAY_WAIT then
			hud.tile_time = #show_tile.text_lines*32+TEXT_DELAY_WAIT
		end
	else
		hud.tile_time=hud.tile_time-2
		if hud.tile_time<0 then hud.tile_time=0 end
	end

-- 1 sec pause
	if hud.tile_time>TEXT_DELAY_WAIT and show_tile then
		hud.dst=V2(0,#show_tile.text_lines*16+16)
	else
		hud.dst=V2(0,0)
	end


	hud.pos=(hud.dst+hud.pos*7)/8

	hud:set_values()
end


-- when drawing get will auto tween values
-- so it can be called multiple times between updates for different results
huds.item.draw=function(hud)
	if hud.idx~=1 then return end -- only 1 hud


	local text=system.components.text
	local screen=system.components.screen
	local level=hud:get_singular("level") -- only one level is active at a time


	hud:get_values()

	text.window_py=screen.hy-hud.pos[2]
	text.py=0

    local bg=24
--	text.text_clear(0x01000000*bg) -- clear text forcing a background color

--	text.text_print("This is the hud, 4 lives and 10 secs",0,0,31,24) -- (text,x,y,color,background)
--	text.text_print2("This is the hud, 4 lives and 10 secs",0,1,31,24) -- (text,x,y,color,background)

	if hud.tile_idx>=0 then
		local maxchar=math.floor((hud.tile_time-TEXT_DELAY_WAIT)/1)
		
		local tile=level:get_tile_by_idx(hud.tile_idx)
		if tile and tile.text_lines then
			for i,line in ipairs(tile.text_lines) do
				if maxchar>0 then
					local s=line:sub(1,maxchar)
					maxchar=maxchar-#s
					text.text_print4(s,2,i*2-1,31,24) -- (text,x,y,color,background)
				end
			end
		end
	end



	local tf=math.floor(level.time*100)%100
	local ts=math.floor(level.time)%60
	local tm=math.floor(level.time/60)
	local s=tm..":"..("0"..ts):sub(-2)
	draws.string16(s,128-(#s*4),0)

	local players=scene:caste("player")
	for i,p in ipairs(players) do
		if p.idx==1 then
			local s=draws.integer_to_string_with_commas(p.score)
--			if p.onfloor>0 then s=s.." _" end
			draws.string4(s,2,1)
		elseif p.idx==2 then
			local s=draws.integer_to_string_with_commas(p.score)
--			if p.onfloor>0 then s="_ "..s end
			draws.string4(s,254-(#s*4),1)
		end
	end

end

--#
--------------------------------------------------------------------------------

-- lock globals to help catch future accidents
global=require("global").__newindex_create_meta_lock(_G)
global.__newindex_lock()
