--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Main Good Luck Have Fun system virtual machine management.


local wgrd =require("wetgenes.grd")
local wsandbox=require("wetgenes.sandbox")
local wzips=require("wetgenes.zips")


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
	if system.co then
		if coroutine.status(system.co)~="dead" then

			local a,b=coroutine.resume(system.co,need)
			
			if a then return a,b end -- no error
			
			error( b.."\nin coroutine\n"..debug.traceback(system.co) ) -- error
			
		end
	end
end

system.load_and_setup=function(name,path)
	
	name=assert(name or oven.opts.fun)
	path=path or ""

-- remember source text
	system.source_filename=path..name
	system.source={}
	system.source.glsl=wzips.readfile(system.source_filename..".fun.glsl")
	system.source.lua=wzips.readfile(system.source_filename..".fun.lua")

	local lua=assert(system.source.lua,"file not found: "..system.source_filename..".fun.lua")
	if system.source.glsl then gl.shader_sources( system.source.glsl , system.source_filename..".fun.glsl" ) end
	system.setup(lua)

	return system
end

system.setup=function(code)

	system.ticks=0

print("system setup")

	if code then
		system.code=wsandbox.ini(code,{ -- we are not really trying to sandbox, just a convenient function
			print=print,
			system=system,
			oven=oven,
			require=require,
			ups=oven.rebake("wetgenes.gamecake.spew.recaps").ups, -- input, for 1up - 6up 
		})
		system.opts=system.code.hardware
		system.co=coroutine.create(system.code.main)
	end	

-- possible components and perform global setup, even if they never get used

	system.tiles  =oven.rebake("wetgenes.gamecake.fun.tiles").setup()
	system.sprites=oven.rebake("wetgenes.gamecake.fun.sprites").setup()
	system.tilemap=oven.rebake("wetgenes.gamecake.fun.tilemap").setup()
	system.screen =oven.rebake("wetgenes.gamecake.fun.screen").setup()
	system.copper =oven.rebake("wetgenes.gamecake.fun.copper").setup()
	system.sfx    =oven.rebake("wetgenes.gamecake.fun.sfx").setup()


	for i,v in ipairs(system.opts) do
	
		local it
	
		
		if     v.component=="screen" then

			it=system.screen.create({system=system},v)

		elseif v.component=="tiles" then

			it=system.tiles.create({system=system},v)

		elseif v.component=="sprites" then

			it=system.sprites.create({system=system},v)

		elseif v.component=="tilemap" then

			it=system.tilemap.create({system=system},v)

		elseif v.component=="copper" then

			it=system.copper.create({system=system},v)

		elseif v.component=="sfx" then

			it=system.sfx.create({system=system},v)

		end
		
		if it then

			print("+",v.component,v.name or v.component)
		
			system.components[#system.components+1]=it
			system.components[it.name or it.component]=it	-- link by name

		end
		
	end
	
	assert(system.components.screen,"need a screen component")

-- perform specific setup of used components
	local opts={
	}
	for _,it in ipairs(system.components) do
		if it.setup then it.setup(opts) end
	end

	system.resume({setup=true})

--hax
--	system.save_fun_png()

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
end

system.msg=function(m)
	system.resume({msg=true})
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

	system.resume({draw=true})

	local screen=system.components.screen

	screen.draw_into_start()

	for _,it in ipairs(system.components) do
		if it.draw then it.draw() end
	end
	
--	system.components.copper.draw()

	screen.draw_into_finish()
	screen.draw_fbo()

end

-- try and turn textfiles and memory into a fun.png format image
-- this may include the graphics twice once in the png, once in the source
-- but should look like the final plan we have for .fun.png files 
system.save_fun_png=function(name,path)

	local its={}

-- find grds
	for n,it in ipairs(system.components) do
		local t
		if it.bitmap_grd then
			t=t or {}
			t.component=it
			t.grd=it.bitmap_grd
		end
		if it.tilemap_grd then
			t=t or {}
			t.component=it
			t.grd=it.tilemap_grd
		end
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

-- debug
	local hx,hy=0,0
	for i,it in ipairs(its) do
		if it.px+it.hx+bb > hx then hx=it.px+it.hx+bb end
		if it.py+it.hy+bb > hy then hy=it.py+it.hy+bb end
	end
	print(0,0,0,hx,hy,"BITMAP")
	local g=wgrd.create("U8_RGBA", hx , hy , 1)

	for i,it in ipairs(its) do
		g:pixels(it.px,it.py,it.hx,it.hy, it.grd )
		print(i,it.px,it.py,it.hx,it.hy,it.component.name)
	end
	
	g:save(system.source_filename..".fun.png")

end


	return system
end
