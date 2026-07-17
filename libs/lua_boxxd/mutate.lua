#!/usr/bin/env gamecake

-- start by 2d to 3d only with a plan to mutate both ways...

-- all input changes should be checked into git before running this
-- since the output files are checked into git then we can use git
-- to revert and check changes

mutate_from = mutate_from or "2"
mutate_into = mutate_into or "3"


-- input output filenames

mutate_files={
	{  "code/box"..mutate_from.."d.lua"    ,  "code/box"..mutate_into.."d.lua"    , },
	{  "code/lua_box"..mutate_from.."d.c"  ,  "code/lua_box"..mutate_into.."d.c"  , },
}


-- start with some basic string replacement, fixes plenty
-- provided we are careful with how we do everything

mutate_strings={
}

if mutate_from=="2" then
	for i,v in ipairs({
	-- swap the defines around
	{  "#define BOX_2 2"        ,  "#define BOX_3 3"        , },
	{  "#define BOX_V_COUNT 2"  ,  "#define BOX_V_COUNT 3"  , },
	{  "#define BOX_R_COUNT 1"  ,  "#define BOX_R_COUNT 4"  , },
	-- swap generic names
	{  "b2"                     ,  "b3"                     , },
	{  "B2"                     ,  "B3"                     , },
	{  "box2"                   ,  "box3"                   , },
	{  "Vec2"                   ,  "Vec3"                   , },
	}) do
		mutate_strings[#mutate_strings+1]=v	
	end
else
	for i,v in ipairs({
	-- swap the defines around
	{  "#define BOX_3 3"        ,  "#define BOX_2 2"        , },
	{  "#define BOX_V_COUNT 3"  ,  "#define BOX_V_COUNT 2"  , },
	{  "#define BOX_R_COUNT 4"  ,  "#define BOX_R_COUNT 1"  , },
	-- swap generic names
	{  "b3"                     ,  "b2"                     , },
	{  "B3"                     ,  "B2"                     , },
	{  "box3"                   ,  "box2"                   , },
	{  "Vec3"                   ,  "Vec2"                   , },
	}) do
		mutate_strings[#mutate_strings+1]=v
	end
end


for _ , filenames in ipairs(mutate_files) do

	local from_filename , into_filename = filenames[1] , filenames[2]

	print("Mutating from "..from_filename.." into "..into_filename)
	
	local fp=assert( io.open(from_filename,"rb") )
	local data=fp:read("*all")
	fp:close()
	
	for _,ss in ipairs(mutate_strings) do
		data=data:gsub(ss[1],ss[2])
	end

	local fp=assert( io.open(into_filename,"wb") )
	fp:write(data)
	fp:close()
end
