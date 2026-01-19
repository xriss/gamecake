--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

oven.opts.fun="" -- back to menu on reset

sysopts={
	mode="swordstone", -- select a characters+sprites on a 256x128 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
	lox=256,loy=128, -- minimum size
	hix=256,hiy=256, -- maximum size
	autosize="lohi", -- flag that we want to auto resize
}

hardware,main=system.configurator(sysopts)

-- create global all
all=require("wetgenes.gamecake.zone.flat.all"):import()
all.create_scene=function(scene)

	-- a scene is a bunch of systems and items
	scene=scene or require("wetgenes.gamecake.zone.scene").create()
	
	scene.require_search={
		"wetgenes.gamecake.zone.flat.",
		"",
	}
	for _,it in pairs({ -- static data and functions for each system
		all,
--		scene:require("test"),
	}) do
		scene.infos[it.caste]=it
	end
	
	scene.full_clean=function(scene)
		scene:systems_cocall("clean")
		scene:call("destroy")
		scene:reset()
	end
	
	scene.full_setup=function(scene)
		scene:systems_cocall("setup")
	end

	return scene

end

-- create global scene
scene=all.create_scene()




-- old test code

local px,py=0,0
local vx,vy=2,1
local speed=0

update=function()

    if setup then setup() setup=nil end

    local cscreen=system.components.screen
    local ctext=system.components.text
    local bg=9
    local fg=system.ticks%32 -- cycle the foreground color

	ctext.text_clear(0x01000000*bg) -- clear text forcing a background color
	
	local tx=math.ceil(cscreen.hx/4)
	local ty=math.ceil(cscreen.hy/8)
	local s="Hello World!"
	local sx=#s
	
	speed=speed+1
	if speed>=60 then
		px=px+vx
		py=py+vy
		speed=0
	end
	
	if px<=0 then px=0 ; vx=2 end
	if py<=0 then py=0 ; vy=1 end
	if px>=tx-sx then px=tx-sx ; vx=-2 end
	if py>=ty-1  then py=ty-1  ; vy=-1 end
	
	fg=1+((px+py)%31)
	if fg==bg then fg=bg+1 end
	
	ctext.text_print("Hello World!",px,py,fg,bg) -- (text,x,y,color,background)

end
