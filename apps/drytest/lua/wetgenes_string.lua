





module(...,package.seeall)

local wetstr=require("wetgenes.string")

function test_serialize(dat)

	if type(dat)=="table" then
		
		local dat1=dat or
		{
			poop={1,2,3}
		}

		local str=wetstr.serialize(dat1)
		
		local f=assert(loadstring("return( "..str.." )"))
		
		local dat2=assert(f())
				
		assert_tables_equal(dat1,dat2)
		
	end
	
end

function test_serialize_tabs()

	test_serialize( {
		{
			ok=true
		},
		{
			[true]=true
		},
		{
			[false]=true
		},
	} )

end

