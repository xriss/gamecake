
local apps=require("apps")

-- try hard to find any files wemay need
apps.default_paths()

local wzips=require("wetgenes.zips")

-- strip some args before passing on to main code
local a=arg or {}
local argx={}

local done_start=false -- only remove the first
local done_zip=false

for i=1,#a do
	local v=tostring(a[i])

	if v=="-lstart" and not done_start then
		done_start=true
		v=nil
	elseif v:sub(-4)==".zip" and not done_zip then -- the first zip only
		wzips.add_zip_file(v)
		done_zip=true	
		v=nil
	elseif v:sub(-5)==".cake" then -- all .cake files we are given
		wzips.add_zip_file(v)
		v=nil
	end

	if v then argx[#argx+1]=v end 
end

local str=wzips.readfile("lua/init.lua")

if str then

	if str:sub(1,2)=="#!" then
		str="--"..str -- ignore hashbang on first line
	end

	local func=assert(loadstring(str,name))

	func(unpack(argx))

	os.exit(0) -- force close so that we do not end up at a console?
end
