--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--local log,dump,PRINT=require("wetgenes.logs"):export("log","dump","PRINT")
local logs=require("wetgenes.logs")
local global=require("global")

local toaster=require("wetgenes.gamecake.toaster")
-- help luajit work on android/arm
toaster.jit_prealloc()
local profile=require("jit.profile")

--[[#lua.wetgenes.gamecake.oven

	oven=require("wetgenes.gamecake.oven").bake(opts)

The oven module must be baked so only exposes a bake function.

All the other functions are returned from within the bake function.

possible ENV settings

	gamecake_tongue=english

]]

local wpath=require("wetgenes.path")
local wpackage=require("wetgenes.package")
local wzips=require("wetgenes.zips")
local wsbox=require("wetgenes.sandbox")
local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local wtongues=require("wetgenes.tongues")

local function print(...) return _G.print(...) end
local dprint=function(a) print(wstr.dump(a)) end

local function assert_resume(co)

	local ok,err=coroutine.resume(co)

	if ok then return ok,err end -- no error

	print( debug.traceback(co,err) ) -- error
	os.exit(20)
end


local M={ modname=(...) } ; package.loaded[M.modname]=M

M.help_text=[[
gamecake -lcake  # Run a cake in cake mode
gamecake -lfun   # Run a fun in fun mode
gamecake -lcmd   # Run a cmd in cmd mode

Mostly we will behave the same as the lua or luajit command, but some
filenames will be treated as special. So filename.fun.lua will auto
run in fun mode and filename.cake will auto run in cake mode. When
running a cake or fun script then the following args can control it.

  --help
    Print help text and exit
		gamecake -lcake --help
		gamecake -lcmd --help

  --logs
    Enable all log output. ( essentially a verbose mode )

  --logs=MODE
	Enables MODE log output only, eg --logs=oven for oven logs only.
	Prefixing mode with a - will remove that mode from the logs and a +
	will add it and we assume + if neither is present, eg --logs=-oven
	will show everything except oven logs.

  --win-hx=640
  --win-hy=480
  --win-px=0
  --win-py=0
	Set the window size and position, without these values we will
	place and position the window automatically.

  --win-vsync=1
	Set vsync to 0 for immediate updates, 1 for updates synchronized 
	with the vertical retrace, -1 for adaptive vsync. We default to 
	1 so will not be able to run at an fps faster than your screens 
	refresh rate.

  --win-borderless
    Ask for a borderless window.

  --win-hidden
    Ask for window to start off hidden.

  --win-title="hello world"
    Set window title.

  --show=win
	Show window as a normal draggable and resizable window. This is the
	default.

  --show=max
	Show window as a maximised window. Desktop resolution possibly
	with a visible title bar and desktop panel still visible.

  --show=full
	Show window as a borderless full screen window. Desktop resolution
	no title bar.

  --screen=1280x720
  --screen=1280x720.RGB888
  --screen=1280x720.RGB888/60
	When going full screen request this resolution, optional SDL pixel
	format and optional framerate.

  --pixel
    Disable screen space pixel processing, eg fun scanlines filter.

  --pixel=NUMBER
	Disable screen space pixel processing and set default window size
	to NUMBER view pixels per game pixel.
]]

-- probably not generic? maybe

--[[
  --tasks=many
  --tasks=only
  --tasks=net
  --tasks=fake
	Request app threading style (which may be ignored if unsupported),
	defaults to many which probably means draw on main thread, update
	on another thread. Set to only for easier debugging in a single
	thread. Net means we are running multiple states and trying to sync
	them over a network. Fake means we are doing network but faking
	multiple clients on one machine for debugging.

]]

--[[#lua.wetgenes.gamecake.oven.bake


	oven=wetgenes.gamecake.oven.bake(opts)

Bake creates an instance of a lua module bound to a state. Here we
are creating the main state that other modules will then bind to.

We call each state an OVEN to fit into the gamecake naming scheme
then we bake a module in this oven to bind them to the common state.

Think of it as a sub version of require, so require gets the global
pointer for a module and bake is used to get the a module bound to
an oven.

By using this bound state we reduce the verbosity of connecting
modules and sharing state between them.


]]
function M.bake(opts)

	local oven={}
	oven.newticks=toaster.newticks
	oven.is={}
	wwin.oven=wwin.oven or oven -- store a global oven on first use

	if opts.hints then -- pass hints from opts to sdl
		wwin.hints(opts.hints)
	end

	oven.enable_close_window=true -- let the close button, close the window (otherwise you should catch close messages in app)

	oven.opts=opts or {}
	global.OVEN_OPTS=oven.opts -- remember last oven opts in global
	global.TASK_NAME="#MAIN"
	global.PRINT=logs.print
	global.DUMP=logs.dump
	global.LOG=logs.log
	global.TRACEBACK=logs.traceback

	if type(opts[1])=="table" then -- probably passed in from nacl
		for n,v in pairs(opts[1]) do -- copy it all into opts
			opts[n]=v
		end
	end

