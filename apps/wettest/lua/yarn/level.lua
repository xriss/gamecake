
-- a collection of everything


local _G=_G

local debug=debug
local table=table
local ipairs=ipairs
local pairs=pairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local require=require
local type=type
local setmetatable=setmetatable

module(...)
local yarn_map=require("yarn.map")
local yarn_room=require("yarn.room")
local yarn_cell=require("yarn.cell")
local yarn_item=require("yarn.item")
local yarn_attr=require("yarn.attr")

local yarn_prefab=require("yarn.prefab")
local attrdata=require("yarn.attrdata")


function create(t,up)

local d={}
setfenv(1,d)

	attr=yarn_attr.create(t)
	metatable={__index=attr}
	setmetatable(d,metatable)

	level=d -- we are the level
	main=up

	time_passed=0
	time_update=0
	
	xh=t.xh or 30
	yh=t.yh or 30

	rooms={}
	items={}
		
	cells={}
	cellfind={}
	celllist={}
	
	call=call or {}

-- create blank cells

	for y=0,yh-1 do
		for x=0,xh-1 do
			local i=x+y*xh
			cells[i]=yarn_cell.create(attrdata.get("cell",0,{ level=d, xp=x, yp=y, id=i }))
		end
	end

--	function draw_map(m) map.draw_map(m) end
	function get_asc(x,y)
		local cell=get_cell(x,y)
		return (cell and cell.asc())
	end

-- use to find a unique named item in this level
	function find_item(name)
		for v,b in pairs(items) do
			if v.name==name then
				return v
			end
		end
	end
	
	function get_cell(x,y)
		if x<0 then return nil end
		if x>=xh then return nil end
		if y<0 then return nil end
		if y>=yh then return nil end
		return cells[ x+y*xh ]
	end

-- iterate an area of cells	
	function cpairs(x,y,w,h)
		return function(a,i)
			local px=i%w
			local py=(i-px)/w
			if py>=h then return end
			return i+1,get_cell(x+px,y+py)
		end, cells, 0
	end
	
	function new_item(n,l)
		local at
		if type(n)=="string" then
			at=attrdata.get(n,l)
		else
			at=n
			n=at.name
		end
		local it=yarn_item.create( at ,d)
		items[it]=true -- everything lives in items list
		
		for i,v in pairs(it.call) do -- every item puts its call functions in the levels call table
			d.call[i]=v
		end
		-- this means we can easily add uniquely named call functions to a level using any item

		return it
	end
	function del_item(it)
		items[it]=nil
		it.del()
	end
	
	function rand(a,b)
		if a>=b then return a end
		return math.random(a,b)
	end

-- get a random room	
	function rand_room(t)
		local n=0
		for i,v in ipairs(rooms) do if v.xh>1 and v.yh>1 then n=n+1 end end -- count rooms
		n=rand(1,n)
		for i,v in ipairs(rooms) do
			if v.xh>1 and v.yh>1 then
				n=n-1
				if n<1 then return v end -- found it
			end
		end
	end
	
-- get a random cell in the given range
	function rand_cell(t)
		local x=rand(t.xp,t.xp+t.xh-1)
		local y=rand(t.yp,t.yp+t.yh-1)
		return get_cell(x,y)
	end

-- get a random cell in a random room

	function rand_room_cell(t)
		return rand_cell(rand_room(t))
	end
	
	do
		local opts=yarn_prefab.map_opts(d.name,d.pow)
		opts.xh=d.xh
		opts.yh=d.yh
		map=yarn_map.create(opts) -- create an empty map, this is only a room layout
	end
	
-- now turn that generated map into real rooms we can put stuff in
	for i,v in ipairs(map.rooms) do
		rooms[i]=yarn_room.create(attrdata.get("room",0,
			{ level=d, xp=v.x, yp=v.y, xh=v.xh, yh=v.yh, }) )
		rooms[i].opts=v.opts
	end


-- find link door locations	
	for i,v in ipairs(rooms) do v.find_doors() end
	

	
	for i,v in ipairs(rooms) do
		v.post_create()
	end
	
	for i,r in ipairs(rooms) do
		if r.opts then -- special?
			local cs=r.opts.cells
			for y=1,#cs do
				local v=cs[y]
				for x=1,#v do
					local n=v[x]
					
					if n=="space" then -- do nothing
					else
						local c=get_cell(r.xp+x-1,r.yp+y-1)
						if r.opts.callback then
							r.opts.callback({call="cell",cell=c,name=n,level=d,room=r})
						end
					end
				end
			end
			if r.opts.callback then
				r.opts.callback({call="room",level=d,room=r})
			end
		end
	end
	
	
	if map.opts.bigroom then

