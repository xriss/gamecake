
local apps=require("apps")
-- try hard to find any files wemay need
apps.default_paths()

-- need to know about the system we are running on
local SDL=require("SDL")
local platform=SDL.getPlatform()

-- need to auto mount some zip files for reading from
local wzips=require("wetgenes.zips")

if platform=="Android" then

	SDL.log("START ANDROID")

-- replace print so we send print output to SDL.log	
	print=function(...)
		local t={...}
		for i=1,#t do t[i]=tostring(t[i]) end
		SDL.log( table.concat(t,"\t") )
	end
	
	os.exit=function()print("os.exit() IN ANDROID IS DISABLED") return 1/0 end

end

	if jit then -- start by trying to force a jit memory allocation
--		print("LUAJIT",jit.status())
--		require("jit.opt").start("sizemcode=128","maxmcode=128")
--		local t={} ; for i=1,1000 do t[#t+1]=i end
--		print("LUAJIT",jit.status())

--		if jit and jit.off then
--			jit.off()
--			print("LUAJIT","OFF")
--		end -- sometimes jit causes problems

  	end


-- strip some args before passing on to main code
local a=arg or {}
local argx={}

local done_cake=false -- only remove the first -lcake we see
local done_apk=false
local done_zip=false

for i=1,#a do
	local v=tostring(a[i])

	if v=="-lstart" and not done_cake then
		done_cake=true
		v=nil
	elseif v:sub(-4)==".apk" and not done_apk then -- the first apk only
		wzips.add_apk_file(v)
		done_apk=true
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

else

	local m_oven=require("wetgenes.gamecake.oven")
	print(m_oven.help_text)

end

	os.exit(0) -- force close so that we do not end up at a console?