-- handle commandline options, copy --flags into opts.args and put other args into number keys
	opts.args=opts.args or {}
	for i=0,#opts do local v=opts[i]
		if type(v)=="string" then
			if v:sub(1,2)=="--" then -- strip --from-start-of-flags
				local n=v:sub(3)
				local d=true
				local s,e = v:find("=")
				if s then -- its a setting so set it
					n=(v:sub(3,s-1))
					d=v:sub(e+1)
				end
				n=n:lower():gsub("[^%w]", "_")
				opts.args[ n ]=d -- simple setting, strings only
			else
				opts.args[#opts.args+1]=v -- normal arg
			end
		end
	end
--dprint(opts)

	if opts.args.help then

		print(opts.help_text or M.help_text)

		os.exit(0)
	end

	opts.width=opts.width or 0
	opts.height=opts.height or 0

	if opts.sanitize then -- sanitize args
		opts:sanitize()
	end

	require("wetgenes.logs").setup(opts.args)

do	-- get best time we can, should have at least ms accuracy, possibly slightly more
	local ok=pcall(function()
		local socket = require("socket")
		oven.time=socket.gettime
		LOG( "oven" , "using time from socket" )
	end) or pcall(function()
		local lanes = require("lanes")
		oven.time=lanes.now_secs
		LOG( "oven" , "using time from lanes" )
	end) or pcall(function()
		local wwin = require("wetgenes.win") -- probably SDL
		oven.time=wwin.time
		LOG( "oven" , "using time from wetgenes.win" )
	end)
end

if jit then -- now logs are setup, dump basic jit info
	local t={jit.version,jit.status()}
	t[2]=tostring(t[2])
	t[#t+1]="jit_mcode_size="..toaster.jit_mcode_size.."k"
	LOG( "oven" , table.concat(t,"\t") )
end

-- no more flavour, only SDL
--print(wwin.flavour)


-- check if we already have settings in our local dir, and if so then we will keep using that
-- so you can force appdir use with a touch files/settings.lua command

oven.homedir="./"
local sniff_homedir=true

local fp=io.open(wwin.files_prefix.."settings.lua","r")
if fp then -- stick with the default files in the app dir (lets the user force local use)
	fp:close()
	sniff_homedir=false
end



if sniff_homedir then -- smart setup to save files into some sort of user file area depending on OS

	local homedir

	if not homedir and wwin.sdl_platform=="Android" then
		homedir=wwin.GetPrefPath("wetgenes","gamecake")
		if homedir then
			wwin.files_prefix=wpath.resolve(homedir)
			wwin.cache_prefix=wpath.resolve(homedir,"../cache/")
		end
	end

	if not homedir and (wwin.sdl_platform=="Windows" ) then

		homedir=os.getenv("USERPROFILE") -- windows only check

		if homedir then
			wwin.files_prefix=homedir.."/gamecake/"..(opts.name or "gamecake").."/files/"
			wwin.cache_prefix=homedir.."/gamecake/"..(opts.name or "gamecake").."/cache/"

			local wbake=require("wetgenes.bake")
			wbake.create_dir_for_file(wwin.files_prefix.."t.txt")
			wbake.create_dir_for_file(wwin.cache_prefix.."t.txt")

			oven.homedir=homedir.."/"
		end

	end


	if not homedir then

		homedir=os.getenv("HOME") -- good for most everyone else

		if homedir then
			wwin.files_prefix=homedir.."/.config/"..(opts.name or "gamecake").."/files/"
			wwin.cache_prefix=homedir.."/.config/"..(opts.name or "gamecake").."/cache/"

			local wbake=require("wetgenes.bake")
			wbake.create_dir_for_file(wwin.files_prefix.."t.txt")
			wbake.create_dir_for_file(wwin.cache_prefix.."t.txt")

			oven.homedir=homedir.."/"
		end

	end

end

if wwin.steam then -- steamcloud prefers files in your app dir for easy sync between multiple platforms

	local wbake=require("wetgenes.bake")

	wwin.files_prefix=wwin.files_prefix..(wwin.steam.userid).."/"
	wbake.create_dir_for_file(wwin.files_prefix.."t.txt")

	sniff_homedir=false
end


-- setup tasks early so we can use recipes.sqlite for data management and run loading tasks on another thread
	oven.tasks=require("wetgenes.tasks").create()
-- and http requests
	oven.tasks:add_global_thread({
		count=8,
		id="http",
		code=oven.tasks.http_code,
		globals={
			TASK_NAME="#HTTP"
		}
	})
-- and sqlite requests to a default database
	oven.tasks:add_global_thread({
		count=1,
		id="sqlite",
		code=oven.tasks.sqlite_code,
		globals={
			sqlite_filename=wwin.files_prefix.."recipes.sqlite",
			sqlite_pragmas=[[ PRAGMA synchronous=0; ]],
			TASK_NAME="#SQL",
		}
	})
-- so we can run off thread code and coroutines


--[[
print(wwin.files_prefix)
print(wwin.cache_prefix)
print(oven.homedir)
os.exit()
]]


-- pull in info about what art we baked
		local lson=wzips.readfile("lua/init_bake.lua")
		if lson then
--print(lson)
			oven.opts.bake=wsbox.lson(lson)
			oven.opts.smell=oven.opts.bake.smell or oven.opts.smell -- smell overides
--print(oven.opts.bake.stamp)
		end


--opts.disable_sounds=true


-- handle language loading, you will need to load again to change language

		wtongues.set(os.getenv("gamecake_tongue") or ( wwin.steam and wwin.steam.language ) or "english")
		wtongues.load()


		oven.baked={}
		oven.mods={}


		oven.frame_rate_limited=function()
			return oven.frame_rate and oven.frame_time and (not oven.frame_rate_auto)
		end
--
-- preheat a normal oven
-- you may perform this yourself if you want more oven control
--
		function oven.preheat()

			if opts.fps then
				if opts.fps=="auto" then -- we will auto regulate between 1 and 60 fps
					oven.frame_rate=1/60
					oven.frame_rate_auto=1/60
				else
					oven.frame_rate=1/opts.fps -- how fast we want to run
				end
				oven.frame_time=0
			end

			local inf={width=opts.width,height=opts.height,title=opts.title,overscale=opts.overscale,
				console=opts.args.console,		-- use --console on commandline to keep console open
				borderless=opts.borderless,
				hidden=opts.hidden,
				}
--				border=opts.args.border}		-- use --border  on commandline to keep window borders

			local set_inf_arg=function(inf_name,arg_name,arg_cast)
				arg_cast=arg_cast or function(v) return v end
				if opts.args[arg_name] then
					inf[inf_name]=arg_cast( opts.args[arg_name] )
				end
			end

			local screen=wwin.screen()

			inf.name=opts.class_name or opts.title

			if opts.bake and opts.bake.smell and opts.version then
				inf.title=inf.title.." ( "..opts.version.." "..string.upper(opts.bake.smell).." ) "
			elseif opts.version then
				inf.title=inf.title.." ( "..opts.version.." ) "
			end

			local tobool=function(v) return v and true or false end
			set_inf_arg("borderless","win-borderless",tobool)
			set_inf_arg("hidden","win-hidden",tobool)
			set_inf_arg("title","win-title",tostring)
			set_inf_arg("width","win-hx",tonumber)
			set_inf_arg("height","win-hy",tonumber)
			set_inf_arg("vsync","win-vsync",tonumber)

			-- auto center dependng on size
			inf.x=math.floor((screen.width-inf.width)*(opts.win_px or 0.5))
			inf.y=math.floor((screen.height-inf.height)*(opts.win_py or 0.5))
			if inf.x<0 then inf.x=0 end
			if inf.y<0 then inf.y=0 end

			-- overide auto center
			set_inf_arg("x","win-px",tonumber)
			set_inf_arg("y","win-py",tonumber)

			inf.screen_mode=opts.args.screen or opts.screen_mode

			oven.win=wwin.create(inf)
			oven.win:context({})

--wwin.hardcore.peek(oven.win[0])

			if not inf.hidden then
				local doshow=opts.args.show or opts.show or "win" -- default window display
				if doshow then oven.win:show(doshow) end
			end

--			oven.win:show("full")
--			oven.win:show("max")

			oven.rebake("wetgenes.gamecake.cake") -- bake the cake in the oven,

			oven.view=oven.cake.views.create({

				mode="win",
				win=oven.win,
				vx=oven.win.width,
				vy=oven.win.height,
				fov=0,

			})
			oven.msg_view=view -- the view to use to fix mouse position
			oven.cake.views.push(oven.view) -- add master view which is the size of the main window

--			oven.skeys=oven.rebake("wetgenes.gamecake.spew.keys") -- need to use spew keys always or mouse button no click on gui

			oven.ups=oven.rebake("wetgenes.gamecake.ups") -- replace keys and recaps and msg(m) calls with cached msgs/state per frame

			-- the order these are added is important for priority, top of list is lowest priority, bottom is highest.
			oven.rebake_mod("wetgenes.gamecake.mods.escmenu") -- escmenu gives us a doom style escape menu
			oven.rebake_mod("wetgenes.gamecake.mods.console") -- console gives us a quake style tilda console
			oven.rebake_mod("wetgenes.gamecake.mods.snaps") -- builtin screen snapshot code

-- handle default win icon if it exists and we need it .06.png will be 64x64 version
			if wwin.hardcore.icon then
				local fname
				if     wzips.exists("data/icons/win_icon.png")    then fname="data/icons/win_icon.png"
				elseif wzips.exists("data/icons/win_icon.06.png") then fname="data/icons/win_icon.06.png"
				end
				if fname then
					local wgrd=require("wetgenes.grd")
					local g=assert(wgrd.create())
					local d=assert(wzips.readfile(fname),"Failed to load "..fname)
					assert(g:load_data(d,"png")) -- last 3 letters pleaze
					wwin.hardcore.icon(oven.win[0],g)
				elseif opts.icon then -- load ascii pixels
					local bd=require("wetgenes.gamecake.fun.bitdown")
					local g=bd.pix_grd(opts.icon)
					wwin.hardcore.icon(oven.win[0],g)
				end
			end

			if wzips.exists("data/wskins/soapbar.png") or wzips.exists("data/wskins/soapbar.00.lua")then -- we got us better skin to use :)
				oven.rebake("wetgenes.gamecake.widgets.skin").load("soapbar")
			end


			if opts.start then
				if type(opts.start)=="string" then
					oven.next=oven.rebake(opts.start)
				else
					oven.next=opts.start
				end
				oven.main=oven.next
			end


			return oven
		end

