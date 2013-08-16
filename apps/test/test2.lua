#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wxox=require("wetgenes.xox")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local startbake

local opts={
	times=true, -- request simple time keeping samples
	
	width=640,	-- display basics
	height=480,
	title="testing",
	fps=60,

}

opts.start=function(oven,demo)

	demo.loads=function()

		oven.cake.fonts.loads({1}) -- load 1st builtin font, a basic 8x8 font

	end
			
	demo.setup=function()

		demo.loads(oven)


		demo.xox=wxox.create_xox({}):load_dae("cog.dae")
		
	end


	demo.clean=function()

	end

	demo.msg=function(m)
	end

	demo.update=function()
	end

	local ii=0
	demo.draw=function()

		ii=ii+1
		
	--print("draw")
		local cake=oven.cake
		local canvas=cake.canvas
		local font=canvas.font
		local flat=canvas.flat
		local gl=oven.gl
		
		canvas.viewport() -- did our window change?
		canvas.project23d(opts.width,opts.height,0.25,opts.height*4)
		canvas.gl_default() -- reset gl state
			
		gl.ClearColor(0,0,0,0)
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( canvas.pmtx )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		gl.Translate(-opts.width/2,-opts.height/2,-opts.height*2) -- top left corner is origin

		gl.PushMatrix()
		

		gl.Color(pack.argb4_pmf4(0x800f))
		flat.quad(0,(opts.height/2)-32,opts.width,(opts.height/2)+32) -- draw a blue box as a background

		font.set(cake.fonts.get(1)) -- default font
		font.set_size(32,0) -- 32 pixels high
		
		local s="Hello World!"
		local sw=font.width(s) -- how wide the string is
		local x,y=(opts.width-(sw))/2,(opts.height-32)/2 -- center the text in  middle of display

		gl.Color(pack.argb4_pmf4(0xf000)) -- draw drop shadow
		font.set_xy(x+4,y+4)
		font.draw(s)
		
		gl.Color(pack.argb4_pmf4(0xffff)) -- draw white text
		font.set_xy(x,y)
		font.draw(s)


		gl.Color(pack.argb4_pmf4(0xffff)) -- draw white text
		canvas.flat.tristrip("xyzrgba",{
			100,100,0, 1,0,0,1,
			200,100,0, 0,1,0,1,
			100,200,0, 0,0,1,1,
			})
		
		gl.Translate(200,200,0)
	--	gl.Scale(300,300,300)
		gl.Scale(100,100,100)
		gl.Rotate(ii,1,1,0)
	--	gl.Rotate(ii,1,0,0)
		
		gl.Enable(gl.DEPTH_TEST)
	--	gl.Enable(gl.CULL_FACE)

	--	demo.draw_test(state)
		demo.xox:draw_canvas(canvas)

		gl.Disable(gl.DEPTH_TEST)
	--	gl.Disable(gl.CULL_FACE)
		
		gl.PopMatrix()
		
	end

	return demo
end

-- this will busy loop or hand back control depending on system we are running on
return require("wetgenes.gamecake.oven").bake(opts).preheat():serv()



