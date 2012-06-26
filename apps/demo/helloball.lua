
-- setup some default search paths
require("apps").default_paths()

-- grab some libs
local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")

-- a window/screen handler
local win=require("wetgenes.win").create({})

-- wrap some extra shader compiler functions around a basic gles2 library
local gl=require("glescode").create( assert(require("gles").gles2) )

-- gles needs this header
local shaderprefix="#version 100\nprecision mediump float;\n"

-- source code for a simple a shader, the name= are just for debuging
local prog_color=
{
	name="prog_color",
	vshaders=
	{{
		name="vtx_color",
		source=shaderprefix..[[

uniform mat4 modelview;
uniform mat4 projection;

attribute vec3 vertex;
uniform vec4 color;

varying vec4  v_color;
 
void main()
{
    gl_Position = projection * modelview * vec4(vertex, 1.0);
	v_color=color;
}

]]
	}},
	fshaders=
	{{
		name="frg_color",
		source=shaderprefix..[[

varying vec4  v_color;

void main(void)
{
	gl_FragColor=v_color ;
}

	]]
	}},
}


-- an amiga ball

function ball_create()
	local ball={}

	local p={} -- tempory table to build points into
	function pp(...) for i,v in ipairs{...} do p[#p+1]=v end end -- and data push function
	function xy(x,y) return p[((x+(y*17))*3)+1],p[((x+(y*17))*3)+2],p[((x+(y*17))*3)+3] end
	for y=0,8 do
		for x=0,16 do
			local s=math.cos(math.pi*(y-4)/8)
			pp(	math.sin(2*math.pi*x/16)*s	,	math.sin(math.pi*(y-4)/8)	,	math.cos(2*math.pi*x/16)*s	)
		end
	end
	

	local t={} -- tempory table to build tris into
	function tp(...) for i,v in ipairs{...} do t[#t+1]=v end end -- and data push function
	for check=0,1 do -- make chequered pattern order
		for y=0,7 do
			for x=0,15 do
				if ((x+y)%2)==check then
					tp(	xy(x+0,y+0) )
					tp(	xy(x+1,y+0) )
					tp(	xy(x+0,y+1) )
					
					tp(	xy(x+1,y+0) )
					tp(	xy(x+1,y+1) )
					tp(	xy(x+0,y+1) )
				end
				
			end
		end
	end

	ball.vdat=pack.alloc( #t*4 )
	pack.save_array(t,"f32",0,#t,ball.vdat)
	

	ball.vbuf=gl.GenBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER,ball.vbuf)
	gl.BufferData(gl.ARRAY_BUFFER,#t*4,ball.vdat,gl.STATIC_DRAW)

--	print(wstr.dump(t))
	function ball.draw(siz,color,part)
	
		local p=gl.program(prog_color)
		gl.UseProgram( p[0] )

		gl.PushMatrix()
		gl.Scale(siz,siz,siz)

		gl.BindBuffer(gl.ARRAY_BUFFER,ball.vbuf)

		gl.VertexAttribPointer(p:attrib("vertex"), 3 , gl.FLOAT , gl.FALSE , 3*4 , 0*4)
		gl.EnableVertexAttribArray(p:attrib("vertex"))

		gl.UniformMatrix4f(p:uniform("modelview"), gl.matrix(gl.MODELVIEW) )
		gl.UniformMatrix4f(p:uniform("projection"), gl.matrix(gl.PROJECTION) )

		gl.Uniform4f(p:uniform("color"), color[1],color[2],color[3],color[4] )

		if part==0 then
			gl.DrawArrays(gl.TRIANGLES,0,(#t/3)/2)
		elseif part==1 then
			gl.DrawArrays(gl.TRIANGLES,(#t/3)/2,(#t/3)/2)
		elseif part==2 then
			gl.DrawArrays(gl.TRIANGLES,0,(#t/3))
		end
		
		gl.PopMatrix()
	end

	return ball
end


win:context({})

local frame_rate=1/60
local frame_time=0

local ball=ball_create()

-- lets bounce
local siz=256
local pos={0,0,0}
local vec={4,4,0}
local rot={0,0,0}
local vrt={1,1,1/2}

while true do
	
-- frame limit

	if frame_time<win:time() then frame_time=win:time() end -- prevent race condition
	while frame_time>win:time() do win:sleep(0.001) end -- simple frame limit
	frame_time=frame_time+frame_rate -- step frame forward

	repeat -- handle msg queue (so we know the window size)
		local m={win:msg()}
	until not m[1]

-- update bouncyness

	rot[1]=rot[1]+vrt[1]
	rot[2]=rot[2]+vrt[2]
	rot[3]=rot[3]+vrt[3]
	
	pos[1]=pos[1]+vec[1]
	pos[2]=pos[2]+vec[2]
	
	vec[2]=vec[2] + (1/8)
	
	if pos[1] > -siz+1920/2 then pos[1]=-siz+1920/2 vec[1]=vec[1]*-1 vrt[1]=vrt[1]*-1 end
	if pos[1] <  siz-1920/2 then pos[1]= siz-1920/2 vec[1]=vec[1]*-1 vrt[1]=vrt[1]*-1 end
	
	if pos[2] > -siz+1080/2 then pos[2]=-siz+1080/2 vec[2]=vec[2]*-1 vrt[2]=vrt[2]*-1 end
	if pos[2] <  siz-1080/2 then pos[2]= siz-1080/2 vec[2]=0         vrt[2]=vrt[2]*-1 end

-- prepare to draw a frame

	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)
	gl.Enable(gl.DEPTH_TEST)

	win:info()
	gl.Viewport(0,0,win.width,win.height)

	gl.ClearColor(0,0,1/4,1/4)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( tardis.m4_project23d(win.width,win.height,1920,1080,0.25,1080*4) )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()

	gl.PushMatrix()

-- draw the ball

	gl.Translate(0,0,-1080*2)
	gl.Translate(pos[1],pos[2],pos[3])
	
	gl.Rotate(rot[2],1,0,0)
	gl.Rotate(rot[1],0,0,1)
	
	ball.draw(siz,{1,1,1,1},0) -- draw white parts
	ball.draw(siz,{1,0,0,1},1) -- draw red parts

--print(gl.GetError())

	gl.PopMatrix()

	win:swap()
end
