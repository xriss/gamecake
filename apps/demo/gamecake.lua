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

-- we want the following mods to load and be setup
-- console gives us a ` style quake console
	state.require_mod("wetgenes.gamecake.mods.console")


	state.escmenu=require("wetgenes.gamecake.widget").setup(state.win,{state=state--[[font=win.font_sans]]})


	local hooks={}
	function hooks.click(widget)
print(widget.id)
	end
	local top=state.escmenu:add({hx=640,hy=480,mx=1,class="hx",ax=0,ay=0})
	top:add({sy=4,sx=1})
	top:add({text="Continue",color=0xff00ff00,id="continue",hooks=hooks})
	top:add({text="Main Menu",color=0xffffff00,id="menu",hooks=hooks})
	top:add({text="Reload",color=0xffff8800,id="reload",hooks=hooks})
	top:add({text="Quit",color=0xffff0000,id="quit",hooks=hooks})
	top:add({sy=4,sx=1})
	
	state.escmenu:layout()

	
end

demo.clean=function(state)

end

demo.update=function(state)

	state.cake.screen.width=state.win.width
	state.cake.screen.height=state.win.height
	state.cake.screen:project23d(320,480,0.5,1024)

	state.escmenu:update()
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
	gl.Translate(-320,-240,-480*2) -- a good starting point

	canvas:gl_default()

	gl.PushMatrix()
	
	canvas:font_set(cake.fonts:get(1))
	canvas:font_set_size(32,0)
	canvas:font_set_xy((640-(12*32))/2,240-16)
	canvas:font_draw("Hello World!")
	
	gl.PopMatrix()


	state.escmenu:draw()
	
end




start()
