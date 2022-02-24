

module(...,package.seeall)

local wtxt=require("wetgenes.txt")
local wtxtlex=require("wetgenes.txt.lex")
local wstr=require("wetgenes.string")

local wtxtdiff=require("wetgenes.txt.diff")

local txta = [[
This part of the document has stayed
the same from version to version.

This paragraph contains text that is
outdated - it will be deprecated '''and'''
deleted '''in''' the near future.

It is important to spell check this
dokument. On the other hand, a misspelled
word isn't the end of the world.
]]

local txtb = [[
This is an important notice! It should
therefore be located at the beginning of
this document!

This part of the document has stayed
the same from version to version.

It is important to spell check this
document. On the other hand, a misspelled
word isn't the end of the world. This
paragraph contains important new
additions to this document.
]]



function test_find()

	local taba=wtxtdiff.split(txta,"\n")
	local tabb=wtxtdiff.split(txtb,"\n")

	local l,a,b = wtxtdiff.find(taba,tabb)

	assert_true(l==3)
	assert_true(a==1)
	assert_true(b==5)

end


function test_trim()

	local taba=wtxtdiff.split(txta,"\n")
	local tabb=wtxtdiff.split(txtb,"\n")

	local a,b = wtxtdiff.trim(taba,tabb)

	assert_true(a==0)
	assert_true(b==0)

end


function test_match()

	local taba=wtxtdiff.split(txta,"\n")
	local tabb=wtxtdiff.split(txtb,"\n")

	local a,b = wtxtdiff.match(taba,tabb)
	
	assert_equal(#a,#b)

	assert_equal(table.concat(a),txta)
	
	assert_equal(table.concat(b),txtb)
	
	local d=""

	for i=1,#a do
	
		local sa=a[i]
		local sb=b[i]
		
		local ta=wstr.split(sa,"\n")
		local tb=wstr.split(sb,"\n")

		if sa==sb then
			d=d.."="
			for l=1,#ta do
			end
		else
			d=d.."-"
			for l=1,#ta do
			end
			d=d.."+"
			for l=1,#tb do
			end
		end
	
	end

	assert_equal( d , "-+=-+=-+" )

end


