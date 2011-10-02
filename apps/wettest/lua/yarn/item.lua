
-- a single item

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

local dbg=dbg or function()end



module(...)
local yarn_attr=require("yarn.attr")
local yarn_fight=require("yarn.fight")


function create(t,_level)

	
local d={}
setfenv(1,d)

	is=yarn_attr.create(t) -- allow attr access via item.is.wood syntax
	metatable={__index=is}
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
		return is.asc
	end
	
	function view_text()
		return "You see "..(is.desc or "something").."."
	end

	function move(vx,vy)
		local x=cell.xp+vx
		local y=cell.yp+vy
		local c=level.get_cell(x,y)
		
		if c and c.name=="floor" then -- its a cell we can move into
		
			local char=c.get_char()

			if char then -- interact with another char?
dbg("char "..tostring(char))				
dbg("char.can.use "..tostring(char.can.use))				
for i,v in pairs(char.can) do
	dbg(tostring(i).." = "..tostring(v))
end
dbg("can.operate "..tostring(can.operate))				
				if char.can.use and can.operate then

					local usename=char.can.use
dbg("use "..tostring(usename))				
					
					if char.can[usename] then
						char.can[usename](char , d )
					elseif usename=="menu" then
						if char.can.menu then
							char.can.menu(char,d)
						else
							level.main.menu.show_item_menu(char)
						end
					end
					
				elseif char.can.fight and can.fight then
dbg("char.can.fight "..tostring(char.can.fight))				
				
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
		local p=level.new_item( name.."_corpse" )
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