-- require and bake oven.baked[modules] in such a way that it can have simple circular dependencies

		function oven.rebake(name)

			local ret=oven.baked[name]

			if not ret then

				if type(name)=="function" then -- allow bake function instead of a name
					return name(oven,{})
				end

				ret={modname=name}
				oven.baked[name]=ret -- need to create and remember here so we can always rebake even if the result is not filled in yet
				ret=assert(require(name)).bake(oven,ret)

--				print("REBAKED",name,ret)
			end

			return ret
		end

		function oven.reload(name)

			if name=="*" then

				for n,v in pairs(oven.baked) do -- reload all baked modules
					LOG("rebake",n)
					local suc,err=pcall(function()
						oven.reload(n)
					end)
					if not suc then LOG("rebake","IGNORE",err) end
				end

			else

				local ret=oven.rebake(name) -- make sure it has been baked once ( probably just finds it)

				assert(wpackage.reload(name)).bake(oven,ret) -- then bake again

				return ret

			end
		end

-- this performs a rebake and adds the baked module into every update/draw function
-- so we may insert extra functionality without having to modify the running app
-- eg a console or an onscreen keyboard
		function oven.rebake_mod(name)

			if oven.mods[name] then return oven.mods[name] end -- already setup, nothing else to do
			local m=oven.rebake(name) -- rebake mod into this oven

			oven.mods[name]=m			-- store baked version by its name
			table.insert(oven.mods,m)		-- and put it at the end of the list for easy iteration

			if m.setup then m.setup() end -- and call setup since it will always be running from now on until it is removed

			local shortname=name:gmatch("%.([^.]*)$")() -- get the bit after the last .
			oven[shortname]=m -- and store in main oven for easy access to this mod
			return m
		end


		if opts.times then
			oven.times={}
			function oven.times.create()
				local t={}
				t.time=0
				t.time_live=0

				t.hash=0
				t.hash_live=0

				t.started=0

				function t.start()
					t.started=oven.win and oven.time() or 0
				end

				function t.stop()
					local ended=oven.win and oven.time() or 0

					t.time_live=t.time_live + ended-t.started
					t.hash_live=t.hash_live + 1
				end

				function t.done()
					t.time=t.time_live
					t.hash=t.hash_live
					t.time_live=0
					t.hash_live=0

				end

				return t
			end
			oven.times.update=oven.times.create()
			oven.times.draw=oven.times.create()
		end

		function oven.change(oven_next)
		if oven_next then oven.next=oven_next end

		-- handle oven changes

			if oven.next then

				oven.clean()

				if type(oven.next)=="string" then	 -- change by required name

					oven.next=oven.rebake(oven.next)

				elseif type(oven.next)=="boolean" then -- special exit oven

