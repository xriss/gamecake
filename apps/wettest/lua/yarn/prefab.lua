
-- pre fabricated rooms, dungeon building for the use of

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

local dbg=dbg or function()end

module(...)

local yarn_strings=require("yarn.strings")
local yarn_attrdata=require("yarn.attrdata")

-- names that have a .in them are sub classes
-- we need to be able to find them using their subclass
-- so turn a name with . into a list of possible names
function keys_name_and_subnames(s)
	local splits={}
	local i=1
	repeat
		i=s:find(".",i,true)
		if i then
			splits[ #splits+1 ] = s:sub(1,i-1)
			i=i+1
		end
	until not i
	splits[ #splits+1 ]=s -- add all last
	return splits
end

strings={}
keys={}


local function room(name,map,key)
	if map then
		strings[name]=map
	end
	if key then
		keys[name]=key
	end
end


-- basic key, every map string uses this by default and then adds more or overides
keys.base={
	["# "]="wall",
	[". "]="space",
	["- "]="item_spawn",
	["= "]="bigitem_spawn",
	["@ "]="player_spawn",
	["< "]="stairs",
}


room("bigroom",[[
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
]])

room("pub",[[
# # # # # # # #
# . . . . . . #
# . = = = = . #
# . = = = = . #
# . . . . . . #
# # # # # # # #
]])

room("bank",[[
# # # # # #
# . . . . #
# . = = . #
# . = = . #
# . . . . #
# # # # # #
]])

room("shop",[[
# # # # # #
# . . . . #
# . = = . #
# . = = . #
# . = = . #
# . = = . #
# . . . . #
# # # # # #
]])

room("hotel",[[
# # # # # # #
# . . . . . #
# . = = = . #
# . = = = . #
# . = = = . #
# . . . . . #
# # # # # # #
]])


room("home_bedroom",[[
# # # # # # # # # #
# . . . . . . . . #
# . # # # # # # . #
# . # =1@ . . =2. #
# . # # # # # # . #
# . . . . . . . . #
# # # # # # # # # #
]],{
	["=1"]="cryo_bed",
	["=2"]="cryo_door",
})

room("home_mainroom",[[
# # # # # # # # # #
# . . . . . . . . #
# . = = . . = = . #
# . = = . . = = . #
# . = = . . = = . #
# . = = . . = = . #
# . . . . . . . . #
# # # # # # # # # #
]])

room("home_stairs",[[
# # # # # # #
# . . . . . #
# . # # # . #
# . . < # . #
# . # # # . #
# . . . . . #
# # # # # # #
]],{
	["< "]="stairs.home",
})

room("dump_stairs",[[
# # # # # #
# . . . . #
# . < @1. #
# . . . . #
# # # # # #
]],{   
	["< "]="stairs.dump",
	["@1"]="sensei.dump",
})

room("test_stairs",[[
# # # # # #
# . . . . #
# . < @1. #
# . . . . #
# # # # # #
]],{   
	["< "]="stairs.test",
	["@1"]="sensei.test",
})

room("test_stairs_1",[[
# # # # # # # # # # # #
# . . . . . . . . . . #
# . . . . . . . . . . #
# . . < @1. . . ! . . #
# . . . . . . . . . . #
# . . . . . . . . . . #
# # # # # # # # # # # #
]],{   
	["< "]="stairs.test",
	["@1"]="sensei.test",
	["! "]="pointy_stick",
})

room("test_lair_1",[[
# # # # # # # # # # # #
# . . . . . . . . . . #
# . . . . . . . . . . #
# . . r r r r r r . . #
# . . . . . . . . . . #
# . . . . . . . . . . #
# # # # # # # # # # # #
]],{   
	["r "]="rat",
})

room("stairs",[[
# # # # #
# . . . #
# . < . #
# . . . #
# # # # #
]],{
	["< "]="stairs",
})

function string_to_room(s,key)

	if not key then key=keys.base end

	local r={}

	local lines=yarn_strings.split_lines(s)
	for i,v in ipairs(lines) do lines[i]=yarn_strings.trim(v).." " end -- trim, but add space back on end
	
	local xh=0
	for i,v in ipairs(lines) do if #v>xh then xh=#v end end -- find maximum line length

	local ls={}
	for i,v in ipairs(lines) do if #v==xh then ls[#ls+1]=v end end -- only keep lines of this length
	local yh=#ls
	xh=math.floor(xh/2) -- 2 chars to one cell
	
	xh=xh-2
	yh=yh-2
	
	if xh<0 then xh=0 end
	if yh<0 then yh=0 end
	
	r.xh=xh
	r.yh=yh
	
	r.name="unnnamed"
	
	r.cells={}
	for n=2,#ls-1 do -- skip top/bottom line
		local l=ls[n]
		local t={}
		r.cells[ #r.cells+1 ]=t
		for i=1+2,#l-2,2 do -- skip left/right chars
			local ab=l:sub(i,i+1)
			t[#t+1]=key[ab] or keys.base[ab] or "space"
		end
	end
	return r
end


function get_room(name)

	local r

	if strings[name] then	
		r=string_to_room( strings[name] , keys[name] )
	end

	return r
end



function map_opts(name,pow)

	local opts={}
	opts.rooms={} -- required rooms for this map

	local function callback(d) -- default callback when building maps
		if d.call=="cell" then
		
			for _,n in ipairs(keys_name_and_subnames(d.name)) do
			
				d.level.cellfind[n]=d.cell -- last generated cell of this type
				
				local l=d.level.celllist[n] or {} -- all generated cells of this type
				l[#l+1]=d.cell
				d.level.celllist[n]=l
				
			end
			
			local at
			if d.name=="wall" then
				d.cell.set.name("wall")
			else
				at=yarn_attrdata.get(d.name)
			end
			if at then
				local it=d.level.new_item( at )
				if it then
					it.set_cell( d.cell)
				end
			end
		end
	end
	
	local function add_room(r)
		opts.rooms[#opts.rooms+1]=r
		r.callback=callback
		return r
	end


	local function generate_player(level)
		level.player=level.new_item( "player" )
		level.player.is.soul=level.main.soul -- we got soul
		level.player.set_cell( level.cellfind["player_spawn"] or level.rand_room_cell({}) )
	end
	
	local function generate_player_bystairs(level)
		level.player=level.new_item( "player" )
		level.player.is.soul=level.main.soul -- we got soul
		
		local stairs
		if level.soul.last_stairs then -- aim to stick to the same stairs
			stairs=level.cellfind[level.soul.last_stairs]
dbg("fond real stairs : "..tostring(stairs))
		end
		if not stairs then stairs=level.cellfind["stairs"] end
		
		if stairs then
			for i,v in stairs.neighbours() do
				if v.is_empty() then --empty so place palyer here
					level.player.set_cell( v )
					break
				end
			end
		else -- if we got here then just pick a random place
			level.player.set_cell( level.rand_room_cell({}) )
		end
	end

	local function generate_ants(level)
		for i=1,10 do
			local c=level.rand_room_cell({})
			if not c.char then
				local p=level.new_item( "ant" )
				p.set_cell( c )
			end
		end
	end

	local function generate_blobs(level)
		for i=1,5 do
			local c=level.rand_room_cell({})
			if not c.char then
				local p=level.new_item( "blob" )
				p.set_cell( c )
			end
		end
	end

--default generation	
	opts.generate=function(level)
		generate_player(level)
		generate_ants(level)
		generate_blobs(level)
	end
	
	local r
	if pow==0 then -- level 0 is always town no matter what the name
	
		r=add_room(get_room("home_stairs"))
		r=add_room(get_room("dump_stairs"))
		r=add_room(get_room("test_stairs"))
		r=add_room(get_room("pub"))
		r=add_room(get_room("bank"))
		r=add_room(get_room("shop"))
		r=add_room(get_room("hotel"))
		
		opts.mode="town"
		opts.only_these_rooms=true

		opts.generate=function(level)
			generate_player_bystairs(level)
		end
	
	elseif name=="level.home" then
	
		r=add_room(get_room("home_stairs"))
		r=add_room(get_room("home_bedroom"))
		r=add_room(get_room("home_mainroom"))
		
		opts.generate=function(level)
		
			if level.soul.capsule_done then
				generate_player_bystairs(level)
			else
				generate_player(level)
			end
			
			level.soul.capsule_done=true
			
		end
		
		
	elseif name=="level.dump" then

		r=add_room(get_room("dump_stairs"))

			opts.generate=function(level)
			
				generate_player(level)
				generate_ants(level)
				generate_blobs(level)
				
			end
	
	elseif name=="level.test" then

		if pow==1 then
		
			r=add_room(get_room("test_stairs_1"))
			r=add_room(get_room("test_lair_1"))
			
			opts.generate=function(level)
				generate_player_bystairs(level)
			end

		else
		
			r=add_room(get_room("test_stairs"))
			
		end
	
	else

		r=add_room(get_room("stairs"))
	
	end
	
	
	return opts

end

