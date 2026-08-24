

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
codes="["..codes.."]+" -- all valid bitdown chars patters

print("codes",codes) -- valid bitdown chars

print("searching text for bitdown images")

local funtext
do
	local fp=assert( io.open("lua/fun/poopeepanda.fun.lua") )
	funtext=assert(fp:read("*all"))
	assert(fp:close())
end

print("input size:",#funtext)

local chunks={}
local chunk

local line_idx=1
local char_idx=0
for line in string.gmatch(funtext, "[^\n]*\n?") do

	if chunk then -- continue thinking
	
		if line:sub(1,2)=="]]" then -- last line
			chunk.foot=line
			chunk.foot_line_idx=line_idx
			chunk.foot_char_idx=char_idx
			chunks[#chunks+1]=chunk
			chunk=nil
		else -- continue body
			if line == line:match(codes) then -- possible image data
				chunk.body=chunk.body..line
				chunk.body_lines=chunk.body_lines+1
				chunk.body_size=chunk.body_size+(#line)
			else
				chunk=nil -- failed to find
			end
		end
	
	else
		if line:sub(-3,-1)=="[[\n" then -- check for head line
			chunk={} -- start thinking
			chunk.head=line
			chunk.head_line_idx=line_idx
			chunk.head_char_idx=char_idx
			chunk.body=""
			chunk.body_line_idx=line_idx+1
			chunk.body_char_idx=char_idx+(#line)
			chunk.body_lines=0
			chunk.body_size=0
		end
	end

	line_idx=line_idx+1
	char_idx=char_idx+(#line)
end

print("found chunks:",#chunks)


for idx,chunk in ipairs(chunks) do
	print("CHUNK",idx)
	print( chunk.head_line_idx , chunk.head )
	print( chunk.body )
	print( chunk.foot )
end





