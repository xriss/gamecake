
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


function test_uint32s()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_uint32s(dat1)
	local dat2=fats.uint32s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end

function test_int32s()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_int32s(dat1)
	local dat2=fats.int32s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end


function test_uint16s()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_uint16s(dat1)
	local dat2=fats.uint16s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end

function test_int16s()

	local dat1={123,456,789,312,654,789}
	local str1=fats.table_to_int16s(dat1)
	local dat2=fats.int16s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end


function test_uint8s()

	local dat1={123,156,189,112,154,189}
	local str1=fats.table_to_uint8s(dat1)
	local dat2=fats.uint8s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end

function test_int8s()

	local dat1={23,56,89,12,54,89}
	local str1=fats.table_to_int8s(dat1)
	local dat2=fats.int8s_to_table(str1)

	assert_tables_equal(dat1,dat2)

end



