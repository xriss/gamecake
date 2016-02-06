#!/usr/bin/env gamecake

local zips=require("wetgenes.zips")
local bake=require("wetgenes.bake")
local sbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")

local args={...}

local basedir=assert(args[1],"Must specify basedir, eg \"./bake.lua ../../apps/dike\"")

local smell=args[2] -- optional smell

local fdat=assert(zips.readfile(basedir.."/opts.lua"),"opts.lua must exist in the given basedir")

os.execute( "cd "..basedir.." ; ../bake "..(smell or "") ) --make sure it is baked

local preopts=sbox.ini(fdat)

if smell and preopts.smells and preopts.smells[smell] then -- override preopts with smell settings
	for i,v in pairs(preopts.smells[smell]) do
		preopts[i]=v
	end
end

--print(wstr.dump(opts))

local version=args[3] or preopts.version or bake.version_from_time()
local opts={
		smell=smell,
        name=preopts.name,
        title=preopts.title,
        namev=preopts.title..".v"..version,
        version=version,
        version_int=math.floor(version*1000),
	orientation=preopts.orientation or "unspecified",
	
	package=preopts.android_package or "com.wetgenes."..preopts.name,
	activity=preopts.android_activity or "com.wetgenes.FeralActivity",
	permissions=preopts.android_permissions or "",
}

bake.replacefile("AndroidManifest.xml.base","AndroidManifest.xml",opts)
bake.replacefile("build.xml.base","build.xml",opts)

-- remove all files in res

os.execute("rm -rf res")

-- copy all data and code from root into res/raw

for _,dir in ipairs{"lua","data"} do

	local files=bake.findfiles{basedir=basedir,dir=dir}.ret

	for i,v in ipairs(files) do
print(v)
		local n=zips.apk_munge_filename(v) -- this includes the res/raw prefix
		bake.create_dir_for_file(n)
		bake.copyfile(basedir.."/"..v,n)
		print(n)
	end

end

-- patch init.lua
bake.replacefile(basedir.."/lua/init.lua",zips.apk_munge_filename("lua/init.lua"),opts)

if bake.file_exists(basedir.."/lua/init_bake.lua") then
	local lson=bake.readfile(basedir.."/lua/init_bake.lua")
	if lson then
		local data=sbox.lson(lson)
		data.smell=smell -- any given smell overides
		data.version= opts.version or data.version
		bake.writefile( zips.apk_munge_filename("lua/init_bake.lua") , wstr.serialize(data) )
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
		assert(gd:convert(wgrd.FMT_U8_RGBA))
		gd:scale(v.s,v.s,1)
		local n="res/drawable-"..v.o.."/icon.png"
		bake.create_dir_for_file(n)
		gd:save(n)
		print(n)
	end

end