--		rooms[#rooms+1]=yarn_room.create(attrdata.get("room",0,
--			{ level=d, xp=1, yp=1, xh=xh-2, yh=yh-2, }) )

		for y=0,yh-1 do
			for x=0,xh-1 do
				local i=x+y*xh
				local cell=cells[i]
				if map.room_find(x,y)==map.bigroom then
					cell.set.name("floor")
					cell.attr.set.visible(true)
				end
				if y==0 or y==yh-1 or x==0 or x==xh-1 then
					cell.attr.set.visible(true)
				end
			end
		end
		for i,r in ipairs(rooms) do
			if r.xh>1 and r.yh>1 then -- not corridors
				for x=r.xp-1,r.xp+r.xh do
					for y=r.yp-1,r.yp+r.yh do
						if x==r.xp-1 or x==r.xp+r.xh or y==r.yp-1 or y==r.yp+r.yh then
							local cell=get_cell(x,y)
							cell.attr.set.visible(true)
						end
					end
				end
			end
		end
	end
	
	player=new_item( "player" )
	player.set_cell( cellfind["player_spawn"] or rand_room_cell({}) )
	player.attr.soul=main.soul -- we got soul
	
	for i=1,10 do
		c=rand_room_cell({})
		if not c.char then
			local p=new_item( "ant" )
			p.set_cell( c )
		end
	end

	for i=1,5 do
		c=rand_room_cell({})
		if not c.char then
			local p=new_item( "blob" )
			p.set_cell( c )
		end
	end
	
	key_repeat=nil
	key_repeat_count=0
	
	function key_clear()
		key_repeat=nil
	end
	
	function key_check()
		key_repeat_count=key_repeat_count+1
		if key_repeat_count>=10 then
			key_do(key_repeat)
		end
	end
	
	function key_do(key)
	
		if key=="space" or key=="enter" then
		
			up.menu.show_player_menu(player)
			
		end
		
		key_repeat_count=0 -- always zero the repeat counter
	
		local vx=0
		local vy=0
		
		if key=="up" then
			vx=0
			vy=-1
		elseif key=="down" then
			vx=0
			vy=1
		elseif key=="left" then
			vx=-1
			vy=0
		elseif key=="right" then
			vx=1
			vy=0
		end
		
		if vx~=0 or vy~=0 then
			time_update=time_update+player.move(vx,vy)
			return true
		end
	end
	
	function keypress(ascii,key,act)
	
		if act=="down" then
		
			key_repeat=key
			key_repeat_count=0
			
			key_do(key_repeat)
			
		elseif act=="up" then -- an up key cancels all repeats
		
			if key_repeat==key then
				key_repeat=nil
			end
		end
		
	end
	
	function update()
		key_check()
		
		if time_update==0 then return 0 end
--print(time_passed)

-- regen health?
--		player.attr.hp=math.floor(player.attr.hp+time_update)
--		if player.attr.hp > player.attr.hpmax then player.attr.hp = player.attr.hpmax end
		
		for v,b in pairs(items) do
			v.update()
		end
		
		if display_msg_time<time_passed then -- report your most important stats in msg form
		
			local item=player.cell.get_item()
			
			if item then -- standing on an item
				set_msg(item.view_text())
			elseif player.attr.hp~=player.attr.hpmax then
				set_msg("Your health is ".. player.attr.hp .."/".. player.attr.hpmax )
			else
				set_msg("You have scored "..player.attr.score .." points.")
			end
		end

		local t=time_update
		time_passed=time_passed+time_update
		time_update=0
		return t
	end
	
	display_msg=nil
	display_msg_time=0
	function set_msg(a)
		display_msg=a
		display_msg_time=time_passed
	end
	function add_msg(a)
		if display_msg_time<time_passed then display_msg=nil end -- do not add to previously displayed msgs
		if display_msg then display_msg=display_msg.." " else display_msg="" end
		display_msg=display_msg..a
		display_msg_time=time_passed
	end
	function get_msg()
		return display_msg or ""
	end

	function destroy()
	end

-- create a save state for this data
	function save()
		local sd={}
		
		sd.attr=yarn_attr.save(attr)
		
		sd.rooms={}
		sd.cells={}
		
		for i,v in ipairs(cells) do -- cells contain all items so they get saved here
			sd.cells[i]=v.save()
			if sd.cells[i].attr.name=="wall" then
				if not sd.cells[i].attr.visible then
					if not sd.cells[i].items then
						sd.cells[i]=nil				-- do not need to save
					end
				end
			end
		end
		
		for i,v in ipairs(rooms) do -- rooms are just areas of cells
			sd.rooms[i]=v.save()
		end
		
		return sd
	end

-- reload a saved data (create and then load)
	function load(sd)
		d.attr=yarn_attr.load(sd.attr)
		d.metatable.__index=attr
	end
	
	return d
	
end