--					if wwin.hardcore.finish then
--						wwin.hardcore.finish() -- try a more serious quit?
--						oven.next=nil
--					else
					if wwin.hardcore.task_to_back then -- on android there is no quit, only back
						wwin.hardcore.task_to_back()
						if opts.start then
							oven.next=oven.rebake(opts.start) -- beter than staying on the menu
						else
							oven.next=nil
						end
					else
						oven.next=nil
						oven.finished=true
					end

					oven.last=oven.now
					oven.now=oven.next

				end

				if oven.next then
					oven.last=oven.now
					oven.now=oven.next
					oven.next=nil

					oven.setup()
				end

			end

		end

		function oven.setup()
			if oven.now and oven.now.setup then
				oven.now.setup() -- this will probably load data and call the preloader
			end
--print("setup preloader=off")
			oven.preloader_enabled=false -- disabled preloader after first setup completes
		end

		oven.started=false -- keep track of startstop state
		oven.do_start=false -- flag start
		function oven.start()
			oven.do_start=false
			if oven.started then return end -- already started
			oven.started=true

--oven.gl.CheckError()
			oven.win:start()
--oven.gl.CheckError()
			oven.cake.start()
			oven.cake.canvas.start()
			if oven.now and oven.now.start then
				oven.now.start()
			end
