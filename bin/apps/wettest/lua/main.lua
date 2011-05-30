
local wetquire=require("wetquire")
wetquire.overload() -- replace require and module

--
-- Wet test main start code
--
-- arg[...] is set to the commandline as this program is invoked
--
-- need to setup windows etc and run whatever app or state is requested
--
-- print will also output to the debug console under windows as no console may be available
--

print("start\n")

main_next=main_next or arg[2]

print( "next->" .. (main_next or "") ) -- start state

print( arg[1] or "" ) -- app
print( arg[2] or "" ) -- state
print( arg[3] or "" ) -- command
print( arg[4] or "" ) -- hwnd

--print( main.test() or "" )



lanes=require("lanes")

local f=function()

-- special datas for state to use...

wet_setup_comandline=nil--arg[3]
wet_setup_hwnd=nil--arg[4]

	local f=loadfile(wetlua.dir.."lua/state/main.lua")
	f()
	
end


thread={}
	thread.linda=lanes.linda()
	thread.worker=lanes.gen("*",{["globals"]={
		arg=arg,
		oldmain=main,
		main_next=main_next,
		wetlua=wetlua
	}},f)(thread.linda,1)

-- wait for thread? not if this is running under moz...
--	if not arg[2] then 
		local t=thread.worker[1]
--	end
	

-- no lanes?
--f()


