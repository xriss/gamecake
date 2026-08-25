

local wgrd=require("wetgenes.grd")
local bitdown=require("wetgenes.gamecake.fun.bitdown")

local codes="%s" -- any white space
for i=0,31 do
	local v=bitdown.cmap_swanky32[i]
	local c=v.code:sub(1,1)
	if c == c:match("%p+") then -- is char punctuation
		c="%"..c -- escape punctuation
	end
	codes=codes..c
end
codes="["..codes.."]+" -- all valid bitdown chars patterns

print("codes",codes) -- valid bitdown chars

print("searching text for bitdown images")

local funtext
do
	local fp=assert( io.open("lua/fun/poopeepanda.fun.lua") )
	funtext=assert(fp:read("*all"))
	assert(fp:close())
end

print("input size:",#funtext)

local parts={}
local part=""
local chunk
local append_part_string=function(s)
	if s=="" then return end
	if type(parts[#parts])=="string" then
		parts[#parts]=parts[#parts]..s
	else
		parts[#parts+1]=s
	end
end

local line_idx=1
local char_idx=0
for line in string.gmatch(funtext, "[^\n]*\n?") do -- all lines, last line will be ""

	if chunk then -- continue thinking
	
		part=part..line

		if line:sub(1,2)=="]]" then -- last line
			chunk.foot=line
			-- check bitmap
			local g=bitdown.pix_grd_idx(chunk.body)  -- convert from bitdown
			local s=bitdown.grd_pix_idx(g)           -- convert into bitdown
			if s==chunk.body then -- must match so we can recreate exactly
				parts[#parts+1]=chunk
				part=""
				chunk=nil
			else -- bad image
				chunk=nil
			end
		else -- continue body
			if line == line:match(codes) then -- possible image data
				chunk.body=chunk.body..line
			else
				chunk=nil -- failed to find
			end
		end
	
	else
		if line:sub(-3,-1)=="[[\n" then -- check for head line
			chunk={} -- start thinking
			chunk.head=line
			chunk.body=""
			append_part_string(part)
			part=""
		end
		part=part..line
	end

	line_idx=line_idx+1
	char_idx=char_idx+(#line)
end
append_part_string(part) -- final part



for idx,part in ipairs(parts) do
	if type(part)=="string" then
		print("STRING",idx,part)
	else
		print("BITMAP",idx)
		print("HEAD" , part.head )
		print("BODY" , part.body )
		print("FOOT" , part.foot )
	end
end




