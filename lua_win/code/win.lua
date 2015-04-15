--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local jit=jit

local wsbox=require("wetgenes.sandbox")
local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")
local bit=require("bit")

local win={}
local base={}

local softcore=require("wetgenes.win.core") -- we keep some generic C functions here

local hardcore -- but use different hardcores depending on the system we compiled for and are running on

local posix -- set if we are a posix system

local args={...}


base.noblock=false
base.flavour="raw"

if type(args[2]=="table" ) then -- you can force a core by using a second arg to require
	hardcore=args[2]
end

-- see if we have an SDL2 build
if not hardcore then
	local suc,dat=pcall(function() return require("SDL") end )
--	if suc then hardcore=require("wetgenes.win.sdl") base.flavour="sdl" end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.emcc") end )
	if suc then hardcore=dat base.flavour="emcc" base.noblock=true end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.nacl") end )
	if suc then hardcore=dat base.flavour="nacl" base.noblock=true end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.android") end )
	if suc then hardcore=dat base.flavour="android"
--		posix=require("wetgenes.win.posix")
	end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.raspi") end )
	if suc then hardcore=dat base.flavour="raspi"
		posix=require("wetgenes.win.posix")
	end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.windows") end )
	if suc then hardcore=dat base.flavour="windows" end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.linux") end )
	if suc then hardcore=dat base.flavour="linux"
		posix=require("wetgenes.win.posix")
	end
end

if not hardcore then
	local suc,dat=pcall(function() return require("wetgenes.win.osx") end )
	if suc then hardcore=dat base.flavour="osx"
		posix=require("wetgenes.win.posix")
	end
end

print(base.flavour)

win.hardcore=hardcore
win.softcore=softcore
win.posix=posix

-- a dir to store config or user generated files in
win.files_prefix="./files/"
-- a dir to store cache files in, may auto delete but also should be deleted by app
win.cache_prefix="./cache/"


local meta={}
meta.__index=base

setmetatable(win,meta)


function win.screen()
	local it={}
	if hardcore.screen then
--		print("SCREEN",hardcore.screen())
		it.width,it.height=hardcore.screen()
	else
		it.width,it.height=0,0
	end
	return it
end

-- key names are given in a raw OS flavour,
-- this maps tries to map these names to more generic ones
-- really you can never be sure what name a key has...
win.generic_keymap={

-- windows/nacl/emcc/osx/raspi patch, as we aim to use the linux key names
-- might not catch everything here its mostly added when I see its wrong

	["backspace"]="back",
	["kp_enter"]="enter",
	["oem_3"]="grave",
	["esc"]="escape",

	["lshift"]="shift_l",
	["rshift"]="shift_r",

	["left shift"]="shift_l",
	["right shift"]="shift_r",
	
	["lcontrol"]="control_l",
	["rcontrol"]="control_r",
	
	["left control"]="control_l",
	["right control"]="control_r",

	["lmenu"]="alt_l",
	["rmenu"]="alt_r",

	["left alt"]="alt_l",
	["right alt"]="alt_r",

	["page up"]="prior",
	["page_up"]="prior",

	["page down"]="next",
	["page_down"]="next",

	["keypad /"]="kp_divide",
	["keypad *"]="kp_multiply",
	["keypad -"]="kp_subtract",
	["keypad +"]="kp_add",
	
	["`"]="grave",

}


function win.keymap(key)
	key=key:lower()
	return win.generic_keymap[key] or key
end

function win.load_run_init(args)

	local zips=require("wetgenes.zips")

-- Now load and run lua/init.lua which initalizes the window and runs the app

	local s=args.init or zips.readfile("lua/init.lua")
	
	assert(s) -- sanity, may want a seperate path for any missing init.lua ?

	if s:sub(1,2)=="#!" then
		s="--"..s -- ignore hashbang on first line
	end
	
	local f=assert(loadstring(s))
	
	return f(args)

end

