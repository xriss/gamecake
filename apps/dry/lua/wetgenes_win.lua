

module(...,package.seeall)

local wstr=require("wetgenes.string")
local wwin=require("wetgenes.win")



function test_win()

--print(wstr.dump(wwin))

local win=assert(wwin.create())

--print(wstr.dump(win))


--[[
for i=1,10*50 do

	repeat
		local t,a,b,c,d=win:msg()
		if t then
			print(tostring(t),tostring(a),tostring(b),tostring(c),tostring(d))
		end
	until t==nil
	

	win:sleep(1/50)
end
]]
	win:sleep(2)
	

end
