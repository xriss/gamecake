

module(...,package.seeall)

local wtxt=require("wetgenes.txt")
local wtxtlex=require("wetgenes.txt.lex")
local wstr=require("wetgenes.string")


function test_lex_lua()

	local lex=wtxtlex.list.lua

	local state=lex.create()
	local o={}
	local i=" ajndajsndlajds ..as akjhdkjahsd   asd  a d<a  asdjjhasd0@!#ASaca  ok asdas if "
	
	lex.parse(state,i,o)
	
--	print(i)
--	print(table.concat(o))

end


