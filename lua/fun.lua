
print("-lfun is depreciated and probably unnecessary, just end your file with a .fun.lua")

local apps=require("apps")

-- try hard to find any files wemay need
apps.default_paths()

local wzips=require("wetgenes.zips")
local wwin=require("wetgenes.win")

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



local funname
if filename then -- we have a file to run
	funname=filename:gsub("%.fun%.lua$","") -- strip ending if given
end

local func=function(...)

	local global=require("global") -- prevent accidental global use
	
	local screen=wwin.screen()
	
	local hx,hy,ss=424,240,1
	hx=hx*4

-- remove window scale if tiny screen
	if screen.width>0 then
		ss=math.floor(screen.width/hx)
		if ss<1 then ss=1 end
	end

	local opts={
		times=true, -- request simple time keeping samples

		width=hx*ss,	-- display basics
		height=hy*ss,
		screen_scale=ss,
	--	show="full",
		title="fun",
		start="wetgenes.gamecake.fun.main",
		fun=funname,
		fps=60,
		icon=[[
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
b b b 7 7 7 7 7 7 7 b 7 7 7 7 7 b b b 
b b b 7 7 b b b 7 7 b 7 7 b 7 7 b b b 
b b b 7 7 7 7 b 7 7 b 7 7 b 7 7 b b b 
b b b 7 7 b b b 7 7 b 7 7 b 7 7 b b b 
b b b 7 7 b b b 7 7 7 7 7 b 7 7 b b b 
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
b b 7 7 7 7 7 7 7 b 7 7 7 b 7 7 7 b b 
b b 7 7 7 b b b b b 7 7 7 b 7 7 7 b b 
b b 7 7 7 7 7 7 7 b 7 7 7 7 7 7 7 b b 
b b 7 7 7 b 7 7 7 b b b b b 7 7 7 b b 
b b 7 7 7 7 7 7 7 b b b b b 7 7 7 b b 
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
b b b b b b b b b b b b b b b b b b b 
]],
		... -- include commandline opts
	}

	math.randomseed( os.time() ) -- try and randomise a little bit better

	-- setup oven with vanilla cake setup and save as a global value
	global.oven=require("wetgenes.gamecake.oven").bake(opts).preheat()

	-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
	return oven:serv()
end

if funname then
	func(unpack(argx))

	os.exit(0)
end

