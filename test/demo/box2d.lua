

local box2d=require("box2d")

-- important simulation values
local METER=16
local SQR_METER=METER*METER

local var_tostring=function(v)
	if type(v)=="table" then -- any tables will be arrays of numbers
		if v.is then
			local a={}
			for n,v in pairs(v) do a[#a+1]=n.."="..tostring(v) end
			return "{ "..table.concat(a," , ").." }"
		else
			return "{ "..table.concat(v," , ").." }"
		end
	else
		local s=tostring(v)
		-- the second byte of a boxid is almost certainly 0
		-- unless you create more than 255 worlds, which you cant
		-- without changing the box defaults
		if s:find("\0") then -- find and convert boxids to hex
			s="0x"..s:gsub(".", function(c) return ("%02x"):format(c:byte()) end)
		end
		return s
	end
end
local dump_vars=function(vs,prefix)
	local t={}
	for n,v in pairs(vs) do
		t[#t+1]=string.format("%s%-32s = %s",prefix or "",n,var_tostring(v))
	end
	table.sort(t)
	print( table.concat(t,"\n") )
end


-- adjust default box2d variables
box2d.set({
	LengthUnitsPerMeter=METER,
})

print("box2d")
dump_vars(box2d.info(),"box2d\t")

-- create a world
local world=box2d.world({
	gravity={0,10*METER,0},
})
print("world")
dump_vars(world:info(),"world\t")

-- create body
local bodyA=world:body({
})
print("bodyA")
dump_vars(bodyA:info(),"bodyA\t")

local shapeA=bodyA:shape({
	shape="circle",
	radius=1*METER,
})
print("shapeA")
dump_vars(shapeA:info(),"shapeA\t")

-- create body
local bodyB=world:body({
})
print("bodyB")
dump_vars(bodyB:info(),"bodyB\t")

local shapeB=bodyB:shape({
	shape="circle",
	radius=1*METER,
})
print("shapeB")
dump_vars(shapeB:info(),"shapeB\t")

local joint=world:joint({
	joint="distance",
	bodyA=bodyA,
	localFrameA={0,0,0},
	bodyB=bodyB,
	localFrameB={0,0,0},
	forceThreshold=0,
})
print("joint")
dump_vars(joint:info(),"joint\t")

local hits=world:cast_ray({
	origin={0,-100*METER},
	translation={0,100*METER},
})
print("raycast")
for i,hit in ipairs(hits) do
	dump_vars(hit,"hit"..i.."\t")
end

-- advance a second
world:step(1,256)

print("world")
dump_vars(world:info(),"world\t")
print("bodyA")
dump_vars(bodyA:info(),"bodyA\t")
print("bodyB")
dump_vars(bodyB:info(),"bodyB\t")
