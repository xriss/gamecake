--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

local tardis=require("wetgenes.tardis")
local V0,V1,V2,V3,V4,M2,M3,M4,Q4=tardis:export("V0","V1","V2","V3","V4","M2","M3","M4","Q4")


oven.opts.fun="" -- back to menu on reset

sysopts={
	mode="swordstone", -- select a characters+sprites on a 256x128 screen using the swanky32 palette.
	lox=256,loy=128, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
	update=function() update() end, -- call global update
	draw=function() draw() end, -- call global draw
}

hardware,main=system.configurator(sysopts)

all=require("wetgenes.gamecake.zone.flat.all"):import()

all.create_scene=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or require("wetgenes.gamecake.zone.scene").create()
	scene.create=create_scene
	
	scene.require_search={
		"wetgenes.gamecake.zone.flat.",
		"",
	}
	for _,it in pairs({ -- static data and functions for each system
		all,
		tests,
	}) do
		scene.infos[it.caste]=it
	end

	scene.do_clean=function(scene)
		scene:systems_cocall("clean")
		scene:call("destroy")
		scene:reset()
--		scene.infos.all.scene.initialize(scene)
	end
	
	scene.do_setup=function(scene)
		scene:systems_cocall("setup")

		local boots={
			{"test"},
		}
		scene:creates(boots)
	end

	scene.infos.all.scene.initialize(scene)

	return scene

end

setup=function()
	scene=all.create_scene()
	scene:do_setup()
end

update=function()
	if setup then setup=setup() end -- call setup once
	scene:do_update()
end

draw=function()
	scene:do_draw()
end


tests={}
-- methods added to system
tests.system={}
-- methods added to each item
tests.item={}

tests.caste="test"

tests.uidmap={
	length=0,
}

tests.values={
	pos=V3( 0,0,0 ),
	rot=Q4( 0,0,0,1 ),
	vel=V3( 0,0,0 ),
	ang=V3( 0,0,0 ),
}

tests.types={
	pos="get",
	rot="get",
	vel="get",
	ang="get",
}


tests.item.get_values=function(test)

	test:get_auto_values()
--	test:get_body_values()

end

tests.item.set_values=function(test)

	test:set_auto_values()
--	test:set_body_values()

end


tests.system.setup=function(sys)
end

tests.system.clean=function(sys)
end

tests.system.draw=function(sys)
    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=9
	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
end

tests.item.setup=function(test)
	test:get_values()

	test:set_values()
end

tests.item.update=function(test)
	test:get_values()

    local cscreen=system.components.screen

	local tx=math.ceil(cscreen.hx/4)
	local ty=math.ceil(cscreen.hy/8)
	local s="Hello World!"
	local sx=#s
	
--	speed=speed+1
--	if speed>=60 then
		test.pos[1]=test.pos[1]+test.vel[1]
		test.pos[2]=test.pos[2]+test.vel[2]
--		speed=0
--	end
	
	if test.pos[1]<=0     then test.pos[1]=0     ; test.vel[1]= 2/16 end
	if test.pos[2]<=0     then test.pos[2]=0     ; test.vel[2]= 1/16 end
	if test.pos[1]>=tx-sx then test.pos[1]=tx-sx ; test.vel[1]=-2/16 end
	if test.pos[2]>=ty-1  then test.pos[2]=ty-1  ; test.vel[2]=-1/16 end


	test:set_values()
end

tests.item.draw=function(test)

	test:get_values()


    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=9
    local fg=system.ticks%32 -- cycle the foreground color

	local px=math.floor(0.5+test.pos[1])
	local py=math.floor(0.5+test.pos[2])

	ctext.text_print("Hello World!",px,py,fg,bg) -- (text,x,y,color,background)

end

