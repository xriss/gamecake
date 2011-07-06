
-- data to fill up attr with, this is the main game item/monster/logic content

local _G=_G


local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os
local package=package

local pairs=pairs
local setfenv=setfenv
local unpack=unpack
local require=require
local print=print
local tostring=tostring
local exit=exit

module(...)
local yarn_level=require("yarn.level")
local strings=require("yarn.strings")

local attrdata=require(...)

function ascii(a) return string.byte(a,1) end

function get(name,pow,xtra)

	if not dd[name] then return nil end -- no data

	pow=pow or 0 -- pow is a +1 -1 etc, base item adjustment
	
	local it={}
	
	local d=dd[name] -- get base
	
	for i,v in pairs(d) do it[i]=v end -- copy 1 deep only
	for i,v in pairs(d.powup or {} ) do it[i]=(it[i] or 0)+ math.floor(v*pow) end
		
	it.pow=pow -- remember pow
	
	for i,v in pairs(xtra or {}) do
		it[i]=v
	end
	
	local parents=strings.split(name,"%.")
	if parents[2] then -- there is a dot so inherit
		parents[#parents]=nil -- lose trailing part
		local parent=table.concat(parents,".") -- and build parents name
		return get(parent,pow,it) -- recurse
	end
	
-- make sure these exist
	it.call=it.call or {}
	it.can=it.can or {}
	
	return it
end

dd={

{
	name="cell",
},
{
	name="wall",
},
{
	name="floor",
},

{
	name="level",
},
{
	name="level.home",
},
{
	name="level.town",
},
{
	name="level.dump",
},

{
	name="room",
},

{
	name="player",
	form="char",
	class="player",
	asc=ascii("@"),
	desc="a human",
	hp=10,
	score=0,
	
	wheel=0,
	dam_min=20,
	dam_max=20,
	def_add=0,
	def_mul=1,
	
	can=
	{
		fight=true,
		make_room_visible=true,
		operate=true,
	},
	
},

{
	name="sensei",
	form="char",
	class="sensei",
	asc=ascii("&"),
	desc="a sensei",
	
	can=
	{
		use="menu",
	},
	
	call=
	{
		menu=function(it,by)
			it.level.main.menu.show_sensei_menu(it,by)
		end,
	}
},

{
	name="sensei.dump",
	sensei="dump",
	desc="a sensei named eeyore",
},

{
	name="stairs",
	form="char",
	class="stairs",
	asc=ascii("<"),
	desc="a doorstone inscribed, stairs",
	stairs="town",
	stairs_min=0,
	stairs_max=0,
	
	can=
	{
		use="menu",
	},
	
	call=
	{
		menu=function(it,by)
			it.level.main.menu.show_stairs_menu(it,by)
		end,
	}
},

{
	name="stairs.home",
	stairs="home",
	desc="a doorstone inscribed, home",
	stairs_min=1,
	stairs_max=1,
},

{
	name="stairs.dump",
	stairs="dump",
	desc="a doorstone inscribed, garbage dump",
	stairs_min=1,
	stairs_max=5,
},

{
	name="cryo_bed",
	form="char",
	class="story",
	asc=ascii("<"),
	desc="your SwordStone vault capsule",
	
	open=true,
	
	can=
	{
		use="menu",
	},
	
	call=
	{
		acts=function(it,by)
			local t={"read welcome","read license","look"}
			if it.open then
				t[#t+1]="close"
			else
				t[#t+1]="open"
			end
			return t
		end,
		
		look=function(it,by)
			it.level.main.menu.show_text(it.desc,
			"Your SwordStone vault capsule is ".. (it.open and "open" or "closed").."." )
		end,
		open=function(it,by)
			if not it.open then
				it.level.main.menu.show_text(it.desc,
				"Your SwordStone vault capsule is held shut by something inside.")
			end
		end,
		close=function(it,by)
			if it.open then
				it.attr.open=false
				it.level.main.menu.show_text(it.desc,
				"Your SwordStone vault capsule closes very very very slowly.")
				it.attr.asc=ascii("=")
			end
		end,
		
		["read welcome"]=function(it,by)
			it.level.main.menu.show_text("Welcome to YARN, where an @ is you",
[[
Press the CURSOR keys to move up/down/left/right.

Press SPACE bar for a menu or to select a menu item.

If you are standing near anything interesting press SPACE bar to interact with it.

Press SPACE to continue.
]])
		end,
		
		["read license"]=function(it,by)
			it.level.main.menu.show_text(it.desc,
[[
SwordStone technologies: Where your future, is our business.

Handling this license* creates a binding and unbreakable contract between SwordStone technologies and you.

Please remain calm.

Our patent pending hero from the past vault is guaranteed to create a successful hero.

Eventually.

*The full text of this license is copyright SwordStone technologies and cannot be reproduced here.
]])
		end,
		
	},
},

{
	name="cryo_door",
	form="char",
	class="story",
	asc=ascii("|"),
	desc="your SwordStone vault door",
	
	open=false,
	
	can=
	{
		use="menu",
	},
	
	call=
	{
		acts=function(it,by)
			local t={"look"}
			if it.open then
				t[#t+1]="close"
			else
				t[#t+1]="open"
			end
			return t
		end,
		
		look=function(it,by)
			it.level.main.menu.show_text(it.desc,"your SwordStone vault door is ".. (it.open and "open" or "closed") )
		end,
		open=function(it,by)
			local capsule=it.level.find_item("cryo_bed")
			if not capsule or capsule.open==true then
				it.level.main.menu.show_text(it.desc,"please ensure that your SwordStone vault capsule is closed before exiting your SwordStone vault")
			else
				it.attr.open=true
				it.attr.form="item"
				it.attr.asc=ascii("/"),
				it.call.look(it,by)
			end
		end,
		close=function(it,by)
			it.attr.open=false
			it.attr.form="char"
			it.attr.asc=ascii("|"),
			it.call.look(it,by)
		end,
	},
},


{
	name="ant",
	form="char",
	class="ant",
	asc=ascii("a"),
	desc="an ant",
	score=2,
	hp=2,
	
	wheel=0,
	dam_min=1,
	dam_max=2,
	def_add=0,
	def_mul=1,
	
	can=
	{
		fight=true,
		roam="random",
	},
	
	powup={
		score=2,
		hp=2,
		dam_min=0,
		dam_max=1,
		def_add=-1,
		def_mul=0,
		},
},


{
	name="blob",
	form="char",
	class="blob",
	asc=ascii("b"),
	desc="a blob",
	score=10,
	hp=10,
	
	wheel=0,
	dam_min=2,
	dam_max=4,
	def_add=0,
	def_mul=0.75,
	
	can=
	{
		fight=true,
		roam="random",
	},
	
	powup={
		score=10,
		hp=10,
		dam_min=2,
		dam_max=2,
		def_add=-1,
		def_mul=0,
		},
},



{
	name="ant_corpse",
	form="item",
	class="corpse",
	flavour="ant",
	asc=ascii("%"),
	weight=1,
	desc="a corpse of an ant",
},


{
	name="blob_corpse",
	form="item",
	class="corpse",
	flavour="blob",
	asc=ascii("%"),
	weight=1,
	desc="a corpse of a blob",
},

{
	name="rat",
	form="char",
	class="rat",
	asc=ascii("r"),
	desc="a rat",
	score=10,
	hp=10,
	
	wheel=0,
	dam_min=2,
	dam_max=4,
	def_add=0,
	def_mul=0.75,
	
	can=
	{
		fight=true,
		roam="random",
	},
	
	powup={
		score=10,
		hp=10,
		dam_min=2,
		dam_max=2,
		def_add=-1,
		def_mul=0,
		},
},



{
	name="rat_corpse",
	form="item",
	class="corpse",
	flavour="rat",
	asc=ascii("%"),
	weight=1,
	desc="a corpse of a rat",
},

}

-- swing data both ways
for i,v in ipairs(dd) do

	dd[ v.name ] = v -- look up by name
	v.id=i -- every data gets a unique id

end



