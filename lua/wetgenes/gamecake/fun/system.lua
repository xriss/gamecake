--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

-- Main Good Luck Have Fun system virtual machine management.


local wwin=require("wetgenes.win")
local wgrd =require("wetgenes.grd")
local wsandbox=require("wetgenes.sandbox")
local wzips=require("wetgenes.zips")
local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wjson=require("wetgenes.json")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,system)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

	system.components={}

-- utility code


system.resume=function(need)
	if system.main then
		if coroutine.status(system.main)~="dead" then

			local a,b=coroutine.resume(system.main,need)
			
			if a then return a,b end -- no error
			
			error( b.."\nin coroutine\n"..debug.traceback(system.main) , 2 ) -- error
			
		end
	end
end

system.load_and_setup=function(name,path)

	gl.forget() -- force reload of shaders
	
	if not name or name=="" then name=oven.opts.fun end --default menu launcher
	if not name or name=="" then name="lua/fun/all" end --default menu launcher
	path=path or ""

	name=assert(name)


-- remember source text
	system.source_filename=path..name
	system.source={}
	
	if system.source_filename=="|" then -- read in pipe if we use | as the filename.
	
		system.source.lua=io.read("*all")

	else
	
		system.source.glsl=wzips.readfile(system.source_filename..".fun.glsl") or wzips.readfile(system.source_filename..".glsl")
		system.source.lua=wzips.readfile(system.source_filename..".fun.lua") or wzips.readfile(system.source_filename..".lua") or wzips.readfile(system.source_filename )
	
	end

	local lua=assert(system.source.lua,"file not found: "..system.source_filename..".fun.lua")
	
	if system.source.glsl then gl.shader_sources( system.source.glsl , system.source_filename..".fun.glsl" ) end
	
	gl.shader_sources( system.source.lua , system.source_filename..".fun.lua" ) -- also try and load GLSL embeded in the lua file -> #SHADER

	system.setup(lua)

	return system
end

system.setup=function(code)

	system.ticks=0
	
	local screensize=wwin.screen() -- make screen size available for autoconfig
	system.fullscreen_width=screensize.width or 1920
	system.fullscreen_height=screensize.height or 1080

--print("system setup "..system.fullscreen_width.."x"..system.fullscreen_height)

	if code then

		local env=wsandbox.make_env()
		for n,v in pairs({
			args=oven.opts.args, -- commandline settings
			debug=debug,
			print=print,
			system=system,
			oven=oven,
			gl=oven.gl,
			require=require,
			package=package, -- to help with module creation
			ups=oven.rebake("wetgenes.gamecake.spew.recaps").ups, -- input, for 1up - 6up 
		}) do env[n]=v end
		
		local tab={}
		local meta={__index=env}
		env._G=tab
		setmetatable(tab, meta)

		local f
		if setfenv and loadstring then
			f=assert(loadstring(code))
			setfenv(f,tab)
		else
			f=assert(load(code, nil,"t",tab))
		end

		local co=coroutine.create(f)
		
		local a,b=coroutine.resume(co,{})
		if not a then error( b.."\nin coroutine\n"..debug.traceback(co) , 2 ) end

		system.code=tab
		
		if system.code.hardware and system.code.main then -- hardware and main
			system.hardware=system.code.hardware
			system.main=coroutine.create(system.code.main)
		elseif system.code.hardware then -- hardware only
			system.hardware=system.code.hardware
		end
		
		if not system.main then -- the whole file is the coroutine
			system.hardware={}
			system.main=co
		end
		
	end	

