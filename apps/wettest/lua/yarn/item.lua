
-- a single item

local _G=_G


local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local require=require
local setmetatable=setmetatable



module(...)
local yarn_attr=require("yarn.attr")
local yarn_fight=require("yarn.fight")


function create(t,_level)

	
local d={}
setfenv(1,d)

	attr=yarn_attr.create(t)
	metatable={__index=attr}
	setmetatable(d,metatable)

	level=_level or t.level
	class=t.class
	
	time_passed=level.time_passed
	
	function del()
		if cell then -- remove link from old cell
			cell.items[d]=nil
		end
	end

	function set_cell(c)
	
		if cell then -- remove link from old cell
			cell.items[d]=nil
		end
		
		cell=c
		cell.items[d]=true
		
		if can.make_room_visible then -- this item makes the room visible (ie its the player)
			for i,v in cell.neighboursplus() do -- apply to neighbours and self
				if v.room and ( not v.room.get.visible() ) then -- if room is not visible
					v.room.set_visible(true)
				end
			end
		end
		
	end
	
	function asc()
		return attr.asc
	end
	
	function view_text()
		return "You see "..(attr.desc or "something").."."
	end

	function move(vx,vy)
		local x=cell.xp+vx
		local y=cell.yp+vy
		local c=level.get_cell(x,y)
		
		if c and c.name=="floor" then -- its a cell we can move into
		
			local char=c.get_char()
			if char then -- interact with another char?
				if char.can.use and can.operate then
				
					local usename=char.can.use
					
					if char.call[usename] then
						char.call[usename](char , d )
					elseif usename=="menu" then
						if char.call.menu then
							char.call.menu(char,d)
						else
							level.main.menu.show_item_menu(char)
						end
					end
					
				elseif char.can.fight and can.fight then
				
					yarn_fight.hit(d,char)
					return 1
					
				end
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

		level.del_item(d)
	end

	function update()
	
		if can.roam=="random" then
		
			if 	time_passed<level.time_passed then
		
				local vs={ {1,0} , {-1,0} , {0,1} , {0,-1} }
				
				vs=vs[level.rand(1,4)]
				
				move(vs[1],vs[2])
				
				time_passed=time_passed+1
			end
			
		end
		
	end

-- create a save state for this data
	function save()
		local sd={}
		
		sd.attr=yarn_attr.save(attr)
		
		return sd
	end

-- reload a saved data (create and then load)
	function load(sd)
		d.attr=yarn_attr.load(sd.attr)
		d.metatable.__index=attr
	end

	return d
	
end

