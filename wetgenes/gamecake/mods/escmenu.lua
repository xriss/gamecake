-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


module("wetgenes.gamecake.mods.escmenu")

name="escmenu" -- by this name shall ye know me

function bake(opts)

	local escmenu={}
	

	function escmenu.setup(state)
	
		state.cake.fonts:loads({1}) -- load builtin font number 1 a basic 8x8 font

		escmenu.show=false

		escmenu.master=require("wetgenes.gamecake.widget").setup(state.win,{state=state})

		local hooks={}
		function hooks.click(widget)
	print(widget.id)
		end
		local top=escmenu.master:add({hx=640,hy=480,mx=1,class="hx",ax=0,ay=0})
		top:add({sy=3,sx=1})
		top:add({text="Continue",color=0x8800ff00,id="continue",hooks=hooks})
		top:add({text="Main Menu",color=0x88ffff00,id="menu",hooks=hooks})
		top:add({text="Reload",color=0x88ff8800,id="reload",hooks=hooks})
		top:add({text="Quit",color=0x88ff0000,id="quit",hooks=hooks})
		top:add({sy=3,sx=1})
		
		escmenu.master:layout()

	end

	function escmenu.clean(state)
	
	end
	
	

	function escmenu.update(state)
	
		if escmenu.show then

			escmenu.master:update()
		
		end
		
	end
	
	function escmenu.draw(state)
	
		if escmenu.show then
		
			escmenu.master:draw()
			
		end
		
	end
		
	function escmenu.msg(state,m)
		if escmenu.show then
			escmenu.master:msg(m)
		end
		if m[1]=="key" then
			if m[5]=="escape" and m[3]==1 then -- escape key toggles
				escmenu.show=not escmenu.show
			end
		end
	end

	return escmenu
end
