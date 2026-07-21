#!/usr/bin/env gamecake

local zips=require("wetgenes.zips")
local bake=require("wetgenes.bake")
local sbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local wgrd=require("wetgenes.grd")
local wpath=require("wetgenes.path")

local bitdown=require("wetgenes.gamecake.fun.bitdown")
local wgfun=require("wetgenes.gamecake.fun.main")

local args={...}

local basedir=assert( args[1] or os.getenv("ANDROID_APP_BASEDIR") ,"Must specify basedir, eg \"./copyfiles.lua ../../apps/dike\"")

local smell=args[2] or os.getenv("ANDROID_APP_SMELL") or "" -- optional smell

-- os.execute( "cd "..basedir.." ; ../bake "..(smell or "") ) --make sure it is baked

local preopts={}

if smell=="fun" then

preopts={
	name="fun",
	title="fun",
	version=1.234567,
	fun=basedir,
	fun_file="lua/"..wpath.file(basedir),
}
preopts.commandline=[[ "]]..preopts.fun_file..[[" , "--" , "--logs" , "--show=full" ]]

print(basedir)
local info=wgfun.get_info(basedir)
print(info)
preopts.name=info.android_class or "fun"
preopts.title=info.title or "fun"
preopts.icon=info.icon or [[
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g d d d d g d d g d d g d d d d d g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g d d d d g d d g d d g d d g d d g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g d d g g g d d d d d g d d g d d g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g d d g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g d d g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d g g g g g g d d g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g d d d d d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g d d d d d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d g g d d g g g g g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g g g g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g d d d d d d g g g g g g d d g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
]]

else

local fdat=assert(zips.readfile(basedir.."/opts.lua"),"opts.lua must exist in the given basedir")
preopts=sbox.ini(fdat)

end

if smell and preopts.smells and preopts.smells[smell] then -- override preopts with smell settings
	for i,v in pairs(preopts.smells[smell]) do
		preopts[i]=v
	end
end




--print(wstr.dump(opts))

local version=preopts.version or bake.version_from_time()
local opts={
	basedir=basedir,
	smell=smell,
	fun=preopts.fun,
	fun_file=preopts.fun_file,
	commandline=preopts.commandline or [[ "--" , "--logs" , "--fullscreen" ]],
	name=preopts.name,
	title=preopts.title,
	namev=preopts.title..".v"..version,
	version=version,
	version_int=math.floor(version*100000),
	orientation=preopts.orientation or "unspecified",
	package=preopts.android_package or "com.wetgenes.gamecake."..preopts.name,
	activity=preopts.android_activity or "com.wetgenes.gamecake."..preopts.name..".CakeAct",
	permissions=preopts.android_permissions or "",
}
for n,v in pairs(opts) do
	print(n,"=",v)
end


os.execute("rm -rf gamecake/src/main/assets")
os.execute("rm -rf gamecake/src/main/java/com")
os.execute("mkdir -p gamecake/src/main/java/com/wetgenes/gamecake/"..opts.name)

bake.replacefile("input/config.sh","config.sh",opts)
bake.replacefile("input/AndroidManifest.xml","gamecake/src/main/AndroidManifest.xml",opts)
bake.replacefile("input/strings.xml","gamecake/src/main/res/values/strings.xml",opts)
bake.replacefile("input/CakeAct.java","gamecake/src/main/java/com/wetgenes/gamecake/"..opts.name.."/CakeAct.java",opts)
bake.replacefile("input/build.gradle","gamecake/build.gradle",opts)

if smell=="fun" then
	local n="gamecake/src/main/assets/"..opts.fun_file
	bake.create_dir_for_file(n)
	bake.copyfile(opts.fun,n)
end

-- copy all data and code from root into res/raw

for _,dir in ipairs{"lua","data"} do

	local files=bake.findfiles{basedir=basedir,dir=dir}.ret

	for i,v in ipairs(files) do
		local n="gamecake/src/main/"..zips.apk_munge_filename(v) -- this includes the res/raw prefix
		bake.create_dir_for_file(n)
		bake.copyfile(basedir.."/"..v,n)
		print(n)
	end

end

-- include cache of lua/wetgenes
	local files=bake.findfiles{basedir="..",dir="lua/wetgenes"}.ret

	for i,v in ipairs(files) do
		local n="gamecake/src/main/"..zips.apk_munge_filename(v) -- this includes the res/raw prefix
		bake.create_dir_for_file(n)
		bake.copyfile("../"..v,n)
		print(n)
	end



-- patch init.lua if it exists
if bake.isfile( basedir.."/lua/init.lua" ) then
	bake.replacefile(basedir.."/lua/init.lua","gamecake/src/main/"..zips.apk_munge_filename("lua/init.lua"),opts)
end

if bake.isfile(basedir.."/lua/init_bake.lua") then
	local lson=bake.readfile(basedir.."/lua/init_bake.lua")
	if lson then
		local data=sbox.lson(lson)
		data.smell=smell -- any given smell overides
		data.version= opts.version or data.version
		bake.writefile( "gamecake/src/main/"..zips.apk_munge_filename("lua/init_bake.lua") , wstr.serialize(data) )
	end
end



local ficon=basedir.."/art/icons/android_icon.png"
if not bake.isfile(ficon) then
	ficon="art/icons/android_icon.png"
end

if bake.isfile(ficon) then

	for i,v in ipairs{
		{s=192,o="xxxhdpi"},
		{s=144,o="xxhdpi"},
		{s=96,o="xhdpi"},
		{s=72,o="hdpi"},
--		{s=48,o="mdpi"},
--		{s=36,o="ldpi"},
	} do
		local gd
		if preopts.icon then
			gd=bitdown.pix_grd(preopts.icon)
			assert(gd:convert(wgrd.FMT_U8_RGBA))
			gd:scale(v.s,v.s,1)
		else
			gd=assert(wgrd.create(ficon))
			assert(gd:convert(wgrd.FMT_U8_RGBA))
			gd:scale(v.s,v.s,1)
		end
		local n="gamecake/src/main/res/mipmap-"..v.o.."/ic_launcher.png"
		bake.create_dir_for_file(n)
		gd:save(n)
		print(n)
	end

end

