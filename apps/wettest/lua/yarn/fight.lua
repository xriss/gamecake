
-- a monter or player or any other character, really just a slightly more active item
-- these are items that need to update as time passes

local _G=_G

local win=win

local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

local pairs=pairs
local setfenv=setfenv
local unpack=unpack
local require=require



--[[

air  evade water smother fire  burn   earth absorb  meta control

fire burn  air   evade   earth absorb water smother meta control

     air
     / \
fire     earth
    |   |
meta  -  water



wheel values, 0 to 1 where 0 and 1 are the same value

these are compared and produce a relative wheel value that goes

	0.5
	
0		1
	
	0.5



earth=0.0	-- absorbs
fire =0.2	-- burns
water=0.4	-- smothers
air  =0.6   -- evades
magic=0.8   -- controls

stone   =0.000
paper   =0.333
scissors=0.666

so

	
0.5 + (((a-b)*2)%1)

kinda ish

]]



module(...)

--
-- get the amount of damage, done by this char with their currently welded weapon
--
-- weapon will have a min-max damage amount we randoml pick an integer between these two numbers
--
function get_damage_char(c)

	local mn=c.attr.dam_min
	local mx=c.attr.dam_max
	
	local num=0
	
	if mx<=mn then
		num=mn
	else
		num=math.random(mn,mx)
	end
	
	return math.floor(num)
end

--
-- adjust the damage by this chars defense abilities, return the new damage number
--
function adjust_defense_char(c,damage)

	local ma=c.attr.def_add
	local mm=c.attr.def_mul
	
	damage=(damage+ma)*mm
	
	return math.floor(damage)
	
end


--
-- c1 hits c2 with the weapon in hand
--
-- adjust hitpoints trigger events etc etc
--
function hit(c1,c2)

	local damage=get_damage_char(c1)
	
	damage=adjust_defense_char(c2,damage)
	
	local hp=c2.attr.hp
	
	hp=hp-damage
	
	if hp<=0 then -- dead
		
		c2.attr.hp=0
	
		if c1.class=="player" then
		
			c1.level.add_msg("You hit for "..damage.." damage!")
			c1.level.add_msg("You killed "..(c2.attr.desc).." and won "..c2.attr.score.." points!")
			c1.attr.score=c1.attr.score+c2.attr.score
			c2.die()
			
		elseif c2.class=="player" then

			c1.level.add_msg("You took "..damage.." damage from "..(c1.attr.desc).." and died!")
			
		end
	
	else -- just hit
	
		c2.attr.hp=hp
		
		if c1.class=="player" then
		
			c1.level.add_msg("You hit for "..damage.." damage!")
			
		elseif c2.class=="player" then

			c1.level.add_msg("You took "..damage.." damage from "..(c1.attr.desc).." and now have "..c2.attr.hp.." health!")
			
		end
		
	end

end

