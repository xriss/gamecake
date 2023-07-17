
local apps=require("apps")

-- try hard to find any files wemay need
apps.default_paths()

local wzips=require("wetgenes.zips")
local wwin=require("wetgenes.win")

-- strip some args before passing on to main code
local a=arg or {}
local argx={}

local done_fun=false -- only remove the first
local done_zip=false

local filename=""

for i=1,#a do
	local v=tostring(a[i])

	if v=="-lfun" and not done_fun then
		done_fun=true
		v=nil
	elseif v:sub(-4)==".zip" and not done_zip then -- the first zip only
		wzips.add_zip_file(v)
		done_zip=true	
		v=nil
	elseif v:sub(-5)==".cake" then -- all .cake files we are given
		wzips.add_zip_file(v)
		v=nil
	elseif v:sub(1,1)~="-" then -- this is the file we plan to run
		filename=v
		v=nil
	end

	if v then argx[#argx+1]=v end 
end

local funname=filename:gsub("%.fun%.lua$","") -- strip ending if given
--local funpath=filename:gsub("[^/]+$","") -- path to file

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
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g 7 7 7 7 7 7 7 g g g g g g g g 
g g g g g g g 7 7 7 g g g g g 7 7 7 g g g g g g 
g g g g g g 7 7 g g g g g g g g g 7 7 g g g g g 
g g g g g 7 7 g g g g g g g g g g g 7 7 g g g g 
g g g g g 7 g g 7 7 7 7 g 7 7 7 7 g g 7 g g g g 
g g g g 7 7 g g 7 g g 7 g 7 7 g 7 g g 7 7 g g g 
g g g g 7 g g g 7 7 g 7 g 7 7 g 7 g g g 7 g g g 
g g g g 7 g g g 7 g g 7 7 7 7 g 7 g g g 7 g g g 
g g g g 7 g g g g g g g g g g g g g g g 7 g g g 
g g g g 7 g g g g 7 7 7 g 7 g 7 g g g g 7 g g g 
g g g g 7 g g g g 7 g g g 7 g 7 g g g g 7 g g g 
g g g g 7 7 g g g 7 7 7 g 7 7 7 g g g 7 7 g g g 
g g g g g 7 g g g 7 g 7 g g g 7 g g g 7 g g g g 
g g g g g 7 7 g g 7 7 7 g g g 7 g g 7 7 g g g g 
g g g g g g 7 7 g g g g g g g g g 7 7 g g g g g 
g g g g g g g 7 7 7 g g g g g 7 7 7 g g g g g g 
g g g g g g g g g 7 7 7 7 7 7 7 g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
g g g g g g g g g g g g g g g g g g g g g g g g 
]],
		... -- include commandline opts
	}

	require("apps").default_paths() -- default search paths so things can easily be found

	math.randomseed( os.time() ) -- try and randomise a little bit better

	-- setup oven with vanilla cake setup and save as a global value
	global.oven=require("wetgenes.gamecake.oven").bake(opts).preheat()

	-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
	return oven:serv()
end

func(unpack(argx))

os.exit(0) -- force close so that we do not end up at a console?

