





module(...,package.seeall)

local wetstr=require("wetgenes.string")

function test_serialize(dat)

	local dat1=dat or
	{
		poop={1,2,3,ok="yes",num=78,{ok="yes",21,22,23}}
	}

	if type(dat1)=="table" then
		

-- test normal
		local str=wetstr.serialize(dat1)		
		local f=assert(loadstring("return( "..str.." )"))
		local dat2=assert(f())
		assert_tables_equal(dat1,dat2)

-- test pretty		
		local str=wetstr.serialize(dat1,{pretty=true})
		local f=assert(loadstring("return( "..str.." )"))
		local dat2=assert(f())
		assert_tables_equal(dat1,dat2)

-- test compact
		local str=wetstr.serialize(dat1,{compact=true})
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

