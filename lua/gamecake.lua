-- lua.lua - Lua 5.1 interpreter (lua.c) reimplemented in Lua.
--
-- WARNING: This is not completed but was quickly done just an experiment.
-- Fix omissions/bugs and test if you want to use this in production.
-- Particularly pay attention to error handling.
--
-- (c) David Manura, 2008-08
-- Licensed under the same terms as Lua itself.
-- Based on lua.c from Lua 5.1.3.
-- Improvements by Shmuel Zeigerman.

-- HAXTBH

-- remorked the argument handling to be more lua less c but mostly the same results, probably

-- Variables analogous to those in luaconf.h
local LUA_INIT = "LUA_INIT"
local LUA_PROGNAME = "lua"
local LUA_PROMPT   = "> "
local LUA_PROMPT2  = ">> "
local function LUA_QL(x) return "'" .. x .. "'" end

-- Variables analogous to those in lua.h
local LUA_RELEASE   = "Lua x.x.x"
local LUA_COPYRIGHT = "Copyright (C) 1994-2008 Lua.org, PUC-Rio"

local progname = LUA_PROGNAME



-- attempt to find lua code relative to executable
do
	--
	-- get/set current dir
	--
	local get_cd=function()

		local lfs ; pcall( function() lfs=require("lfs") end )

		if lfs then	return string.gsub(lfs.currentdir(),'\\','/') end
		
		return "." -- lfs is not available

	end

	local set_cd=function(str)
		local lfs=require("lfs")

		lfs.chdir(str)

	end

	local file_exists=function(str)
		local fp=io.open(str,"r")
		if fp then fp:close() return true end
		return false
	end

		
	local setpaths = function(dll,dirs)

		if dll then
			local cpath={}
			for i,v in ipairs(dirs) do
				cpath[#cpath+1]=v .. "?." .. dll
				cpath[#cpath+1]=v .. "?/init." .. dll
			end
			local newpath=table.concat(cpath,";")
			if not string.find(package.cpath,newpath,1,true) then -- only add once
				package.cpath=newpath..";"..package.cpath
			end
		end
		
		local path={}
		for i,v in ipairs(dirs) do
			path[#path+1]=v .. "?.lua"
			path[#path+1]=v .. "?/init.lua"
			path[#path+1]=v .. "lua/?.lua"
			path[#path+1]=v .. "lua/?/init.lua"
		end
		local newpath=table.concat(path,";")
		if not string.find(package.path,newpath,1,true) then -- only add once
			package.path=newpath..";"..package.path
		end

	end

	--
	-- find where our exe lives
	--
	local find_bin = function ()

		local dir=get_cd()
		local exe="."
		local exe_path
		pcall(function() exe_path=require("wetgenes.win.core").get_exe_path() end)
		if exe_path then -- remove filename
			local dir,name,ext=string.match(exe_path,"(.-)([^\\/]-%.?([^%.\\/]*))$")
			exe=dir:sub(1,-2) -- remove trailing / or \
		end

	-- print(dir,exe)

		local tdirs={ -- look in these dirs
			dir,
			dir.."/..",
			dir.."/../..",
			exe,
			exe.."/..",
			exe.."/../..",
		}
		local bin_dir=dir.."/"
		for i=1,#tdirs do local v=tdirs[i]
			if file_exists(v.."/lua/gamecake.lua") then bin_dir=v.."/" break end -- found a bin dir?
		end

		return bin_dir
	end

	--
	-- find our bin dir and set search for all lua files under there, makes debuging a bit easier
	-- than using the builtin strings. Also lets us pick up any dlls in there.
	--
	local default_paths = function (appdir)
	-- we are looking for a dir/lua/name.lua and dir will be our base dir so look in various places

		if not pcall( function() return require("lfs") end ) then return end -- not possible without lfs

		local osflavour="win"
		local os_shell=os.getenv("SHELL")
		if os_shell and string.sub(os_shell,1,5)== "/bin/" then -- if your shell is not here then we assume windows...
			osflavour="nix"
		end

		local dll="dll"
		if osflavour=="nix" then dll="so" end

		local luadir=find_bin()
		
		if appdir and appdir~="" then -- use this as app dir

			appdir=appdir:gsub("\\","/").."../"
			setpaths(dll,{luadir,appdir,"./"})

		else
		
			appdir=get_cd().."/" -- use cd as app dir	
			setpaths(dll,{luadir,"./"})
			
		end
		
		local wzips=require("wetgenes.zips") -- and search for data+lua here
		wzips.paths[#wzips.paths+1]=appdir

		return luadir,appdir
		
	end
	
default_paths()

end


-- need to be able to auto mount some zip files for reading from
local wzips=require("wetgenes.zips")



local function print_usage()
-- need to use print so we can overload it on android
  print(string.format([=[
%s [options] [mountfile.zip|.cake|.apk] [script -- [script_args]]
Script filenames that end in .fun.lua will auto run inside a fun oven.
Mounting a zip will allow you to require lua code from within its lua directory.
Available options are:
	-e stat  execute string 'stat'
	-l name  require library 'name'
	-i       enter interactive mode after executing 'script'
	-v       show version information
	--       stop handling options
	-        execute stdin and stop handling options
]=],progname))
end


local function l_message (pname, msg)
-- might need to replace global print, so do not cache (eg -landroid will mess with globals)
	print( string.format("%s%s",pname and string.format("%s: ", pname) or "",msg) )
end

--------------------------------------------------------------------------
-- all the old codes, should reqrite and add better interactive mode
-- fix the gamecake console interactive mode and run it in a terminal too, maybe?
--------------------------------------------------------------------------

local function report(status, msg)
  if not status and msg ~= nil then
    msg = (type(msg) == 'string' or type(msg) == 'number') and tostring(msg)
          or "(error object is not a string)"
    l_message(progname, msg);
  end
  return status
end

local function tuple(...)
  return {n=select('#', ...), ...}
end

local function traceback (message)
  local tp = type(message)
  if tp ~= "string" and tp ~= "number" then return message end
  local debug = _G.debug
  if type(debug) ~= "table" then return message end
  local tb = debug.traceback
  if type(tb) ~= "function" then return message end
  return tb(message, 2)
end

local function docall(f, ...)
  local tp = {...}  -- no need in tuple (string arguments only)
  local F = function() return f(unpack(tp)) end
--  setsignal(true)
  local result = tuple(xpcall(F, traceback))
--  setsignal(false)
  -- force a complete garbage collection in case of errors
  if not result[1] then collectgarbage("collect") end
  return unpack(result, 1, result.n)
end

local function dofile(name)
  local f, msg = loadfile(name)
  if f then f, msg = docall(f) end
  return report(f, msg)
end

local function dostring(s, name)
  local f, msg = loadstring(s, name)
  if f then f, msg = docall(f) end
  return report(f, msg)
end

local function dolibrary (name)
  return report(docall(_G.require, name))
end

local function print_version()

	if jit then -- dump basic jit info
		local t={jit.version,jit.status()}
		t[2]=tostring(t[2])
	--	t[#t+1]="jit_mcode_size="..toaster.jit_mcode_size.."k"
		l_message(nil, table.concat(t,"\t") )
	end

	local s=require("wetgenes.gamecake.core").get_version()
	l_message(nil, s)

--  l_message(nil, LUA_RELEASE .. "  " .. LUA_COPYRIGHT)
end


--FIX? readline support
local history = {}
local function saveline(s)
--  if #s > 0 then
--    history[#history+1] = s
--  end
end


local function get_prompt (firstline)
  -- use rawget to play fine with require 'strict'
  local pmt = rawget(_G, firstline and "_PROMPT" or "_PROMPT2")
  local tp = type(pmt)
  if tp == "string" or tp == "number" then
    return tostring(pmt)
  end
  return firstline and LUA_PROMPT or LUA_PROMPT2
end


local function incomplete (msg)
  if msg then
    local ender = LUA_QL("<eof>")
    if string.sub(msg, -#ender) == ender then
      return true
    end
  end
  return false
end


local function pushline (firstline)
  local prmt = get_prompt(firstline)
  io.stdout:write(prmt)
  io.stdout:flush()
  local b = io.stdin:read'*l'
  if not b then return end -- no input
  if firstline and string.sub(b, 1, 1) == '=' then
    return "return " .. string.sub(b, 2)  -- change '=' to `return'
  else
    return b
  end
end


local function loadline ()
  local b = pushline(true)
  if not b then return -1 end  -- no input
  local f, msg
  while true do  -- repeat until gets a complete line
    f, msg = loadstring(b, "=stdin")
    if not incomplete(msg) then break end  -- cannot try to add lines?
    local b2 = pushline(false)
    if not b2 then -- no more input?
      return -1
    end
    b = b .. "\n" .. b2 -- join them
  end

  saveline(b)

  return f, msg
end


local function dotty ()
  local oldprogname = progname
  progname = nil
  while true do
    local result
    local status, msg = loadline()
    if status == -1 then break end
    if status then
      result = tuple(docall(status))
      status, msg = result[1], result[2]
    end
    report(status, msg)
    if status and result.n > 1 then  -- any result to print?
      status, msg = pcall(_G.print, unpack(result, 2, result.n))
      if not status then
        l_message(progname, string.format(
            "error calling %s (%s)",
            LUA_QL("print"), msg))
      end
    end
  end
  io.stdout:write"\n"
  io.stdout:flush()
  progname = oldprogname
end




local function handle_luainit()
  local init = os.getenv(LUA_INIT)
  if init == nil then
    return  -- status OK
  elseif string.sub(init, 1, 1) == '@' then
    dofile(string.sub(init, 2))
  else
    dostring(init, "=" .. LUA_INIT)
  end
end




--------------------------------------------------------------------------
-- mostly new arg handling codes
--------------------------------------------------------------------------

local args = _G.arg or {}
if args[0] and #args[0] > 0 then progname = args[0] end
handle_luainit()

local load_script=function(fname)
	local code=wzips.readfile(fname) -- check file system and mounted zips
	if code then
		if code:sub(1,2)=="#!" then
			code="--"..code -- ignore hashbang on first line
		end
	else
		error("missing file : "..fname)
	end
	return code
end

local do_script=function(fname,args)
	local code=load_script(fname)
	local func = assert( loadstring(code,fname) )
	assert( docall( func, unpack(args) ) )
	-- continue here if no error
end

local do_fun=function(fname,args)
	
	local wwin=require("wetgenes.win")

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
		fun=fname,
		fps=60,
		icon=[[
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
. . d d d d d d d d d d d d . . 
. . d d d d d d d d d d d d . . 
. . d d d d d d d d d d d d . . 
. . . . . d d d d d d . . . . . 
. . . . . d d d d d d . . . . . 
. . . . . d d d d d d . . . . . 
. . d d d d d d d d d d d d . . 
. . d d d d d d d d d d d d . . 
. . d d d d d d d d d d d d . . 
. . d d d . . . . . . d d d . . 
. . d d d . . . . . . d d d . . 
. . d d d . . . . . . d d d . . 
. . . . . . . . . . . . . . . . 
. . . . . . . . . . . . . . . . 
]],
		unpack(args)
	}

	math.randomseed( os.time() ) -- try and randomise a little bit better

	-- setup oven with vanilla cake setup and save as a global value
	global.oven=require("wetgenes.gamecake.oven").bake(opts).preheat()

	-- this will busy loop or hand back control depending on the system we are running on, eitherway opts.start will run next 
	return oven:serv()
end

local none=true
local interact
local pipe
local script
local script_args={}
local script_try
local skip=0
for idx=1,#args do
	local arg=args[idx]
	if skip>0 then
		skip=skip-1
	else

		if arg=="--" then -- everything else is script_args
			for i=idx+1,#args do
				script_args[#script_args+1]=args[i]
			end
			break
		end 
		if arg=="-"  then pipe=true break end -- ignore everything else and pipe in

		if     arg:sub(1,2)=="-e" then
			none=false

			local chunk = arg:sub(3)
			if chunk=="" then
				chunk=assert(args[idx+1])
				skip=skip+1
			end
			for i=idx+1,#args do
				if args[i]~="--" then
					script_args[#script_args+1]=args[i]
				end
			end
			script_args[-1]=progname
			script_args[0]="=(command line)"
			_G.arg=script_args
			if not dostring(chunk, "=(command line)") then
				os.exit(1)
			end

		elseif arg:sub(1,2)=="-l" then

			local fname=arg:sub(3)
			if fname=="" then
				fname=assert(args[idx+1])
				skip=skip+1
			end
			if not dolibrary(fname) then
				os.exit(1)
			end

		elseif arg=="-i" then

			interact=true 

		elseif arg=="-h" then

			print_usage()
			os.exit(0)

		elseif arg=="-v" then

			print_version()
			os.exit(0)

		else

			if arg:sub(-4)==".apk" then -- mount apk
				script_auto=true
				wzips.add_apk_file(arg)
			elseif arg:sub(-4)==".zip" then -- mount zip
				script_auto=true
				wzips.add_zip_file(arg)
			elseif arg:sub(-5)==".cake" then -- mount cake
				script_auto=true
				wzips.add_zip_file(arg)
			else
				if ( not script ) and arg:sub(1,1)~="-" then -- script can not start with -
					script=arg
				else
					script_args[#script_args+1]=arg
				end
			end

		end
	end
end

if not script and script_auto then
	if wzips.exists("lua/init.lua") then -- use this file if it exists
		script="lua/init.lua"
	end
end
if script then
	script_args[-1]=progname
	script_args[0]=script
	_G.arg=script_args
	none=false
	if script:sub(-8)==".fun.lua" then
		do_fun(script,script_args)
	else
		do_script(script,script_args)
	end
end

if interact then
	print_version()
	dotty()
elseif pipe then
	dofile(nil)  -- executes stdin as a file
elseif none then
	print_usage()
	os.exit(0)
end
