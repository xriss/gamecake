--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local wzips=require("wetgenes.zips")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local snames=require("wetgenes.gamecake.spew.names")

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local t_randumbs={
	"Holding the {adjective} {thing}.",
	"Giant {adjective} {thing} aproaching.",
	"This is not my {adjective} {thing}.",
	"Beware of {adjective} {thing}.",
	"Lost {adjective} {thing}.",
	"Found {adjective} {thing}.",
	"Searching for {adjective} {thing}.",
	"Testing {adjective} {thing}.",
	"Checking {adjective} {thing}.",
	"Dropped {adjective} {thing}.",
	"Disabled {adjective} {thing}.",
	"Enabled {adjective} {thing}.",
	"Buying {adjective} {thing}.",
	"Selling {adjective} {thing}.",
}

local ts={"Bios OK.","Looking for boot.","Other shoe found.","Bable mode initiated!"}



M.bake=function(oven,main)
	local main=main or {}
		
	local gl=oven.gl
	local cake=oven.cake
	local opts=oven.opts
	local canvas=cake.canvas
	local font=canvas.font
	local flat=canvas.flat
	local sheets=oven.cake.sheets
--	local layout=cake.layouts.create{}
	
	

	
main.config=function(opts)

	main.clean() -- remove old loaded datas

	opts=opts or {}
	
	main.screen_hx=opts.screen_hx or 256
	main.screen_hy=opts.screen_hy or 256
	main.screen_argb=opts.screen_argb or  0xff000011

-- the origin is in the center and you can guarantee that the above screen size is visible, there may be much more border
	main.text_dx=opts.text_dx or -128
	main.text_dy=opts.text_dy or -128
	main.text_hx=opts.text_hx or  256
	main.text_hy=opts.text_hy or  256
	main.text_argb=opts.text_argb or  0xff1188ff
	main.text_rz=opts.text_rz or  0
	
	main.img=opts.img or nil -- a background image name, will also be loaded before we load anything else
	main.img_hx=opts.img_hx or nil -- size of image to draw, relative to screen_hx/hy size
	main.img_hy=opts.img_hy or nil -- always positioned in the center of the screen

--[[
main.img="imgs/title"
main.img_hx=512
main.img_hy=256
]]

	if main.img then
		oven.cake.images.preload={ [main.img]=main.img } -- high priority load first
		sheets.loads_and_chops{
			{main.img,1,1,0.5,0.5},
		}
	else
		oven.cake.images.preload=nil
	end

end


main.config_as=function(name)

	if not name then -- pick a name, based on available files
		if     oven.cake.images.exists("imgs/preloader/pimoroni")    then name="pimoroni"
		elseif oven.cake.images.exists("imgs/preloader/kittyscreen") then name="kittyscreen"
		elseif oven.cake.images.exists("imgs/preloader/kittychair")  then name="kittychair"
		end
	end
	
	if name=="kittychair" then
		if oven.cake.images.exists("imgs/preloader/kittychair") then
			main.config{
				screen_hx=256,
				screen_hy=512,
				screen_argb=0x00000000,
				text_dx=(630/2)-512,
				text_dy=(460/2)-256,
				text_hx=(930-630)/2,
				text_hy=(640-460)/2,
				text_argb=0xff008800,
				text_rz=0,
				img="imgs/preloader/kittychair",
				img_hx=1024,
				img_hy=512,
			}
		end
	elseif name=="pimoroni" then
		if oven.cake.images.exists("imgs/preloader/pimoroni") then
			main.config{
				screen_hx=256,
				screen_hy=512,
				screen_argb=0x00000000,
				text_dx=(630/2)-512,
				text_dy=(460/2)-256,
				text_hx=(930-630)/2,
				text_hy=(640-460)/2,
				text_argb=0xff008800,
				text_rz=0,
				img="imgs/preloader/pimoroni",
				img_hx=1024,
				img_hy=512,
			}
		end
	elseif name=="kittyscreen" then
		if oven.cake.images.exists("imgs/preloader/kittyscreen") then
			main.config{
				screen_hx=256,
				screen_hy=512,
				screen_argb=0x00000000,
				text_dx=(850/2)-512,
				text_dy=(380/2)-256,
				text_hx=(1250-860)/2,
				text_hy=(570-380)/2,
				text_argb=0xff008800,
				text_rz=-3,
				img="imgs/preloader/kittyscreen",
				img_hx=1024,
				img_hy=512,
			}
		end
	else
		main.config()
	end

