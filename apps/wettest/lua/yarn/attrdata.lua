
-- data to fill up attr with, this is the main game item/monster/logic content

-- functions into locals
local assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- libs into locals
local coroutine,package,string,table,math,io,os,debug=coroutine,package,string,table,math,io,os,debug

module(...)
local yarn_fight=require("yarn.fight")
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

local call_fight={
	acts=function(it,by)
		if it.can.fight and by.can.fight then return {"hit","look"} end
		return {"look"}
	end,
	hit=function(it,by)
		if it.can.fight and by.can.fight then yarn_fight.hit(by,it) end
		it.level.menu.hide()
		it.level.step(1)
--		it.level.update()
	end,
	look=function(it,by)
		it.level.menu.show_text(it.desc,it.longdesc or it.desc)
	end,
}

local call_talk={
	acts=function(it,by)
		if by.can.operate then return {"talk","look"} end
		return {"look"}
	end,
	talk=function(it,by)
		if by.can.operate then
			it.level.menu.show_talk_menu(it,by)
		end
	end,
	look=function(it,by)
		it.level.menu.show_text(it.desc,it.longdesc or it.desc)
	end,
}

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
	name="level.test",
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
		use="talk",
	},
	
	call=call_talk,

	chat={
		["welcome"]={
			text=[[Anybody there?]],
			says={"hello"},
		},
		["hello"]={
			text=[[Why hello there.]],
			says={"TTFN"},
		},
	},
},

