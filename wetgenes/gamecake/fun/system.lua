--
-- (C) 2016 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- Main Good Luck Have Fun system virtual machine management.


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,system)

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local flat=canvas.flat

	system.components={}
	system.opts=oven.opts.fun or {} -- place your fun system setup in lua/init.lua -> opts.fun={}

-- utility code



system.setup=function()

print("system setup")

-- possible components and perform global setup, even if they never get used

	system.sprites=oven.rebake("wetgenes.gamecake.fun.sprites").setup()
	system.tilemap=oven.rebake("wetgenes.gamecake.fun.tilemap").setup()
	system.screen =oven.rebake("wetgenes.gamecake.fun.screen").setup()
	system.copper =oven.rebake("wetgenes.gamecake.fun.copper").setup()


	for i,v in ipairs(oven.opts.fun) do
	
		local it
	
		
		if     v.component=="screen" then

			it=system.screen.create({system=system},v)

		elseif v.component=="sprites" then

			it=system.sprites.create({system=system},v)

		elseif v.component=="tilemap" then

			it=system.tilemap.create({system=system},v)

		elseif v.component=="copper" then

			it=system.copper.create({system=system},v)

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
	for _,it in ipairs(system.components) do
		if it.clean then it.clean() end
	end
end

system.msg=function(m)
	for _,it in ipairs(system.components) do
		if it.msg then it.msg(m) end
	end
end

system.update=function()
	for _,it in ipairs(system.components) do
		if it.update then it.update() end
	end
end

system.draw=function()

	local screen=system.components.screen

	screen.draw_into_start()

	for _,it in ipairs(system.components) do
		if it.draw then it.draw() end
	end
	
--	system.components.copper.draw()

	screen.draw_into_finish()
	screen.draw_fbo()

end

	return system
end
