#!../../bin/dbg/lua

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local wwin=require("wetgenes.win")
local wstr=require("wetgenes.string")

local opts={}


function start()
	local state=require("wetgenes.gamecake.state").bake(opts)

	do
		local screen=wwin.screen()
		local inf={width=640,height=480,title="testing"}
		inf.x=(screen.width-inf.width)/2
		inf.y=(screen.height-inf.height)/2
		state.win=wwin.create(inf)
		state.win:context()

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
			gl=require("gles").gles1,
			disable_sounds=true,
		})
	end
	
	table.insert(state.mods,require("wetgenes.gamecake.mods.console").bake(opts))

	state.next=runthis

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

runthis={}


start()
