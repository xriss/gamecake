
-- a monter or player or any other character, really just a slightly more active item
-- these are items that need to update as time passes

local _G=_G

local win=win

local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack

local gl=gl


local rouge_attr=require("state.rouge.attr")
local rouge_fight=require("state.rouge.char_fight")

local function print(...) _G.print(...) end


module("state.rouge.char")


local a_at=string.byte("@",1)
local a_a=string.byte("a",1)


function create(t,_level)

	
local d={}
setfenv(1,d)

	level=_level or t.level
	class=t.class
	
	cell_fx=t.cell_fx or {}
	
	time_passed=level.time_passed

	attr=rouge_attr.create(t)
	
	function del()
		if cell then -- remove link from old cell
			cell.char=nil
		end
	end
	
	function set_cell(c)
	
		if cell then -- remove link from old cell, only one char per cell
			cell.char=nil
		end
		
		cell=c
		cell.char=d
		
		if cell_fx.room_visible then -- this char makes the room visible (ie its the player)
			if not cell.room.attr.get.visible() then -- if room is not visible
				cell.room.set_visible(true)
			end
		end
		
	end
	
	function move(vx,vy)
		local x=cell.xp+vx
		local y=cell.yp+vy
		local c=level.get_cell(x,y)
		if c and c.room then -- its a cell we can move into
			if c.char then -- interact with another char?
				rouge_fight.hit(d,c.char)
				return 1					
			else -- just move
				set_cell(c)
				return 1 -- time taken to move
			end
		end
		return 0
	end

	function die()
		local p=level.new_item( class.."_corpse" )
		p.set_cell( cell )

		level.del_char(d)
	end
	
	function asc()
		return attr.asc
	end
	
	function view_text()
		return "You see "..(attr.desc or "something").."."
	end

	function update()
	
		if class=="player" then return end
		
		if 	time_passed<level.time_passed then
	
			local vs={ {1,0} , {-1,0} , {0,1} , {0,-1} }
			
			vs=vs[level.rand(1,4)]
			
			move(vs[1],vs[2])
			
			time_passed=time_passed+1
		end
	end
	
	return d
	
end