{
	name="sensei.dump",
	sensei="dump",
	desc="a sensei named hobob",
	longdesc="Although it must be assumed that this is a person it looks more like a walking talking ball of fluff wearing leather Y-fronts.",
	chat={
		["welcome"]=function(it,by)
			if it.level.name=="level.dump" then
				if it.level.pow==1 then
					return it.chat["welcome.0"]
				end
			end
			return it.chat["welcome.0"]
		end,
		["welcome.0"]={
			text=[[That you, Maud?]],
			says={{say="hello",text="Were you expecting the queen?"},{say="maud",text="Maud?"}},
		},
		["hello"]={
			text=[[You sure took your time. You know I can't stand being by myself.]],
			says={"TTFN"},
		},
		["maud"]={
			text=[[Oh, nevermind. It must be the fumes. They get things all confused with each other.]],
			says={"TTFN"},
		},
	},
},

{
	name="sensei.twin1",
	sensei="dump",
	desc="a young girl",
	longdesc="A normal looking young girl with long braids on each side of her head, fashioned from her dark brown hair.",
	chat={
		["welcome"]={
			text=[[Would you like to play with us?]],
			says={{say="yes",text="Sure, why not."},{say="no",text="I think i'll pass."}},
		},
		["yes"]={
			text=[[Oh, good! I've been waiting for someone to try this new game I'll think of in a while.]],
			says={"If you say so."},
		},
		["no"]={
			text=[[You should really speak to my sister.]],
			says={"If you say so"},
		},
	},
},

{
	name="sensei.twin2",
	sensei="dump",
	desc="a young girl",
	longdesc="Although it looks as if this young girl looks eerily similar to the other one, she actually does.",
	chat={
		["welcome"]={
			text=[[Would you like to play with us?]],
			says={{say="yes",text="What if i said yes?"},{say="no",text="No, thanks."}},
		},
		["yes"]={
			text=[[Then you should speak to my sister.]],
			says={"Ok, I'll do that now."},
		},
		["no"]={
			text=[[Well, I really think you should speak to my sister.]],
			says={"Fine."},
		},
	},
},

{
	name="sensei.test",
	sensei="test",
	desc="a sensei named chester",
	longdesc="Cauliflower ears are the least of his worries, worries that include broccoli nose and turnip tongue. One can only assume that the best years of his boxing career are far, far behind him.",
	chat={
		["welcome"]=function(it,by)
			if it.level.name=="level.test" then
				if it.level.pow==1 then
					return it.chat["welcome.1"]
				end
			end
			return it.chat["welcome.0"]
		end,
		["welcome.0"]={
			text=[[Anybody there?]],
			says={"hello",{say="nobody",text="There's nobody here but us chickens."}},
		},
		["question"]={
			sticky=true,
			text=[[
			Ya wanna me ta learn ya somink?
			Well do ya punk?
			Do ya?]],
			says={{say="teach",text="Teach me, oh great one."},{say="no",text="Nope."}},
		},
		["hello"]={
			text=[[
			!!!
			Gorden Freeman Bennett, don't be suprisin' on me like that.
			]],
			says={{say="question",text="..."}},
		},
		["nobody"]={
			text=[[
			Good to hear it.
			I can't be 'avin with ninjas sneaking up on me all sneaky like.
			The smoke bombs play 'avok with me poor sinuses.
			]],
			says={{say="question",text="..."}},
		},
		["teach"]={
			text=[[
			Well young padawan, you could work it all out yourself but if you really want a bit of the old toot then nip over to that doorstone next to me and pick a level.
			
			I'll follow you down and let you know what you 'ave to be doing.
			
			Down there is my domain and you may not take anything with you nor bring anything back.
			
			Still it's all fun and games, init?
			]],
			says={"..."},
		},
		["welcome.1"]={
			text=[[
			Here you must learn the importance of items and equipment.
			
			Without tools we are but shaved monkeys.
			
			With tools we are shaved monkeys in suits.
			
			Now go forth and equip yourself with the weapon that lays yonder and find the mass damon hordes.
			
			Destroy them all then return to me victorious.
			
			Or just nip back upstairs, your call blud.
			]],
			says={"..."},
		},
	},
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
		acts=function(it,by)
			if by.can.operate then return {"menu"} end
		end,
		look=function(it,by)
			it.level.menu.show_text(it.desc,it.longdesc or it.desc)
		end,
		menu=function(it,by)
			it.level.menu.show_stairs_menu(it,by)
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
	name="stairs.test",
	stairs="test",
	desc="a doorstone inscribed, testing 123",
	stairs_min=1,
	stairs_max=1,
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
			it.level.menu.show_text(it.desc,
			"Your SwordStone vault capsule is ".. (it.open and "open" or "closed").."." )
		end,
		open=function(it,by)
			if not it.open then
				it.level.menu.show_text(it.desc,
				"Your SwordStone vault capsule is held shut by something inside.")
			end
		end,
		close=function(it,by)
			if it.open then
				it.attr.open=false
				it.level.menu.show_text(it.desc,
				"Your SwordStone vault capsule closes very very very slowly.")
				it.attr.asc=ascii("=")
			end
		end,
		
		["read welcome"]=function(it,by)
			it.level.menu.show_text("Welcome to YARN, where an @ is you",
[[
Press the CURSOR keys to move up/down/left/right.

Press SPACE bar for a menu or to select a menu item.

If you are standing near anything interesting press SPACE bar to interact with it.

Press SPACE to continue.
]])
		end,
		
		["read license"]=function(it,by)
			it.level.menu.show_text(it.desc,
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
			it.level.menu.show_text(it.desc,"your SwordStone vault door is ".. (it.open and "open" or "closed") )
		end,
		open=function(it,by)
			local capsule=it.level.find_item("cryo_bed")
			if not capsule or capsule.open==true then
				it.level.menu.show_text(it.desc,"please ensure that your SwordStone vault capsule is closed before exiting your SwordStone vault")
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
	
	call=call_fight,
	
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
	
	call=call_fight,
	
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

	call=call_fight,
	
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

{
	name="rat_tail",
	form="item",
	class="meat",
	flavour="rat",
	asc=ascii(","),
	weight=0.1,
	desc="a rats tail",
},
{
	name="rat_tooth",
	form="item",
	class="meat",
	flavour="rat",
	asc=ascii(","),
	weight=0.1,
	desc="a rats tooth",
},

{
	name="pointy_stick",
	form="item",
	class="weapon",
	flavour="stick",
	asc=ascii("!"),
	weight=1,
	desc="a pointy stick",
},

}

-- swing data both ways
for i,v in ipairs(dd) do

	dd[ v.name ] = v -- look up by name
	v.id=i -- every data gets a unique id

end



