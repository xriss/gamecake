

module(...,package.seeall)

local wtxt=require("wetgenes.txt")
local wtxtlex=require("wetgenes.txt.lex")
local wstr=require("wetgenes.string")

local wtxtdiff=require("wetgenes.txt.diff")

local txta = {
    "This part of the document has stayed","\n",
    "the same from version to version.","\n",
    "","\n",
    "This paragraph contains text that is","\n",
    "outdated - it will be deprecated '''and'''","\n",
    "deleted '''in''' the near future.","\n",
    "","\n",
    "It is important to spell check this","\n",
    "dokument. On the other hand, a misspelled","\n",
    "word isn't the end of the world.","\n",
}

local txtb = {
    "This is an important notice! It should","\n",
    "therefore be located at the beginning of","\n",
    "this document!","\n",
    "","\n",
    "This part of the document has stayed","\n",
    "the same from version to version.","\n",
    "","\n",
    "It is important to spell check this","\n",
    "document. On the other hand, a misspelled","\n",
    "word isn't the end of the world. This","\n",
    "paragraph contains important new","\n",
    "additions to this document.","\n",
}



function test_find()

	local l,a,b = wtxtdiff.find(txta,txtb)
	
	assert_true(l==6)
	assert_true(a==1)
	assert_true(b==9)

end


function test_trim()

	local a,b = wtxtdiff.trim(txta,txtb)

	assert_true(a==0)
	assert_true(b==1)

end


function test_match()

	local a,b = wtxtdiff.match(txta,txtb)
	
	assert_equal(#a,#b)

	assert_equal(table.concat(a),table.concat(txta))
	
	assert_equal(table.concat(b),table.concat(txtb))
	
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

	assert_equal( d , "-+=-+=-+=-+=" )

end