--print("start preloader=off")
			if oven.preloader_enabled=="stop" then -- we turned on at stop so turn off at start
				oven.preloader_enabled=false
			end

		end

		function oven.stop()
			if not oven.started then return end
--print("stop preloader=on")
			oven.rebake(opts.preloader or "wetgenes.gamecake.spew.preloader").reset()
			oven.preloader_enabled="stop"
			oven.win:stop()
			oven.cake.stop()
			oven.cake.canvas.stop()
			if oven.now and oven.now.stop then
				oven.now.stop()
			end
			oven.started=false
		end

		function oven.clean()
			if oven.now and oven.now.clean then
				oven.now.clean()
			end
		end

		function oven.update()
--print(oven.ticks)

local prof={TOTAL=0}
if false then
	local ms=5
	profile.start("fi"..ms,function(thread, samples, vmstate)
		prof.TOTAL=prof.TOTAL+samples*ms
		local s=profile.dumpstack(thread, "l", 1)
		prof[s]=(prof[s] or 0 ) +samples*ms
	end)
end

			if oven.do_backtrace then
				oven.do_backtrace=false
				if oven.update_co then
					LOG( "oven" , debug.traceback(oven.update_co) ) -- debug where we are?
				else
					LOG( "oven" , debug.traceback() ) -- debug where we are?
				end
			end

--			oven.upnet_pause=nil -- release pause

			local cached_time=oven.time() -- cache time here to prevent possible race
			local f
			f=function()

				if oven.do_start then
					oven.start()
				end

				oven.change() -- run setup/clean codes if we are asked too

				if oven.frame_rate_limited() then
					oven.frame_time=oven.frame_time+oven.frame_rate -- step frame forward one tick
				end

