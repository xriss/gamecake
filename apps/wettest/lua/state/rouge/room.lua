
-- a collection of cells

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

local function print(...) _G.print(...) end


module("state.rouge.room")


function create(t)

local d={}
setfenv(1,d)

	level=t.level
	xp=t.xp or 0
	yp=t.yp or 0
	xh=t.xh or 0
	yh=t.yh or 0
	doors={} -- a cell->room table of links to bordering rooms
	
	attr=rouge_attr.create(t)
	
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
--		set_walls()
	end

-- work out the wall edges for the cells of this room
	function set_walls()
	
		for _,cell in level.cpairs(xp,yp-1,xh,1) do
			if cell.wall=="|" or cell.wall=="-" then
				cell.wall="-"
			else
				cell.wall="-"
			end
		end
		for _,cell in level.cpairs(xp,yp+yh,xh,1) do
			if cell.wall=="|" or cell.wall=="-" then
				cell.wall="-"
			else
				cell.wall="-"
			end
		end
		
		for _,cell in level.cpairs(xp-1,yp,1,yh) do
			if cell.wall=="-" or cell.wall=="+" then
				cell.wall="-"
			else
				cell.wall="|"
			end
		end
		for _,cell in level.cpairs(xp+xh,yp,1,yh) do
			if cell.wall=="-" or cell.wall=="+" then
				cell.wall="-"
			else
				cell.wall="|"
			end
		end

		level.get_cell(xp-1 ,yp-1 ).wall="-"
		level.get_cell(xp+xh,yp-1 ).wall="-"
		level.get_cell(xp-1 ,yp+yh).wall="-"
		level.get_cell(xp+xh,yp+yh).wall="-"

	end
	
	return d
	
end

