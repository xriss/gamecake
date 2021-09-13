
local log,dump=require("wetgenes.logs"):export("log","dump")


module(...,package.seeall)


local fats=require("wetgenes.fats")

function test_floats()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_floats(dat1)
	local dat2=fats.floats_to_table(str1)

	assert_tables_equal(dat1,dat2)

end

function test_floats_sub()

	local dat1={123,456,789,312,654,789}

	local str3=fats.table_to_floats(dat1,2,3)
	local dat3=fats.floats_to_table(str3)

	local str4=fats.table_to_floats(dat1)
	local dat4=fats.floats_to_table(str4,2,3)

	assert_tables_equal(dat3,dat4)
	
end

function test_doubles()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_doubles(dat1)
	local dat2=fats.doubles_to_table(str1)

	assert_tables_equal(dat1,dat2)

end

function test_doubles_sub()

	local dat1={123,456,789,312,654,789}

	local str3=fats.table_to_doubles(dat1,2,3)
	local dat3=fats.doubles_to_table(str3)

	local str4=fats.table_to_doubles(dat1)
	local dat4=fats.doubles_to_table(str4,2,3)

	assert_tables_equal(dat3,dat4)
	
end
