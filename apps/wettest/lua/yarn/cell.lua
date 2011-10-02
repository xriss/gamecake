

-- a single location

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

module(...)
local yarn_attr=require("yarn.attr")


local a_space=string.byte(" ",1)
local a_under=string.byte("_",1)
local a_star=string.byte("*",1)
local a_hash=string.byte("#",1)
local a_dash=string.byte("-",1)
local a_pipe=string.byte("|",1)
local a_plus=string.byte("+",1)
local a_dot=string.byte(".",1)
local a_equal=string.byte("=",1)


function create(t,_level)

local d={}
setfenv(1,d)

	is=yarn_attr.create(t)
	metatable={__index=is}
	setmetatable(d,metatable)

	level=t.level or _level
	
-- location in the map

	level=t.level
	xp=t.xp or 0
	yp=t.yp or 0
	id=t.id or 0
		
	items={}
	
	set.name("wall")
	
	function neighbours()
		local n_x_look={  0 , -1 , 1 , 0 }
		local n_y_look={ -1 ,  0 , 0 , 1 }
		return function(d,i)
			if i>4 then return nil,nil end -- no more edges
			return i+1 , level.get_cell( xp+n_x_look[i] , yp+n_y_look[i] )
		end , d , 1
	end
	
	function neighboursplus()
		local n_x_look={ 0,  0 , -1 , 1 , 0 }
		local n_y_look={ 0, -1 ,  0 , 0 , 1 }
		return function(d,i)
			if i>5 then return nil,nil end -- no more edges
			return i+1 , level.get_cell( xp+n_x_look[i] , yp+n_y_look[i] )
		end , d , 1
	end

	function borders()
		local n_x_look={ -1 ,  0 ,  1 , -1 , 1 , -1 , 0 , 1 }
		local n_y_look={ -1 , -1 , -1 ,  0 , 0 ,  1 , 1 , 1 }
		return function(d,i)
			if i>8 then return nil,nil end -- no more edges
			return i+1 , level.get_cell( xp+n_x_look[i] , yp+n_y_look[i] )
		end , d , 1
	end
	
	function bordersplus()
		local n_x_look={ 0, -1 ,  0 ,  1 , -1 , 1 , -1 , 0 , 1 }
		local n_y_look={ 0, -1 , -1 , -1 ,  0 , 0 ,  1 , 1 , 1 }
		return function(d,i)
			if i>9 then return nil,nil end -- no more edges
			return i+1 , level.get_cell( xp+n_x_look[i] , yp+n_y_look[i] )
		end , d , 1
	end

	function get_item() -- although there are multiple item slots, just pick one
		for v,b in pairs(items) do
			if v.form=="item" then return v end
		end
	end
	
	function get_char() -- find the char in the items list or nil if no char
		for v,b in pairs(items) do
			if v.form=="char" then return v end
		end
	end
	
	function is_empty()
		if name=="floor" and not get_char() then return true end
		return false
	end
	
	function asc()
		if not is.get.visible() then return a_space end
		
		local char=get_char()
		local item=get_item()
		
		if char then
			return char.asc()
		end
		
		if item then return item.asc() end
		
		if name=="wall" then -- some cells are just walls
			return a_hash
		else
			return a_dot
		end
			
		return a_hash
	end

-- create a save state for this data
	function save()
		local sd={}
		sd.is=yarn_attr.save(is)
		
		sd.is.xp=nil
		sd.is.yp=nil
		sd.is.id=nil
		
		for v,b in pairs(items) do
			sd.items=sd.items or {}
			sd.items[#sd.items+1]=v.save()
		end
		
		return sd
	end

-- reload a saved data (create and then load)
	function load(sd)
		d.is=yarn_attr.load(sd.is)
		d.metatable.__index=is
	end
	
	return d
	
end

