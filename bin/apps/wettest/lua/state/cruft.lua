

local _G=_G

local win=win

local print=print

local oldmain_test_pre=oldmain and oldmain.test_pre
local oldmain_test_msg=oldmain and oldmain.test_msg
local oldmain_test_post=oldmain and oldmain.test_post
local active=false


local state = require("state.main")


module("state.cruft")

function setup()
end

function clean()
end

function update()

	if oldmain_test_pre and oldmain_test_msg and oldmain_test_post and not active and not _G.main_next then

		active=true
		win.setwin("call_update",false)
		
--		state.clean()
		
		-- simple old main loop setup
		oldmain_test_pre("avatar")
		local n=0
		while n==0 do
			n=oldmain_test_msg()
		end
		oldmain_test_post()

		win.setwin("call_update",true)
		active=false
		
		_G.goto("menu")
		
--		state.setup()
	end
	
end
