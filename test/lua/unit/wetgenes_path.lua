

module(...,package.seeall)

local wstr=require("wetgenes.string")
local wpath=require("wetgenes.path")

	local pa="/test/this/path/"
	local pb="/test/this/path/to/a/file.ext"


function test_path1()
	local ps=wpath.split(pb)
	assert( pb == wpath.join(ps) )
end
function test_path2()
	assert( wpath.relative(pa,pb) == "./to/a/file.ext" )
end
function test_path3()
	local pp=wpath.parse(pb)
	assert( pp.root == "/" )
end
function test_path4()
	local pp=wpath.parse(pb)
	assert( pp.dir == "/test/this/path/to/a/" )
end
function test_path5()
	local pp=wpath.parse(pb)
	assert( pp.file == "file.ext" )
end
function test_path6()
	local pp=wpath.parse(pb)
	assert( pp.name == "file" )
end
function test_path7()
	local pp=wpath.parse(pb)
	assert( pp.ext == ".ext" )
end