--print( "UPDATE",math.floor(10000000+(oven.time()*1000)%1000000) )

				if oven.times then oven.times.update.start() end

				oven.msgs() -- send inputs to ups as fast as we can
				oven.ups.update() -- get new inputs from ups
				for _,m in ipairs(oven.ups.msgs) do oven.domsg(m) end -- and handle ups msgs

				for i,v in ipairs(oven.mods) do
					if v.update then
						v.update()
					end
				end

				if oven.now and oven.now.update then
					oven.now.update()
				end

				if oven.times then oven.times.update.stop() end

				if oven.frame_rate_limited() then
					if (oven.frame_time-oven.frame_rate)<cached_time then -- repeat until we are a frame ahead of real time
--					oven.upnet_pause="updates"
--print(oven.upnet_pause ,oven.frame_time , time)
						return f() -- tailcall
					end
				end

			end

			if not oven.update_co then -- create a new one
				oven.update_co=coroutine.create(f)
			end
			while coroutine.status(oven.update_co)~="dead" do
				if oven.tasks then -- update generic coroutines
					oven.tasks:update()
				end
				assert_resume(oven.update_co) -- run it, may need more than one resume before it finishes

--log( "oven" , debug.traceback(oven.update_co) ) -- debug where we are?

			end
			oven.update_co=nil

