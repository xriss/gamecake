#!../../bin/dbg/lua

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local opts={
	times=true, -- request simple time keeping samples
}


function start()
	local state=require("wetgenes.gamecake.state").bake(opts)

	do
		local screen=wwin.screen()
		local inf={width=640,height=480,title="testing"}
		inf.x=(screen.width-inf.width)/2
		inf.y=(screen.height-inf.height)/2
		state.win=wwin.create(inf)
		state.gl=require("gles").gles1
		state.win:context({})

		state.frame_rate=1/60 -- how fast we want to run
		state.frame_time=0

		require("wetgenes.gamecake").bake({
			state=state,
			width=inf.width,
			height=inf.height,
		})
		
	end
	
	state.require_mod("wetgenes.gamecake.mods.escmenu") -- escmenu gives us a doom style esc menu
	state.require_mod("wetgenes.gamecake.mods.console") -- console gives us a quake style tilda console

	state.next=demo -- we want to run a demo state

	local finished
	repeat

		state.msgs()
		state.update()
		state.draw()
		
		finished=state.change()
		
	until finished
end

demo={}

demo.loads=function(state)

	state.cake.fonts:loads({1}) -- load builtin font number 1 a basic 8x8 font

end
		
demo.setup=function(state)

	demo.loads(state)
	
end

demo.clean=function(state)

end

demo.msg=function(state,m)
end

demo.update=function(state)
end

demo.draw=function(state)
--print("draw")
	local win=state.win
	local cake=state.cake
	local canvas=state.canvas
	local font=canvas.font
	local gl=cake.gl
	
	canvas:viewport() -- did our window change?
	canvas:project23d(640,480,0.25,480*4)
	canvas:gl_default() -- reset gl state
		
	gl.ClearColor(0,0,0,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( canvas.pmtx )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()
	gl.Translate(-320,-240,-480*2) -- a good starting point

	gl.PushMatrix()
	
	font:set(cake.fonts:get(1))
	font:set_size(32,0)
	font:set_xy((640-(12*32))/2,240-16)
	font:draw("Hello World!")
	
	gl.PopMatrix()
	
end




start()
