

local tabassert=function(a,b)
	for n,v in pairs(a) do assert(a[n]==b[n],a[n].."=="..b[n].." : "..n) end
	for n,v in pairs(b) do assert(a[n]==b[n],a[n].."=="..b[n].." : "..n) end
end


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


function test_split_whitespace_quotes()

--print( wetstr.dump( wetstr.split_whitespace_quotes("this is 'a test' ok") ) )
--print( wetstr.dump( {"this"," ","is"," ","'a test'"," ","ok"} ) )

tabassert( wetstr.split_whitespace_quotes("this is a test") , {"this"," ","is"," ","a"," ","test"} )

tabassert( wetstr.split_whitespace_quotes("this is 'a test'") , {"this"," ","is"," ","'a test'"} )

tabassert( wetstr.split_whitespace_quotes("this is 'a test' ok") , {"this"," ","is"," ","'a test'"," ","ok"} )

tabassert( wetstr.split_whitespace_quotes("this is 'a \\' test' ok") , {"this"," ","is"," ","'a \\' test'"," ","ok"} )

end