if false then
	profile.stop()
	if prof.TOTAL >= 10 then
		local list={}
		for n,v in pairs(prof) do
			list[#list+1]=string.format("%4i",v).." : "..n
		end
		table.sort(list)
		local c=0
		for i=#list,1,-1 do
			PRINT(list[i])
			c=c+1
			if c>=5 then break end
		end
		PRINT("")
	end
--	DUMP(oven.tasks.linda:dump())
end

		end

		oven.preloader_enabled=false -- do we need this? we is fast and it is broken

		function oven.preloader(sa,sb)
			sa=sa or ""
			sb=sb or ""

LOG("oven",sa.." : "..sb)

			if not oven.preloader_enabled then return end

			local t=oven.rebake("wetgenes.gamecake.images").get("fonts/basefont_8x8") -- font loaded test?
			if oven.win and t and t.gl then

				local p=oven.rebake(opts.preloader or "wetgenes.gamecake.spew.preloader")
				p.setup() -- warning, this is called repeatedly
				p.update(sa,sb)

				if not oven.preloader_time or ( oven.time() > ( oven.preloader_time + (1/60) ) ) then -- avoid frame limiting

					oven.preloader_time=oven.time()

					if wwin.hardcore and wwin.hardcore.swap_pending then -- cock blocked waiting for nacl draw code
						-- do not draw
					else
						oven.cake.canvas.draw()
						p.draw()
						oven.win:swap()
						oven.cake.images.adjust_mips() -- upgrade visible only
					end

--					if  oven.update_co and ( coroutine.status(oven.update_co)=="running" ) then
--						coroutine.yield()
--					end

				end

			end
		end

		function oven.draw()

			if wwin.hardcore and wwin.hardcore.swap_pending then
				return
			end -- cock blocked waiting for nacl draw code

			oven.cake.canvas.draw() -- prepare tempory buffers

--print( "DRAW",math.floor(10000000+(oven.time()*1000)%1000000) )

			if oven.times then oven.times.draw.start() end -- between calls to draw

			if oven.now and oven.now.draw then
				oven.now.draw()
			end

			for i,v in ipairs(oven.mods) do
				if v.draw then
					v.draw()
				end
			end

			if oven.times then oven.times.draw.stop() end -- draw is squify so just use it as the total time

			if oven.win then
				oven.win:swap()
				oven.cake.images.adjust_mips({force=true}) -- upgrade all textures
			end

		end

		local old_mouse={x=0,y=0}
		oven.frame_rate_auto_active=os.time()
		function oven.msgs() -- read and process any msgs we have from win:msg

			local cached_time=oven.time() -- cache time here to prevent possible race
			if oven.frame_rate_auto then -- auto pick
				if cached_time-oven.frame_rate_auto_active <= 10 then -- any recent activity in 10 seconds?

					if oven.frame_rate ~= oven.frame_rate_auto then -- wake up
						oven.frame_time=oven.time()
						oven.frame_rate=oven.frame_rate_auto
					end
				else -- no recent activity , so , sleepy time
					oven.frame_rate=(cached_time-oven.frame_rate_auto_active-10)*oven.frame_rate_auto -- wait longer for next frame
				end
			end

			if oven.win then
				for m in oven.win:msgs() do

--[[
if m.class=="key" and m.keyname=="menu" and m.action==1 then
print("requesting backtrace")
	oven.do_backtrace=true
end
]]

					if m.class=="mouse" then	-- need to fix x,y numbers
						m.x=m.x or old_mouse.x	-- restore if missing
						m.y=m.y or old_mouse.y
						old_mouse.x=m.x			-- remember locally
						old_mouse.y=m.y
						m.xraw=m.x				-- remember in message
						m.yraw=m.y
						oven.frame_rate_auto_active=cached_time
						if oven.msg_view then
							oven.msg_view.msg(m) -- fix mouse coords using this view
						end
					elseif m.class=="touch" then	-- need to fix x,y numbers
						m.xraw=m.x				-- remember in message
						m.yraw=m.y
						oven.frame_rate_auto_active=cached_time
					elseif m.class=="key" or m.class=="padkey" then -- key or joystick buttons
						oven.frame_rate_auto_active=cached_time
					elseif m.class=="resize" then -- window resize
						oven.frame_rate_auto_active=cached_time
					elseif m.class=="close" then -- window has been closed so do a shutdown
						if oven.enable_close_window then
							oven.next=true
						end
					elseif m.class=="app" then -- androidy
LOG("oven","caught : ",m.class,m.cmd)
						if		m.cmd=="init_window" then
							oven.do_start=true
						elseif	m.cmd=="gained_focus"  then
							oven.focus=true
						elseif	m.cmd=="lost_focus"  then
							oven.focus=false
						elseif	m.cmd=="term_window"  then
							oven.do_stop=true
						end
					end

					oven.ups.msg(m) -- manage the msgs the new way (they belong to ups now)

				end
			end
		end
		function oven.domsg(m)
			if not oven.preloader_enabled then -- discard all other msgs during preloader

--				oven.skeys.msg(m)
				for i=#oven.mods,1,-1 do -- run it through the mods backwards, so the topmost layer gets first crack at the msgs
					local v=oven.mods[i]
					if m and v and v.msg then
						m=v.msg(m) -- mods can choose to eat the msgs, they must return it for it to bubble down
					end
				end
				if m and oven.now and oven.now.msg then
					oven.now.msg(m)
				end

			end
		end

-- a busy blocking loop, or not, if we are running on the wrong sort
-- of system it just returns and expects the other functions
-- eg oven.serv_pulse to be called when necesary.
		function oven.serv()

			oven.focus=true -- hack, default to focused
			oven.do_start=true

			if oven.win.noblock then
				return oven -- and expect  serv_pulse to be called as often as possible
			end

			local ok,ret=xpcall(function()
				local finished
				repeat
					finished=oven.serv_pulse(oven)
				until finished
			end,debug.traceback)
			if not ok then print(ret) end
			
			oven.finish()

		end

		function oven.catchup()
			oven.frame_time=oven.time()-- zip forwards
			if oven.upnet then
				oven.upnet.catchup()
			end
			if oven.now and oven.now.catchup then
				oven.now.catchup()
			end
		end
		
		function oven.serv_pulse()
			if oven.finished then return true end

			if oven.frame_rate_limited() then
				if oven.frame_time<(oven.time()-0.250) then
				   oven.catchup()
				end
			end

			oven.msgs()

			oven.cake.update()

			if oven.do_stop then
				oven.do_stop=false
				oven.stop()
			end

			if ( oven.started or oven.do_start ) and oven.focus then -- rolling or ready to roll
				oven.update()
				if oven.started then -- can draw?
					oven.draw()
				end

				-- we probably have some time so try and run a small gc step
				-- this should take less than 1ms
				toaster.garbage_collect_step()

				if oven.frame_rate_limited() then
					while (oven.frame_time-(oven.frame_rate or 0))>oven.time() do
						oven.msgs() -- keep handling msgs?
						if wwin.hardcore.sleep then
							wwin.hardcore.sleep(0.001) -- sleep here 1ms until we need to update again
						end
					end
				end

			else
				if wwin.hardcore.sleep then
					wwin.hardcore.sleep(1/10)
				end
			end
		end

		function oven.finish()
			if oven.win then
				oven.win:show("win") -- this may restore original resolution
			end
			oven.tasks:sqlite({cmd="close"}) -- shut down the default sqlite databsase so we finish with any out standing writes
			wtongues.save() -- last thing we do is remember any new text ids used in this run
			oven.tasks:delete()
		end

	return oven

end
