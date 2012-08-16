#!/usr/local/bin/gamecake

local wbake=require("wetgenes.bake")
local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")


for _,dir in ipairs{"lua","data"} do

	local files=wbake.findfiles{basedir="..",dir=dir,filter="."}.ret

	for i,v in ipairs(files) do
		local n=wzips.apk_munge_filename(v)
		wbake.create_dir_for_file("res/"..n)
		wbake.copyfile("../"..v,"res/"..n)
		print(n)
	end

end


