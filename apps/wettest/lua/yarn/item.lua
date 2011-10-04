
-- a single item

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

local dbg=dbg or function()end



module(...)
local yarn_attr=require("yarn.attr")
local yarn_attrs=require("yarn.attrs")
local yarn_fight=require("yarn.fight")
local yarn_level=require("yarn.level")


function create(t,_level)

	
local d={}
setfenv(1,d)

	is=yarn_attr.create(t) -- allow attr access via item.is.wood syntax
	metatable={__index=is} -- or without the is if we do not fear nameclash
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
		if not cell.items then cell.items={} end -- make space in non cells
		cell.items[d]=true
		
		if can.make_room_visible then -- this item makes the room visible (ie its the player)
			if cell.room then
				for i,v in cell.neighboursplus() do -- apply to neighbours and self
					if v.room and ( not v.room.get.visible() ) then -- if room is not visible
						v.room.set_visible(true)
					end
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
				if char.can.use and can.operate then

					local usename=char.can.use
					
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
				
					if char.is.player or is.player then -- do not fight amongst selfs				
						yarn_fight.hit(d,char)
						return 1
					end
					
				end
			else -- just move
				set_cell(c)
				return 1 -- time taken to move
			end
			
		end
		return 0
	end

	function die()
		if is.player then -- we deaded
			local main=level.main
			main.soul.last_stairs=nil
			main.level=main.level.destroy()
			main.level=yarn_level.create(yarn_attrs.get("level.home",1,{xh=40,yh=28}),main)
			main.menu.hide()
			
			main.level.add_msg("You feel dead...")

		else
		
			local p=level.new_item( name.."_corpse" )
			p.set_cell( cell )

			level.del_item(d)
		end
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

