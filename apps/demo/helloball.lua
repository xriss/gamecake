
-- setup some default search paths
require("apps").default_paths()

-- grab some libs
local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")
local win=require("wetgenes.win").create({})

-- wrap some extra shader compiler functions around a basic gles2 library
local gl=require("glescode").create( assert(require("gles").gles2) )


local shaderprefix="#version 100\nprecision mediump float;\n"

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



function ball_create()
	local ball={}

	local t={} -- tempory table to build into
	function tp(...) for i,v in ipairs{...} do t[#t+1]=v end end -- and data push function
	for check=0,1 do
	for y=0,7 do
		for x=0,15 do
			if ((x+y)%2)==check then
				tp(	(x+0)/16	,	(y+0)/8		,	0	)
				tp(	(x+1)/16	,	(y+0)/8		,	0	)
				tp(	(x+0)/16	,	(y+1)/8		,	0	)
				
				tp(	(x+1)/16	,	(y+0)/8		,	0	)
				tp(	(x+0)/16	,	(y+1)/8		,	0	)
				tp(	(x+1)/16	,	(y+1)/8		,	0	)
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


local i=0
local ball=ball_create()
while true do
	
	if frame_time<win:time() then frame_time=win:time() end -- prevent race condition
	while frame_time>win:time() do win:sleep(0.001) end -- simple frame limit
	frame_time=frame_time+frame_rate -- step frame forward

	repeat -- handle msg queue (so we know the window size)
		local m={win:msg()}
	until not m[1]
	
	i=(i+1)%256


	win:info()
--	print(win.width,win.height)
	gl.Viewport(0,0,win.width,win.height)

	gl.ClearColor(i/255,i/255,i/255,i/255)
--	gl.ClearColor(0,0,0,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( tardis.m4_project23d(win.width,win.height,1920,1080,0.25,2160) )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()

	gl.PushMatrix()
--	gl.Translate(-1920/2,-1080/2,-1080*1)
	gl.Translate(0,0,-1080)
	
--[[
	gl.Disable(gl.LIGHTING)
	gl.Disable(gl.DEPTH_TEST)
	gl.Disable(gl.CULL_FACE)
	gl.Disable(gl.TEXTURE_2D)    
    
	gl.Color(1,1,1,1)
   	gl.EnableClientState(gl.VERTEX_ARRAY)
   	gl.DisableClientState(gl.TEXTURE_COORD_ARRAY)
   	gl.DisableClientState(gl.COLOR_ARRAY)
   	gl.DisableClientState(gl.NORMAL_ARRAY)
]]

	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)

	ball.draw(256,{1,1,1,1},0)
	ball.draw(256,{0,0,0,1},1)

--print(gl.GetError())

	gl.PopMatrix()

	win:swap()
end
