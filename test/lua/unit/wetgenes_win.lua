

module(...,package.seeall)

local wstr=require("wetgenes.string")
local gl=require("gles")


-- disable
local function test_win()

local wwin=require("wetgenes.win")

--print(wstr.dump(wwin))

local win=assert(wwin.create({}))

	win:context()
	
--print(wstr.dump(win))



for i=1,2*50 do

	repeat
		local t,a,b,c,d=win:msg()
		if t then
--			print(tostring(t),tostring(a),tostring(b),tostring(c),tostring(d))
		end
	until t==nil
	
	gl.ClearColor((i%15)/15,(i%15)/15,(i%15)/15,(i%15)/15)
	gl.Clear(gl.COLOR_BUFFER_BIT)
	
	win:swap()
	win:sleep(1/50)
end

	win:sleep(2)
	

end
