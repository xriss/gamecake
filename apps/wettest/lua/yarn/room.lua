
-- a local area of cells

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug


module(...)
local yarn_attr=require("yarn.attr")


function create(t)

local d={}
setfenv(1,d)

	is=yarn_attr.create(t)
	metatable={__index=is}
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
			cell.is.set.visible(v)
		end
		is.set.visible(v)
	end

	
	function post_create()
		for _,cell in level.cpairs(xp,yp,xh,yh) do
			cell.set.name("floor")
		end
	end

-- create a save state for this data
	function save()
		local sd={}
		
		sd.is=yarn_attr.save(is)
		
		return sd
	end

-- reload a saved data (create and then load)
	function load(sd)
		d.is=yarn_attr.load(sd.is)
		d.metatable.__index=is
	end
	
	return d
	
end

