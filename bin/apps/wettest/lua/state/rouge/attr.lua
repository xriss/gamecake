
-- shared attributes, across cells, items and chars

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



local function print(...) _G.print(...) end


module("state.rouge.attr")


function create(t)

	
local d={}
setfenv(1,d)

	set={}
	get={}
	
	asc=t.asc
	hp=t.hp
	hpmax=hp
	score=t.score
	desc=t.desc
	
	wheel=t.wheel
	dam_min=t.dam_min
	dam_max=t.dam_max
	def_add=t.def_add
	def_mul=t.def_mul

	
	function set.visible(v) visible=v end
	
	function get.visible() return visible end

	return d
	
end

