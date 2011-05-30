

-- a single location

local _G=_G

local win=win

local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local pairs=pairs

local gl=gl

local rouge_attr=require("state.rouge.attr")

local function print(...) _G.print(...) end


module("state.rouge.cell")


local a_space=string.byte(" ",1)
local a_under=string.byte("_",1)
local a_star=string.byte("*",1)
local a_hash=string.byte("#",1)
local a_dash=string.byte("-",1)
local a_pipe=string.byte("|",1)
local a_plus=string.byte("+",1)
local a_dot=string.byte(".",1)
local a_equal=string.byte("=",1)


function create(t)

local d={}
setfenv(1,d)

-- location in the map

	level=t.level
	xp=t.xp or 0
	yp=t.yp or 0
	id=t.id or 0
	
	attr=rouge_attr.create(t)
	
	items={}

	function neighbours()
		local n_x_look={  0 , -1 , 1 , 0 }
		local n_y_look={ -1 ,  0 , 0 , 1 }
		return function(d,i)
			if i>4 then return nil,nil end -- no more edges
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
	
	function get_item() -- although there are multiple item slots, just pick one
		for v,b in pairs(items) do return v end
	end
	
	function asc()
		if not attr.get.visible() then return a_space end
		
		if char then
			return char.asc()
		end
		
		local item=get_item()
		if item then return item.asc() end
		
		if room then
--[[
			if door then
				if room.xh==1 or room.yh==1 then
					if door.xh~=1 and door.yh~=1 then
						return a_equal
					end
				end
			end
]]
			if room then return a_dot end
		end
		
--		if wall then return string.byte(wall,1) end
		return a_hash
	end

	return d
	
end

