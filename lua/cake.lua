
local apps=require("apps")
-- try hard to find any files wemay need
apps.default_paths()

-- need to auto mount some zip files for reading from
local wzips=require("wetgenes.zips")

-- strip some args before passing on to main code
local a=arg or {}
local argx={}

local filename

local done_all=false

for i=1,#a do
	local v=tostring(a[i])

	if done_all then
		-- ignore everything after --
	elseif v=="--" then -- stop looking
		done_all=true
		v=nil
	elseif v=="-landroid" then -- may have started as android
		v=nil
	elseif v=="-lcake" then -- we are this lib
		v=nil
	elseif v:sub(1,1)=="-" then
		-- ignore opts
	elseif v:sub(-4)==".apk" then -- mount apk
		wzips.add_apk_file(v)
		v=nil
	elseif v:sub(-4)==".zip" then -- mount zip
		wzips.add_zip_file(v)
		v=nil
	elseif v:sub(-5)==".cake" then -- mount cake
		wzips.add_zip_file(v)
		v=nil
	else
		if not filename then -- filename is first non option
			filename=v
			v=nil
		end
	end

	if v then argx[#argx+1]=v end 
end

local str=wzips.readfile(filename or "lua/init.lua")

if str then

	if str:sub(1,2)=="#!" then
		str="--"..str -- ignore hashbang on first line
	end

	local func=assert(loadstring(str,name))

	func(unpack(argx))

	os.exit(0)
end
