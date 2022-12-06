
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
0 0 0 0 0 0 0 0 
0 0 0 0 R 0 0 0 
0 0 O O O 0 0 0 
0 0 0 0 Y 0 0 0 
0 0 0 0 Y 0 0 0 
0 0 0 0 O 0 0 0 
0 0 0 0 R 0 0 0 
0 0 0 0 0 0 0 0 
]]},

}

local setup_done=false

update=function()

	if not setup_done then setup_done=true
	
		system.components.copper.shader_uniforms.cy0={gl.C8(0xff000000)} -- top
		system.components.copper.shader_uniforms.cy1={gl.C8(0xff000040)} -- middle
		system.components.copper.shader_uniforms.cy2={gl.C8(0xff000000)} -- bottom
			
	end

	system.components.text.text_print("Hello World!",5,2,31,0) -- (text,x,y,color,background)

	system.components.sprites.list_add({t=0x0101,h=8,
		s=2+math.abs(((system.ticks%120)/60)-1),
		px=160,py=120,rz=system.ticks/math.pi})
	
end