end

	
main.loads=function()
	oven.cake.fonts.loads({1}) -- load 1st builtin font, a basic 8x8 font
	if main.img then
	sheets.loads_and_chops{
		{main.img,1,1,0.5,0.5},
	}
	end
end

main.setup=function()
	log("setup",M.modname)

	if main.setup_done then return end -- warning, this is called repeatedly...


--	main.config_as() -- auto config

	main.title=""
	main.count=0

	main.setup_done=true
	main.loads()
end

main.reset=function()
	main.title=""
	main.count=0
end

main.clean=function()
	main.setup_done=false
end

main.msg=function(m)
end

main.update=function(sa,title)

	main.title=title or sa or ""
	
	local s=t_randumbs[math.random(1,#t_randumbs)]
	local t={}
	t.thing    =snames.random_noun()
	t.adjective=snames.random_adjective()
	
	s=wstr.replace(s,t)
	for i,v in ipairs( wstr.smart_wrap(s,30) ) do
		ts[#ts+1]=v
	end
--	ts[#ts+1]=""
	
	
	while #ts>(32-3) do
		table.remove(ts,1)
	end

	main.count=main.count+1

end



-- this is the only function we expect to be called
-- and we expect it to be called very sporadically
main.draw=function()

	main.view=cake.views.create({
		parent=cake.views.get(),
		mode="full",
		vx=main.screen_hx,
		vy=main.screen_hy,
		fov=0,
	})

--	layout.viewport() -- did our window change?
--	layout.project23d(main.screen_hx,main.screen_hy,1/4,main.screen_hy*4)
	cake.views.push_and_apply(main.view)
	canvas.gl_default() -- reset gl state

	gl.ClearColor(gl.C8(main.screen_argb))
	gl.Clear(gl.COLOR_BUFFER_BIT)--+gl.DEPTH_BUFFER_BIT)

--	gl.MatrixMode(gl.PROJECTION)
--	gl.LoadMatrix( layout.pmtx )

--	gl.MatrixMode(gl.MODELVIEW)
--	gl.LoadIdentity()
--	gl.Translate(0,0,-main.screen_hy*2) -- z depth fixed


	if main.img then
		local s=sheets.get(main.img)
		sheets.start() -- safe to call multiple times
		if s and s.img then
			s:draw(1,0,0,nil,main.img_hx,main.img_hy)
		end
	end

	gl.Translate(main.text_dx,main.text_dy,0) -- now top left corner is origin
	gl.Rotate(main.text_rz,0,0,1)

	gl.Scale( main.text_hx / 256 , main.text_hy / 256 , 1 )

	gl.PushMatrix()
	
	font.set(cake.fonts.get(1)) -- default font
	font.set_size(8,0) -- 32 pixels high

	gl.Color(pack.argb8_pmf4(main.text_argb))

	font.set_xy( 8 , 8 )
	font.draw("MemCheck: "..main.count.." "..main.title:sub(1,30-13))
--print("MemCheck: "..main.count.." : "..tostring(oven.preloader_enabled))

	for i=1,#ts do
		font.set_xy( 8, 16+i*8 )
		font.draw(ts[i])
	end

	gl.PopMatrix()	
	cake.views.pop_and_apply()

end

	main.config_as() -- call this if you want to chage the settings
	return main
end

