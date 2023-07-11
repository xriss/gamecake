

module(...,package.seeall)

local wtxtwords=require("wetgenes.txt.words")

function test_load()

	wtxtwords.load()

end


function test_spell()

	local list=wtxtwords.spell("engrish")
	assert( table.concat(list," ") == "english perish garish enrich grayish enravish nourish cherish anguish wearish" )

end

function test_check()

	assert( not wtxtwords.check("engrish") )
	assert(     wtxtwords.check("english") )

end

