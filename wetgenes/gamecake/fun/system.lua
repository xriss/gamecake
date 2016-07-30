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


	system.charmap=oven.rebake("wetgenes.gamecake.fun.charmap").setup()
	system.codemap=oven.rebake("wetgenes.gamecake.fun.codemap")
	system.screen =oven.rebake("wetgenes.gamecake.fun.screen")

	system.config={}
	system.components={}
	
	system.opts=oven.opts.fun or {} -- place your fun system setup in lua/init.lua -> opts.fun={}


system.setup=function()

print("system setup")
	for i,v in ipairs(oven.opts.fun) do
	
		local it
	
		
		if     v.component=="screen" then

			it=system.screen.create({},v)

		elseif v.component=="charmap" then

			it=system.charmap.create({},v)

		end
		
		if it then

			print("+",v.component,v.name or v.component)
		
			system.components[#system.components+1]=it
			system.components[v.name or v.component]=it	-- quick lookup by name

		end
		
	end
	
	assert(system.components.screen,"need a screen component")

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

	screen.draw_into_finish()
	screen.draw_fbo()

end

	return system
end
