-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(state,keys)

	keys=keys or {}
	
	local cake=state.cake
	local canvas=cake.canvas
	
--[[
	function keys.setup()
	end

	function keys.clean()
	end

	function keys.update()	
	end
	
	function keys.draw()
	end
		
	function keys.msg(m)
	end
]]

-- the above are just stubs "incase", most of the meat happens in the recap table


-- a players key mappings, myabe we need multiple people on the same keyboard or device
	function keys.create()
		local key={}
		
		return key
	end


	return keys
end
