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


-- Note: don't allow user scripts to change implementation.
-- Check for globals with "cat lua.lua | luac -p -l - | grep ETGLOBAL"
local _G = _G
local assert = assert
local collectgarbage = collectgarbage
local loadfile = loadfile
local loadstring = loadstring
local pcall = pcall
local rawget = rawget
local select = select
local tostring = tostring
local type = type
local unpack = unpack
local xpcall = xpcall
local io_stderr = io.stderr
local io_stdout = io.stdout
local io_stdin = io.stdin
local string_format = string.format
local string_sub = string.sub
local os_getenv = os.getenv
local os_exit = os.exit


local progname = LUA_PROGNAME



-- attempt to find lua code relative to executable
local apps=require("apps")
apps.default_paths()

-- need to be able to auto mount some zip files for reading from
local wzips=require("wetgenes.zips")



local function print_usage()
-- need to use print so we can overload it on android
  print(string_format([=[
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
	print( string.format("%s%s",pname and string_format("%s: ", pname) or "",msg) )
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
    if string_sub(msg, -#ender) == ender then
      return true
    end
  end
  return false
end


local function pushline (firstline)
  local prmt = get_prompt(firstline)
  io_stdout:write(prmt)
  io_stdout:flush()
  local b = io_stdin:read'*l'
  if not b then return end -- no input
  if firstline and string_sub(b, 1, 1) == '=' then
    return "return " .. string_sub(b, 2)  -- change '=' to `return'
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
        l_message(progname, string_format(
            "error calling %s (%s)",
            LUA_QL("print"), msg))
      end
    end
  end
  io_stdout:write"\n"
  io_stdout:flush()
  progname = oldprogname
end




local function handle_luainit()
  local init = os_getenv(LUA_INIT)
  if init == nil then
    return  -- status OK
  elseif string_sub(init, 1, 1) == '@' then
    dofile(string_sub(init, 2))
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
			if not dostring(chunk, "=(command line)") then
				os_exit(1)
			end

		elseif arg:sub(1,2)=="-l" then

			local fname=arg:sub(3)
			if fname=="" then
				fname=assert(args[idx+1])
				skip=skip+1
			end
			if not dolibrary(fname) then
				os_exit(1)
			end

		elseif arg=="-i" then

			interact=true 

		elseif arg=="-h" then

			print_usage()
			os_exit(0)

		elseif arg=="-v" then

			print_version()
			os_exit(0)

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
	os_exit(0)
end
