#!/usr/local/bin/gamecake

local zips=require("wetgenes.zips")
local bake=require("wetgenes.bake")
local sbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")

local args={...}

local basedir=assert(args[1],"Must specify basedir, eg \"./bake.lua ../../apps/dike\"")


local fdat=assert(zips.readfile(basedir.."/opts.lua"),"opts.lua must exist in the given basedir")


local opts=sbox.ini(fdat)

--print(wstr.dump(opts))

local version=bake.version_from_time()
local opts={
        name=opts.name,
        title=opts.title,
        namev=opts.title..".v"..version,
        version=version,
        version_int=math.floor(version*1000),
}

bake.replacefile("AndroidManifest.xml.base","AndroidManifest.xml",opts)
bake.replacefile("build.xml.base","build.xml",opts)

-- remove all files in res

os.execute("rm -rf res")

-- copy all data and code from root into res/raw

for _,dir in ipairs{"lua","data"} do

	local files=bake.findfiles{basedir=basedir,dir=dir}.ret

	for i,v in ipairs(files) do
		local n=zips.apk_munge_filename(v) -- this includes the res/raw prefix
		bake.create_dir_for_file(n)
		bake.copyfile(basedir.."/"..v,n)
		print(n)
	end

end

local ficon=basedir.."/art/icons/android_icon.png"
if not bake.file_exists(ficon) then
	ficon="art/icons/android_icon.png"
end

if bake.file_exists(ficon) then


	for i,v in ipairs{
		{s=96,o="xhdpi"},
		{s=72,o="hdpi"},
		{s=48,o="mdpi"},
		{s=36,o="ldpi"},
	} do
		local gd=assert(wgrd.create(ficon))
		assert(gd:convert(wgrd.FMT_U8_ARGB_PREMULT))
		gd:scale(v.s,v.s,1)
--		assert(gd:convert(wgrd.FMT_U8_ARGB))
		local n="res/drawable-"..v.o.."/icon.png"
		bake.create_dir_for_file(n)
		gd:save(n)
		print(n)
	end

end


