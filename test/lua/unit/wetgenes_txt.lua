

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


function test_pos_lua()

	local txt=wtxt.construct()
	txt.set_text("abcdefghijklmnopqrstuvwxyz")

assert_equal("z",txt.get_string_sub(1, -1) )
assert_equal("z",txt.get_string_sub(1, -1,-1) )
assert_equal("a",txt.get_string_sub(1, 1,1) )
assert_equal("",txt.get_string_sub(1, 0,0) )
assert_equal("abcdefghijklmnopqrstuvwxyz",txt.get_string_sub(1, 0) )
assert_equal("abcdefghijklmnopqrstuvwxyz",txt.get_string_sub(1, 1) )
assert_equal("fghijkl",txt.get_string_sub(1, 6,12) )
assert_equal("tuvwxyz",txt.get_string_sub(1, 20,30) )
assert_equal("xyz",txt.get_string_sub(1, 24,30) )
assert_equal("yz",txt.get_string_sub(1, 25,30) )

end


