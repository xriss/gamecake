

local _G=_G

local win=win

local table=table
local ipairs=ipairs

local apps=apps

local gl=require("gl")

local box2d=require("box2d.wrap")
local grd=require("grd")


local function print(...) _G.print(...) end


module("state.test")


local items={}
local world	
local ground

local texs={}
	
	
	
function setup()

	world=box2d.world({gravity={0,-10}})

	ground = world.body{}
	ground.shape{box={width=64,height=10},density=1,friction=0.3}
	ground.set{x=0,y=-24-10,a=0}


		
	table.insert(items, new_item(0,10,0) )
	

local fname=apps.dir.."data/skins/test/button_high.png"



print(fname)
local g=grd.create("GRD_FMT_U8_BGRA",fname)
print(g)
local t=win.tex(g)
print(t)

	texs[1]=t

local fname=apps.dir.."data/skins/test/button_high_in.png"

print(fname)
local g=grd.create("GRD_FMT_U8_BGRA",fname)
print(g)
local t=win.tex(g)
print(t)

	texs[2]=t
	
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

	
	

mdown=false

function mouse(act,x,y,key)

	x,y=win.mouse23d(640,480,x,y)

	if act=="down" then
	
		mdown=true

		table.insert(items, new_item((x)/10,(y)/10,0) )
	
	elseif act=="up" then
	
		mdown=false
		
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

	win.begin()
	gl.ClearColor(.8,.8,.8,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	win.project23d(480/640,2,1024)
	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()

	for i,v in ipairs(items) do
	
		
		v.body.get()
		gl.PushMatrix()
		
		gl.Translate(v.body.x,v.body.y, -48)
		gl.Rotate(v.body.a,0,0,1);
		
		win.font_debug.set(v.tx,v.ty,v.tc,v.ts)
		win.font_debug.draw(v.text)
	
		gl.PopMatrix()
	end
	
	
	gl.PushMatrix()
	
		gl.Translate(0,0, -480)
		
		local tex=texs[1]
				
		if mdown then tex=texs[2] end
		
		tex:bind()
		
		local tw,th=tex.width,tex.height
		local vw,vh=512,52
		local mw,mh=24,24
		
		local tww={mw/tw,(tw-2*mw)/tw,mw/tw}
		local thh={mh/th,(th-2*mh)/th,mh/th}
		local vww={mw,vw-2*mw,mw}
		local vhh={mh,vh-2*mh,mh}
		
		gl.Disable(gl.LIGHTING)
		gl.Disable(gl.CULL_FACE)
		gl.Enable(gl.TEXTURE_2D)


		gl.TexParameter(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR)
		gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)


		gl.Begin(gl.QUADS)
			gl.Color({1,1,1,1})
			
			local function drawbox( tx,ty, vx,vy , txp,typ, vxp,vyp )
				gl.TexCoord(tx    ,ty)     gl.Vertex(vx,    vy)
				gl.TexCoord(tx+txp,ty)     gl.Vertex(vx+vxp,vy)
				gl.TexCoord(tx+txp,ty+typ) gl.Vertex(vx+vxp,vy+vyp)
				gl.TexCoord(tx    ,ty+typ) gl.Vertex(vx,    vy+vyp)
			end
			
		local tx,ty=0,0
		local vx,vy=-vw/2,vh/2

			for iy=1,3 do
				tx=0
				vx=-vw/2
				for ix=1,3 do

					drawbox( tx,ty, vx,vy , tww[ix],thh[iy], vww[ix],-vhh[iy] )

					tx=tx+tww[ix]
					vx=vx+vww[ix]
				end
				ty=ty+thh[iy]
				vy=vy-vhh[iy]
			end
			
		gl.End()
		
	gl.PopMatrix()

end


