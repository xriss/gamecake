

module(...,package.seeall)

local wstr=require("wetgenes.string")
local zip=require("zip")

function do_file_read(f)
	local fp=assert(io.open(f,"rb"))
	local d=assert(fp:read("*a"))
	fp:close()
	return d
end


function test_zip()

--print(wstr.dump(zip))

	local z=assert(zip.open("dat/zip/t.zip"))
	
--print(wstr.dump(z))

	for v in z:files() do
--		print(wstr.dump(v))
	end


end


function test_zipmem()

--print(wstr.dump(zip))

	local d=do_file_read("dat/zip/t.zip")
	
	local z=assert(zip.open_mem(d))
	
--print(wstr.dump(z))

	for v in z:files() do
--		print(wstr.dump(v))
	end


end
