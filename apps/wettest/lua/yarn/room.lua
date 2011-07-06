
-- a local area of cells

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


function create(t)

local d={}
setfenv(1,d)

	attr=yarn_attr.create(t)
	metatable={__index=attr}
	setmetatable(d,metatable)

	level=t.level
	xp=t.xp or 0
	yp=t.yp or 0
	xh=t.xh or 0
	yh=t.yh or 0
	doors={} -- a cell->room table of links to bordering rooms
	
--	cellfind={} -- find cells in his room only?
--	celllist={}
	
-- point to this room from the cells we cover, only one room pointer per cell

	for y=yp,yp+yh-1 do
		for x=xp,xp+xh-1 do
			local cell=level.get_cell(x,y)
			cell.room=d
		end
	end
	
 -- call this after adding all the rooms to the level to find all the doors
 -- and mark each room as linked to its neighbours

	function find_doors()
	
		for y=yp,yp+yh-1 do
			for x=xp,xp+xh-1 do
			
				local cell=level.get_cell(x,y)
				
				for i,v in cell.neighbours() do
				
					if v.room and cell.room and v.room~=cell.room then -- connected to a different room?
					
						doors[cell]=v.room
						cell.door=v.room

					end
				end
			
			end
		end
	end


-- set this room and every cell in and around this room to visible 

	function set_visible(v)
		for _,cell in level.cpairs(xp-1,yp-1,xh+2,yh+2) do
			cell.attr.set.visible(v)
		end
		attr.set.visible(v)
	end

	
	function post_create()
		for _,cell in level.cpairs(xp,yp,xh,yh) do
			cell.set.name("floor")
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