-- possible components and perform global setup, even if they never get used

	system.creates={} -- all available systems
	for i,n in ipairs{
			"sfx",
			"colors",
			"screen",
			"copper",
			"tiles",
			"canvas",
			"tilemap",
			"sprites",
			"autocell",
			"overmap",
		} do
		system.creates[n] = oven.rebake( "wetgenes.gamecake.fun."..n).setup()
	end

	for i,v in ipairs(system.hardware) do
	
		local it
	
		if system.creates[ v.component ] then
			it=system.creates[ v.component ].create({system=system},v)
		end
		
		if it then

			log("fun","hardware : ",v.component,v.name or v.component)
		
			system.components[#system.components+1]=it
			system.components[it.name or it.component]=it	-- link by name

		end
		
	end
	
--	assert(system.components.screen,"need a screen component")

-- perform specific setup of used components
	local opts={
	}
	for _,it in ipairs(system.components) do
		if it.setup then it.setup(opts) end
	end

	system.resume({setup=true})

	if system.components.screen then
		oven.mods["wetgenes.gamecake.mods.snaps"].fbo=system.components.screen.fbo
	end
	
	system.is_setup=true
	return system
end

-- this should be called after adding or removing components

system.setup_names=function()

-- remove all name links
	for n,it in pairs(system.components) do
		if type(n)~="number" then system.components[n]=nil end
	end

-- add current systems
	for i=1,#system.components do
		local it=system.components[i]
		system.components[it.name or it.component]=it	-- link by name
	end
end

system.clean=function()
	system.resume({clean=true})
	for _,it in ipairs(system.components) do
		if it.clean then it.clean() end
	end
	system.is_setup=false
end

system.msg=function(m)
	system.resume({msg=m})
	for _,it in ipairs(system.components) do
		if it.msg then it.msg(m) end
	end
end

system.update=function()

	system.ticks=system.ticks+1
	system.resume({update=true})

	for _,it in ipairs(system.components) do
		if it.update then it.update() end
	end
end

system.draw=function()

	local use_this_fbo=gl.Get(gl.FRAMEBUFFER_BINDING)

	system.resume({draw=true})

	local screen=system.components.screen

-- layer 0 is used for uploading textures
	for _,it in ipairs(system.components) do
		if it.draw and it.layer==0 then it.draw() end
	end


	for idx=1,#screen.layers do

		screen.draw_into_layer_start(idx)

		for _,it in ipairs(system.components) do
			if it.draw and it.layer==idx then it.draw() end
		end

		screen.hooks(idx) -- custom extra lowlevel drawing for this layer

		screen.draw_into_layer_finish(idx)

	end

	screen.draw_into_screen_start()
	for idx=1,#screen.layers do screen.draw_layer(idx) end
	screen.draw_into_screen_finish()
	
	gl.BindFramebuffer(gl.FRAMEBUFFER, use_this_fbo)
	screen.draw_screen()

--hax
	if not system.done_save_fun_png then
		system.done_save_fun_png=true
		if oven.opts.args.savepng then -- pass --savepng on commandline to dump grafix memory after setup
			system.save_fun_png()
		elseif oven.opts.args["savegif"] then -- pass --savegif=1200 to record first 20 seconds
			oven.snaps.begin_record( tonumber(oven.opts.args["savegif"]) or 1 )
		end
	end

end

system.draw_debug=function()

	local screen=system.components.screen
	local tiles=system.components.tiles
	local colors=system.components.colors
	
	if screen and tiles and colors then -- draw raw tiles

		local t={
			0,			tiles.hy,	0, 0,1,
			0,			0,			0, 0,0,
			tiles.hx,	tiles.hy,	0, 1,1,
			tiles.hx,	0,			0, 1,0,
		}

		flat.tristrip("rawuv",t,"fun_draw_tiles_debug",function(p)

			gl.Uniform2f( p:uniform("projection_zxy"), 0,0)

			gl.ActiveTexture(gl.TEXTURE0) gl.Uniform1i( p:uniform("tex_tile"), 0 )
			gl.BindTexture( gl.TEXTURE_2D , tiles.bitmap_tex )

		end)
	
	end


end

-- returns true if any component is dirty
system.dirty=function(flag)
	local dirty=false

	if type(flag)=="boolean" then
		for n,it in ipairs(system.components) do
			if it.dirty then it.dirty(flag) end
		end
	end

	for n,it in ipairs(system.components) do
		dirty=dirty or ( it.dirty and it.dirty() )
	end
	return dirty
end


-- try and turn textfiles and memory into a fun.png format image
-- this may include the graphics twice once in the png, once in the source
-- but should look like the final plan we have for .fun.png files 
system.save_fun_png=function(name,path)


	local its={}

-- find grds
	for n,it in ipairs(system.components) do
		local t
		
		if it.fbo then --dump screen fbo
			t=t or {}
			t.component=it
			t.grd=it.fbo:download()
		end
		
		if it.bitmap_grd then
			t=t or {}
			t.component=it
			t.grd=it.bitmap_grd
		end
--[[
		if it.tilemap_grd then
			t=t or {}
			t.component=it
			t.grd=it.tilemap_grd
		end
]]
		if t then
			its[#its+1]=t
		end
	end

-- sort largest grds first
	table.sort(its,function(a,b)
		if a.grd.width == b.grd.width then return a.grd.height > b.grd.height end
		return a.grd.width > b.grd.width
	end)

	local bb=8
	local ax,ay,bx,by=0,0,0,0 -- dumb layout code points
	for i,it in ipairs(its) do
		it.hx=it.grd.width
		it.hy=it.grd.height
		if ay+it.hy > by then -- start a new row
			ay=by
			ax=0 -- reset
			it.px=ax+bb
			it.py=ay+bb
			bx=ax+bb+it.hx
			by=ay+bb+it.hy
			ax=bx -- place next grd to right
		else -- add to right of current row
			it.px=ax+bb
			it.py=ay+bb
			ay=ay+bb+it.hy
		end
	end

-- work out size of output image
	local hx,hy=0,0
	for i,it in ipairs(its) do
		if it.px+it.hx+bb > hx then hx=it.px+it.hx+bb end
		if it.py+it.hy+bb > hy then hy=it.py+it.hy+bb end
	end
	if hx<64 then hx=64 end -- minimum width so data can encode

-- build data string that we will encode into the bottom of the image
	local data={}
	data.source=system.source
	data.images={}
	for i,it in ipairs(its) do
		local t={}
		t.px=it.px
		t.py=it.py
		t.hx=it.hx
		t.hy=it.hy
		t.name=it.component.name
		data.images[#data.images+1]=t
	end
	local dstr=(require("zlib").deflate())( wjson.encode(data) ,"finish") -- zlib compressed json

	local dsize=#dstr+12 -- header size is 12
	local dh=math.ceil(dsize/(hx*4))
	
	log("fun",0,0,0,hx,hy,"BITMAP")
	log("fun",0,0,0,hx,dh,"DATA",dsize)
	
	local g=wgrd.create("U8_RGBA", hx , hy+dh , 1)
	g:clear(0xff000000) -- start with a solid black

	-- add data at bottom of image, filling upwards

-- s32 to 4 byte string
	local s32_to_string=function(n)
		local a=string.char(bit.band(n/0x01000000,0xff))
		local b=string.char(bit.band(n/0x010000,0xff))
		local c=string.char(bit.band(n/0x0100,0xff))
		local d=string.char(bit.band(n,0xff))
		return d..c..b..a
	end
	
	local doff=0 -- data offset
	local checksum=0
	for i=1,dh do
		local d
		if i==1 then -- add a header then start the string
			d="FUN\0"..s32_to_string(checksum)..s32_to_string(dsize)..dstr:sub(doff+1,doff+(hx*4)+1-12)
			doff=doff+(hx*4)-12
		else
			d=dstr:sub(doff+1,doff+(hx*4)+1) -- the last line will automatically be shorter if needed
			doff=doff+(hx*4)
			if i==dh then d=d..(("\0\0\0\0"):rep(hx)) end -- padding of final line
		end
		g:pixels(0,hy+dh-i,hx,1, d )
	end

	-- add each component ( some pixels will now be transparent
	for i,it in ipairs(its) do
		g:pixels(it.px,it.py,it.hx,it.hy, it.grd )
		log("fun",i,it.px,it.py,it.hx,it.hy,it.component.name)
	end

	-- draw box around area
	for i,it in ipairs(its) do		
		g:clip( it.px-5       , it.py-5       , 0 , 2        , it.hy+10 , 1 ):clear(0xff444444)
		g:clip( it.px-5       , it.py-5       , 0 , it.hx+10 , 2        , 1 ):clear(0xff444444)
		g:clip( it.px+it.hx+3 , it.py-5       , 0 , 2        , it.hy+10 , 1 ):clear(0xff444444)
		g:clip( it.px-5       , it.py+it.hy+3 , 0 , it.hx+10 , 2        , 1 ):clear(0xff444444)
	end

	-- draw title
	local font=bitdown.setup_blit_font()
	for i,it in ipairs(its) do
		local s=it.component.name:upper()
		s=s:sub(1,math.floor(it.hx/4)) -- fit into box
		font.draw(g,it.px,it.py-8,s)
	end


	
	g:save(system.source_filename..".fun.png")

end

system.configurator=function(opts)

	local bitdown=require("wetgenes.gamecake.fun.bitdown")
	local bitdown_font=require("wetgenes.gamecake.fun.bitdown_font")
	
	local args=opts.args or oven.opts.args -- handle commandline arguments unless replaced
	
	local fatpix=not args.pixel -- pass --pixel on command line to turn off fat pixel filters

	local hardware,main

	local done=false

	if opts.mode=="picish" then -- tiny settings

		opts.cmap = opts.cmap or bitdown.cmap -- use default swanky32 colors
		opts.hx  = opts.hx  or 128
		opts.hy  = opts.hy  or 128
		opts.ss  = opts.ss  or 6
		opts.fps = opts.fps or 60

		hardware={
			{
				component="screen",
				name="screen",
				size={opts.hx,opts.hy},
				bloom=fatpix and 0.75 or 0,
				filter=fatpix and "scanline" or nil,
				shadow=fatpix and "drop" or nil,
				scale=args.pixel and tonumber(args.pixel) or opts.ss,
				fps=opts.fps,
				layers=3,
			},
			{
				component="sfx",
				name="sfx",
			},
			{
				component="colors",
				name="colors",
				cmap=opts.cmap, -- swanky32 palette
			},
			{
				component="tiles",
				name="tiles",
				tile_size={8,8},
				bitmap_size={64,64},
			},
			{
				component="canvas",
				name="canvas",
				size={opts.hx,opts.hy},
				layer=1,
			},
			{
				component="tilemap",
				name="map",
				tiles="tiles",
				tile_size={8,8},
				tilemap_size={math.ceil(opts.hx/8),math.ceil(opts.hy/8)},
				layer=2,
			},
			{
				component="sprites",
				name="sprites",
				tiles="tiles",
				layer=2,
			},
			{
				component="tilemap",
				name="text",
				tiles="tiles",
				tile_size={4,8}, -- use half width tiles for font
				tilemap_size={math.ceil(opts.hx/4),math.ceil(opts.hy/8)},
				layer=3,
			},
			graphics={
				{0x0000,"_font",0x0340}, -- pre-allocate the 4x8 and 8x8 font area
			},
			opts=opts,
		}

	elseif opts.mode=="fun64" then -- default settings
	
		opts.cmap = opts.cmap or bitdown.cmap -- use default swanky32 colors
		opts.hx  = opts.hx  or 320
		opts.hy  = opts.hy  or 240
		opts.ss  = opts.ss  or 3
		opts.fps = opts.fps or 60
		
		hardware={
			{
				component="screen",
				name="screen",
				size={opts.hx,opts.hy},
				bloom=fatpix and 0.75 or 0,
				filter=fatpix and "scanline" or nil,
				shadow=fatpix and "drop" or nil,
				scale=args.pixel and tonumber(args.pixel) or opts.ss,
				fps=opts.fps,
				layers=3,
			},
			{
				component="sfx",
				name="sfx",
			},
			{
				component="colors",
				name="colors",
				cmap=opts.cmap, -- swanky32 palette
			},
			{
				component="tiles",
				name="tiles",
				tile_size={8,8},
				bitmap_size={64,64},
			},
			{
				component="copper",
				name="copper",
				size={opts.hx,opts.hy},
				layer=1,
			},
			{
				component="tilemap",
				name="back",
				tiles="tiles",
				tile_size={8,8},
				tilemap_size={math.ceil(opts.hx/8),math.ceil(opts.hy/8)},
				layer=1,
			},
			{
				component="tilemap",
				name="map",
				tiles="tiles",
				tile_size={8,8},
				tilemap_size={math.ceil(opts.hx/8),math.ceil(opts.hy/8)},
				layer=2,
			},
			{
				component="sprites",
				name="sprites",
				tiles="tiles",
				layer=2,
			},
			{
				component="tilemap",
				name="text",
				tiles="tiles",
				tile_size={4,8}, -- use half width tiles for font
				tilemap_size={math.ceil(opts.hx/4),math.ceil(opts.hy/8)},
				layer=3,
			},
			graphics={
				{0x0000,"_font",0x0340}, -- pre-allocate the 4x8 and 8x8 font area
			},
			opts=opts,
		}
	
	elseif opts.mode=="swordstone" then -- text focused 256x128 screen with sprites ( 64x16 chars with 4x8 font )
	
		opts.cmap = opts.cmap or bitdown.cmap -- use default swanky32 colors
		opts.hx  = opts.hx  or 256
		opts.hy  = opts.hy  or 128
		opts.ss  = opts.ss  or 4
		opts.fps = opts.fps or 60
		
		hardware={
			{
				component="screen",
				name="screen",
				size={opts.hx,opts.hy},
				bloom=fatpix and 0.75 or 0,
				filter=fatpix and "scanline" or nil,
				shadow=fatpix and "drop" or nil,
				scale=args.pixel and tonumber(args.pixel) or opts.ss,
				fps=opts.fps,
				layers=1,
			},
			{
				component="sfx",
				name="sfx",
			},
			{
				component="colors",
				name="colors",
				cmap=opts.cmap, -- swanky32 palette
			},
			{
				component="tiles",
				name="tiles",
				tile_size={8,8},
				bitmap_size={64,64},
			},
			{
				component="tilemap",
				name="text",
				tiles="tiles",
				tile_size={4,8}, -- use half width tiles for font
				tilemap_size={math.ceil(opts.hx/4),math.ceil(opts.hy/8)},
				layer=1,
			},
			{
				component="sprites",
				name="sprites",
				tiles="tiles",
				layer=1,
			},
			graphics={
				{0x0000,"_font",0x0340}, -- pre-allocate the 4x8 and 8x8 font area
			},
			opts=opts,
		}
	
	end
	
	for i,v in ipairs(hardware) do hardware[v.name]=v end -- for easy tweaking of options
	
	hardware.remove=function(name)
		hardware[name]=nil
		for i,v in ipairs(hardware) do
			if v.name==name then
				return table.remove(hardware,i)
			end
		end
	end
	
	hardware.insert=function(it)
		if it.name then
			for i,v in ipairs(hardware) do
				if v.name==it.name then -- replace
					hardware[i]=it
					hardware[it.name]=it
					return
				end
			end
			hardware[it.name]=it
		end
		hardware[#hardware+1]=it
	end
	
	-- load a single sprite
	hardware.graphics.load=function(idx,name,data)
		local found
		for i,v in ipairs(hardware.graphics) do
			if v[2]==name then
				found=v
				break
			end
		end
		if not found then -- add new graphics
			hardware.graphics[#hardware.graphics+1]={idx,name,data}
		else
			found[1]=idx
			found[2]=name
			found[3]=data
		end
	end	

	-- load a list of sprites
	hardware.graphics.loads=function(tab)
		for i,v in ipairs(tab) do
			hardware.graphics.load(v[1],v[2],v[3])
		end
	end
	
	main=function(need)

		if not need.setup then need=coroutine.yield() end -- wait for setup request (should always be first call)

		if system.components.tiles then
			-- copy font data tiles into top line
			system.components.tiles.bitmap_grd:pixels(0,0 ,128*4,8, bitdown_font.build_grd(4,8):pixels(0,0,128*4,8,"") )
			-- and 8x8 font 
			system.components.tiles.bitmap_grd:pixels(0,8 ,64*8,8, bitdown_font.build_grd(8,8):pixels(0,   0,64*8,8,"") )
			system.components.tiles.bitmap_grd:pixels(0,16,64*8,8, bitdown_font.build_grd(8,8):pixels(64*8,0,64*8,8,"") )

			-- upload graphics
			local graphics=opts.graphics or hardware.graphics
			if graphics then
				if type(graphics)=="function" then graphics=graphics() end -- allow callback to grab value from other environment
				system.components.tiles.upload_tiles( graphics )
			end
		end
		
		if opts.update then opts.update() end -- call update at least once

		local reset_draw=opts.reset_draw or function()
			if system.components.text then
				system.components.text.dirty(true)
				system.components.text.text_window()
				system.components.text.text_clear(0x00000000)
			end
			if system.components.sprites then
				system.components.sprites.list_reset()
			end
		end
		
		-- after setup we should yield and then perform updates only if requested from a yield
		local done=false ; while not done do
			need=coroutine.yield()
			if need.update then
			
				if not opts.draw then
					reset_draw()
				end
				
				if opts.update then opts.update() end
			end
			if need.draw then
				if opts.draw then
					reset_draw()
					opts.draw()
				end
			end
			if need.msg then
				if opts.msg then
					opts.msg(need.msg) -- pass on raw msgs
				end
			end
			if need.clean then done=true end -- cleanup requested
		end

	-- perform cleanup here

		if opts.clean then opts.clean() end

	end

	return hardware,main
end


	return system
end
