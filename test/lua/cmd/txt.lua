
require("apps").default_paths()

-- this is a short comment

local wtxt=require("wetgenes.txt")
local wtxtlex=require("wetgenes.txt.lex")
local wstring=require("wetgenes.string")

--[[

this is a long comment

]]

function test_lex_lua()

	local lex=wtxtlex.list.lua

	local state=lex.create()
	local i=" ajndajsndlajds ..as akjhdkjahsd   asd  a d<a  asdjjhasd0@!#ASaca  ok asdas if "
	
	local fp=io.open("/home/kriss/devcake/gamecake/test/lua/cmd/txt.lua","r")
	local fd=fp:read("a*") ; fp:close()
	local ls=wstring.split(fd,"\n")
	
	for i,s in ipairs(ls) do
		local o={}
		lex.parse(state,s.."\n",o)
		print((s:gsub("\t"," ")))
		local l={}
		for i,v in ipairs(o) do if v=="none" or v=="white" then l[i]="." else l[i]=v:sub(1,1) end end
		print(table.concat(l))
	end
	
end


test_lex_lua()