--
-- Special android entry points, we pass in the location of the apk
-- this does things that must only happen once
--
function win.android_start(apk)

-- replace print
	_G.print=hardcore.print
	print=_G.print


	if jit then -- start by tring to force a jit memory allocation
		print(jit.status())
		require("jit.opt").start("sizemcode=256","maxmcode=256")
		for i=1,1000 do end
		print(jit.status())
  	end

	if hardcore.get_memory_class then
		print("memory class "..hardcore.get_memory_class().." meg")
	end

--	if jit and jit.off then
--		jit.off()
--		hardcore.print("LUA JIT OFF")
--	end -- jit breaks stuff?



	win.apk=apk
	local zips=require("wetgenes.zips")
	zips.add_apk_file(win.apk)
	
	win.files_prefix=hardcore.get_files_prefix().."/"
	win.cache_prefix=hardcore.get_cache_prefix().."/"
--	win.cache_prefix=hardcore.get_files_prefix().."/cache/"
	win.external_prefix=hardcore.get_external_prefix().."/"

	win.smell=hardcore.smell_check()

--print(win.files_prefix)
--print(win.cache_prefix)

--print("ANDROID_SETUP with ",apk)
	
	return win.load_run_init({})
end


--
-- Special nacl entry points, we pass in the url of the main zip we wish to load
-- this does things that must only happen once
--
local main -- gonna have to cache the main state here
function win.nacl_start(args)
--	_G.print=hardcore.print
--	print=_G.print
	
--print("nacl start ")

	if type(args)=="string" then -- just a zip name
		args={zips={args}}
	end

	local zips=require("wetgenes.zips")
	
-- we want nacl msgs to go here.
	_G.nacl_input_event=function(...) return hardcore.input_event(...) end
	
	local lastpct=-1
	local loads={}
	for i,v in ipairs(args.zips) do
		loads[i]={v,100,0}
	end
	for i,v in ipairs(args.zips) do
		local f=function(i,v)
		hardcore.getURL(v,function(mem,total,progress)	
			local l=loads[i]
			
			if total==0 then
				hardcore.print(string.format("Failed to load %s (probably a missing size in the html headers)",v))
				return
			end 
			
			l[2]=total
			l[3]=progress
			
			local t=0
			local p=0
			for i,v in ipairs(loads) do
				t=t+v[2]
				p=p+v[3]
			end

if args.progress then -- callback with progress
	pcall(function() args.progress(t,p) end)
else
	local pct=math.floor(100*p/t)
	if lastpct<pct then
		hardcore.print(string.format("Preloading zips : %02d%%",pct)) -- progress
		lastpct=pct
	end
end

			if total<=progress then -- this file has loaded

				zips.add_zip_data(mem)

				if t<=p then --all loaded
					main=win.load_run_init(args)
				end
			end
		end)
		end
		f(i,v)
	end

	if not loads[1] then -- no zips?
		main=win.load_run_init(args)
	end
	
--print("nacl start done")

end

function win.nacl_pulse() -- called 60ish times a second depending upon how retarted the browser is

	if main then
		main:serv_pulse()
	end
end

function win.emcc_start(args)
	local zips=require("wetgenes.zips")
	assert(zips.add_zip_file(args.zip or "gamecake.zip"))
	local main=win.load_run_init(args)
	require("global").gamecake_pulse=function() main:serv_pulse() end
end

function win.create(opts)

	local w={}
	setmetatable(w,meta)
	
	if hardcore.create then
		w[0]=assert( hardcore.create(opts) )
	end
	w.msgstack={} -- can feed "fake" msgs into here (fifo stack) with table.push
	w.width=0
	w.height=0
	w.overscale=opts.overscale or 1
	
	base.info(w)

	if posix and base.flavour=="raspi" then
		posix.win_translate_msg=function(m) return posix.win_translate_msg_keys_and_mouse(w,m) end -- need to make real keyboard/mouse msgs
	end

	return w
end

