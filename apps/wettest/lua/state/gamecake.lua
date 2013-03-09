-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local win=win
local gl=require("gl")

local function print(...) _G.print(...) end

local wstr=require("wetgenes.string")

module("state.gamecake")


local cake
	
	
function setup()

	cake=require("wetgenes.gamecake").create({width=320,height=240})
	
print(wstr.dump(cake.canvas))

	for y=0,239 do
		for x=0,319 do
			cake.canvas.grd:pixels(x,y,1,1,{255,x%256,y%256,(x*4)%256})
		end
	end
	
--print(wstr.dump(			cake.canvas.grd:pixels(16,32,1,1) ) )

	
end



function mouse(act,x,y,key)

	if act=="down" then

--		table.insert(items, new_item((x-320)/10,(y-240)/10,0) )
	
	end

end


	
function clean()
end



function update()

end


function draw()

	win.begin()
	gl.ClearColor(0,0,0.25,0)
	gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT);
	
	win.project23d(640/480,1,1024)
	gl.MatrixMode(gl.MODELVIEW)
	gl.LoadIdentity()

		
	gl.PushMatrix()
	
	gl.Translate(0,0,-120)
--	gl.Rotate(v.body.a,0,0,1);
	
	cake:draw()

	win.font_debug.set(16,12,0xffffffff,8)
	win.font_debug.draw("testing")


	gl.PopMatrix()

end


