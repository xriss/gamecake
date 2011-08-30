

local _G=_G

local win=win

local table=table
local ipairs=ipairs
local string=string
local math=math
local os=os

-- a rogue like

local unpack=unpack

local tostring=tostring

local require=require

local gl=gl



local function print(...) _G.print(...) end



local _M=module("state.rouge")


yarn=require("yarn")

function setup()

	yarn.setup()
	
	print("setup")

end

function keypress(ascii,key,act)

	if yarn.menu.keypress(ascii,key,act) then -- give the menu first chance to eat this keypress
		yarn.level.key_clear() -- stop level repeats
	else
		yarn.level.keypress(ascii,key,act)
	end
end


function mouse(act,x,y,key)

	if act=="down" then

	end

end


	
function clean()
	yarn.clean()
end



function update()

	yarn.update()
	
end

function draw()

	yarn.draw()

	win.begin()
	gl.ClearColor(0,0,0.25,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	win.project23d(480/640,1,1024)
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()
					
local i=0
local t={}

	gl.Translate(-320,240, -240)
		
	for y=0,yarn.asc_yh-1 do
	
		win.font_debug.set(0,y*-16,0xffffffff,16)
		
		for x=0,yarn.asc_xh-1 do
		
			i=1+x+y*yarn.asc_xh
			t[x+1]=yarn.asc[i]%256
			
		end
		
		local s=string.char(unpack(t))
		
		win.font_debug.draw(s)
		
	end
	
	
--	win.font_debug.set(0,0,0xffffffff,1.6)
--	win.font_debug.draw("asdasdasdsd")
		
end