function win.js_post(m,d)
	if hardcore.js_post then
		hardcore.js_post(m,d)
	end
end


function base.destroy(w)
	if hardcore.destroy then
		hardcore.destroy(w[0],w)
	end
end

function base.show(w,s)
	if hardcore.show then
		hardcore.show(w[0],s)
	end
end

function base.info(w)
	if hardcore.info then
		hardcore.info(w[0],w)
--		print("WH",w.width,w.height)
	end
end

function base.context(w,opts)
	if hardcore.context then
		hardcore.context(w[0],opts)
	end
end

function base.start(w)
	if hardcore.start then
		hardcore.start(w[0])
	end
end

function base.stop(w)
	if hardcore.stop then
		hardcore.stop(w[0])
	end
end

function base.swap(w)
	if hardcore.swap then
		hardcore.swap(w[0])
	end
end

function base.peek(w)
	if hardcore.peek then
		return hardcore.peek(w[0])
	end
end

function base.wait(w,t)
	if hardcore.wait then
		hardcore.wait(w[0],t)
	end
end

-- a msg iterator
function base.msgs(w)
	return function() return w:msg() end
end

-- push a msg onto the msgstack, so it feeds back into the input loop
function base.push_msg(w,m)
	w.msgstack[#w.msgstack+1]=m
end

-- get the next msg or return nil if there are no more
function base.msg(w)

	local m
	
	if not m and w.msgstack[1] then
		m=table.remove(w.msgstack,1)
	end
	if not m and hardcore.msg then
		m=hardcore.msg(w[0])
	end
	if not m and posix then
		m=base.posix_msg(w)
	end
	if not m and hardcore.smell_msg then
		m=hardcore.smell_msg() --hardcoded stuff
		if m then
			m=wsbox.lson(m)
--			print(wstr.dump(m))
		end
	end

	if m then -- proccess the msg some more
	
		if m.class=="mouse" then
			if not m.keyname then -- apply mouse buttn names if all we have is a keycode
				if m.keycode==1 then m.keyname="left"
				elseif m.keycode==2 then m.keyname="middle"
				elseif m.keycode==3 then m.keyname="right"
				elseif m.keycode==4 then m.keyname="wheel_add"
				elseif m.keycode==5 then m.keyname="wheel_sub"
				else m.keyname="mouse"
				end
			end
		end
	
		if m.keyname then -- run it through our keymap probably just force it to lowercase.
			m.keyname=win.keymap(m.keyname)
		end
		
	end
	
	return m
end

function base.jread(w,n)
	if hardcore.jread then
--print("jread")	
		local pkt=hardcore.jread(w[0],n)
		local tab
		if pkt then
			tab=pack.load(pkt,{"u32","time","s16","value","u8","type","u8","number"})
			tab.class="joy"
		end
		return tab
	end
end


function base.sleep(...)
	if hardcore.sleep then
		for i,v in ipairs({...}) do	-- ignore first arg if it is a table so we can call with :
			if type(v)=="number" then
				hardcore.sleep(v)
			end
		end
	end
end
win.sleep=base.sleep

function base.time()
	if hardcore.time then
		return hardcore.time()
	else
		return os.time()
	end
end
win.time=base.time

function base.smell_check()
	if hardcore.smell_check then
		return hardcore.smell_check()
	end
end

function base.glyph_8x8(n)
	return softcore.glyph_8x8(n)
end
win.glyph_8x8=base.glyph_8x8

function base.posix_open_events(w)
	if not posix then return end
	return posix.win_open_events(w)
end

function base.posix_read_events(w)
	if not posix then return end
	return posix.win_read_events(w)
end

function base.posix_close_events(w)
	if not posix then return end
	return posix.win_close_events(w)
end

function base.posix_msg(w)
	if not posix then return end
	return posix.win_msg(w)
end

function base.get_memory_class(w)
	if hardcore.get_memory_class then
		return hardcore.get_memory_class()
	end
end
return win
