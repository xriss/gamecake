

local _G=_G

local win=win

local table=table
local ipairs=ipairs

local apps=apps

local gl=require("gl")

local grd=require("grd")


local function print(...) _G.print(...) end


module(...)


	
	
	
function setup()

	
	print("setup")

end


	

mdown=false

function mouse(act,x,y,key)

	local x,y=win.mouse23d(640,480,x,y)

	if act=="down" then
	
		mdown=true

--		table.insert(items, new_item((x)/10,(y)/10,0) )
	
	elseif act=="up" then
	
		mdown=false
		
	end

end


	
function clean()
end



function update()

end


function draw()

	win.begin()
	gl.ClearColor(14/15,14/15,14/15,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	win.project23d(480/640,2,1024)
	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()


end


