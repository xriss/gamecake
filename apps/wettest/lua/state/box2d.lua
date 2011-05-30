

local _G=_G

local win=win

local table=table
local ipairs=ipairs

local gl=gl

local box2d=require("box2d.wrap")


local function print(...) _G.print(...) end


module("state.box2d")


local items={}
local world	
local ground	
	
	
	
function setup()

	world=box2d.world({gravity={0,-10}})

	ground = world.body{}
	ground.shape{box={width=64,height=10},density=1,friction=0.3}
	ground.set{x=0,y=-24-10,a=0}


		
	table.insert(items, new_item(0,10,0) )
	
	print("setup")

end



function new_item(x,y,a)

	local it={}

	it.text="hello"
	it.ts=1.6
	it.tc=0xff00ff00
	
	it.tx,it.ty=win.font_debug.size(it.text,it.ts)
	
	it.tx=-(it.tx/2)
	it.ty= (it.ty/2)

	it.body=world.body{}
	it.body.shape{box={width=-it.tx,height=it.ty,center={0,0}},density=1,friction=0.3,restitution=0.25}
	it.body.set{mass="shapes",x=x,y=y,a=a} -- calculate from shapes
	
	return it
end

	
	



function mouse(act,x,y,key)

	if act=="down" then

		table.insert(items, new_item((x-320)/10,(y-240)/10,0) )
	
	end

end


	
function clean()
	world.delete()
	items={}
end



function update()

	world.step(1/50,2)

end


function draw()

	gl.ClearColor(0,0,0.25,0)
	win.begin()
	
	win.project23d(480/640,1,1024)
	gl.MatrixMode("MODELVIEW")
	gl.LoadIdentity()

	for i,v in ipairs(items) do
	
		
		v.body.get()
		gl.PushMatrix()
		
		gl.Translate(v.body.x,v.body.y, -24)
		gl.Rotate(v.body.a,0,0,1);
		
		win.font_debug.set(v.tx,v.ty,v.tc,v.ts)
		win.font_debug.draw(v.text)
	
		gl.PopMatrix()
	end

end


