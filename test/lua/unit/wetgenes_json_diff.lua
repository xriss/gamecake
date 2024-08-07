
local log,dump=require("wetgenes.logs"):export("log","dump")


module(...,package.seeall)


local json_diff=require("wetgenes.json_diff")

function test_obj()

	local a={a=1,b=2,c=3,aa={2,3,4,1,6,7,8}}
	local b={a=1,b=-2,c=3,d=4,aa={1,2,3,4,5,6,7}}
	
	local d=json_diff.diff(a,b,true)
	local db=json_diff.apply( json_diff.dupe(a) ,d)
	local da=json_diff.undo( json_diff.dupe(b) ,d)

	assert_tables_equal(a,da)
	assert_tables_equal(b,db)

end
