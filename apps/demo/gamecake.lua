#!../../bin/dbg/lua

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math

local opts={}


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

		state.cake=require("wetgenes.gamecake").bake({
			width=640,
			height=480,
			grdprefix=apps.dir.."data/",
			grdpostfix=".png",
			sodprefix=apps.dir.."data/sfx_",
			sodpostfix=".wav",
			fontprefix=apps.dir.."data/font_",
			fontpostfix=".ttf",
			gl=state.gl,
			win=state.win,
			disable_sounds=true,
		})
	end
	
--	table.insert(state.mods,require("wetgenes.gamecake.mods.console").bake(opts))

	state.next=demo

	local finished=state.change()
	repeat

		repeat -- handle msg queue (so we know the window size on windows)
			local m={state.win:msg()}
		until not m[1]

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

demo.update=function(state)

	state.cake.screen.width=state.win.width
	state.cake.screen.height=state.win.height
	state.cake.screen:project23d(320,480,0.5,1024)

end

demo.draw=function(state)
--print("draw")
	local win=state.win
	local cake=state.cake
	local canvas=cake.canvas
	local gl=cake.gl
	
--print(wstr.dump(win))

	win:info()
	gl.Viewport(0,0,win.width,win.height)

	gl.ClearColor(0,0,0,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

	gl.MatrixMode(gl.PROJECTION)
	gl.LoadMatrix( tardis.m4_project23d(win.width,win.height,640,480,0.25,480*4) )

	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()
	gl.Translate(0,0,-480*2)


	gl.Disable(gl.LIGHTING)
	gl.Disable(gl.DEPTH_TEST)
	gl.Disable(gl.CULL_FACE)
	gl.Enable(gl.TEXTURE_2D)    
    
	gl.Color(1,1,1,1)	
   	gl.EnableClientState(gl.VERTEX_ARRAY)
   	gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
   	gl.DisableClientState(gl.COLOR_ARRAY)
   	gl.DisableClientState(gl.NORMAL_ARRAY)

--	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
	gl.BlendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA)
	gl.Enable(gl.BLEND)


	gl.PushMatrix()
	
	canvas:font_set(cake.fonts:get(1))
	canvas:font_set_size(32,0)
	canvas:font_set_xy(-120,-60)
	canvas:font_draw("Hello World!")
	
	gl.PopMatrix()


--	win:swap()

end




start()
