
hardware,main=system.configurator({
	mode="fun64",
	graphics=function() return graphics end,
	update=function() update() end,
})

graphics={

{0x0100,"char_empty",[[
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
. . . . . . . . 
]]},

{0x0101,"char_test",[[
. 0 0 0 0 0 0 . 
0 0 0 0 R 0 0 0 
0 0 O O O 0 0 0 
0 0 0 0 Y 0 0 0 
0 0 0 0 Y 0 0 0 
0 0 0 0 O 0 0 0 
0 0 0 0 R 0 0 0 
. 0 0 0 0 0 0 . 
]]},

}

-- an amiga ball

local function ball_create()
	local ball={}

	local p={} -- temp table to build points into
	function pp(...) for i,v in ipairs{...} do p[#p+1]=v end end -- and data push function
	function xy(x,y) return p[((x+(y*17))*3)+1],p[((x+(y*17))*3)+2],p[((x+(y*17))*3)+3] end
	for y=0,8 do
		for x=0,16 do
			local s=math.cos(math.pi*(y-4)/8)
			pp(	math.sin(2*math.pi*x/16)*s	,	math.sin(math.pi*(y-4)/8)	,	math.cos(2*math.pi*x/16)*s	)
		end
	end
	

	ball.vdats={}
	local t={} 														-- temp table to build tris into
	function tp(...) for i,v in ipairs{...} do t[#t+1]=v end end 	-- and data push function
	for check=0,1 do -- make chequered pattern order
		if not ball.vdats[check+1] then
			t={}
			ball.vdats[check+1]=t
		end
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
	

	function ball.draw(color,part)
		gl.Color(unpack(color))
		oven.cake.canvas.flat.tristrip("xyz",ball.vdats[part])
	end

	return ball
end


local setup_done=false
local ball

update=function()

	if not setup_done then setup_done=true
	
		ball=ball_create()
	
		system.components.copper.shader_uniforms.cy0={gl.C8(0xff000000)} -- top
		system.components.copper.shader_uniforms.cy1={gl.C8(0xff000040)} -- middle
		system.components.copper.shader_uniforms.cy2={gl.C8(0xff000000)} -- bottom

-- add a some custom gl rendering to copper background layer
		system.components.screen.layers[1].hooks[1]=function(layer)

			gl.PushMatrix()
			gl.Translate(160,120,0)
			gl.Rotate(system.ticks%360,1,1,1)
			gl.Scale(64,64,64)
			ball.draw({gl.C4(0xff00)},1)
			ball.draw({gl.C4(0xffff)},2)
			gl.PopMatrix()
			
		end
			
	end

	system.components.text.text_print("Hello World!",5,2,31,0) -- (text,x,y,color,background)

	system.components.sprites.list_add({t=0x0101,h=8,
		s=2+math.abs(((system.ticks%120)/60)-1),
		px=160,py=120,rz=system.ticks/math.pi})
	
end


