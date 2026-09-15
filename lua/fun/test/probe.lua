

local wgrd=require("wetgenes.grd")
local bitdown=require("wetgenes.gamecake.fun.bitdown")
local bitdown_parse=require("wetgenes.gamecake.fun.bitdown_parse")


print("searching text for bitdown images")

local funtext
do
	local fp=assert( io.open("lua/fun/comicpanda.fun.lua") )
	funtext=assert(fp:read("*all"))
	assert(fp:close())
end

print("input size:",#funtext)

local parts=bitdown_parse.scan_parts_from_text(funtext)

if not parts then
	print("no bitdown found")
end

print( bitdown_parse.render_string_from_parts(parts) )

print( bitdown_parse.debug_parts_string(parts) )

local g=bitdown_parse.render_grd_from_parts(parts)
g:save({fmt="png",filename="test.png"})
