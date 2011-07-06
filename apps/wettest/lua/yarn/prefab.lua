
-- pre fabricated rooms, dungeon building for the use of

local _G=_G


local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local setfenv=setfenv
local unpack=unpack
local require=require

local dbg=dbg or function()end

module(...)

local yarn_strings=require("yarn.strings")
local yarn_attrdata=require("yarn.attrdata")


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
		
			d.level.cellfind[d.name]=d.cell -- last generated cell of this type
			
			local l=d.level.celllist[d.name] or {} -- all generated cells of this type
			l[#l+1]=d.cell
			d.level.celllist[d.name]=l
			
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
	
	local r
	if pow==0 then -- level 0 is always town no matter what the name
	
		r=add_room(get_room("home_stairs"))
		r=add_room(get_room("dump_stairs"))
		r=add_room(get_room("pub"))
		r=add_room(get_room("bank"))
		r=add_room(get_room("shop"))
		r=add_room(get_room("hotel"))
		
		opts.mode="town"
		opts.only_these_rooms=true
	
	elseif name=="level.home" then
	
		r=add_room(get_room("home_stairs"))
		r=add_room(get_room("home_bedroom"))
		r=add_room(get_room("home_mainroom"))
		
	elseif name=="level.dump" then

		r=add_room(get_room("dump_stairs"))
	
	else

		r=add_room(get_room("stairs"))
	
	end
	
	return opts

end

