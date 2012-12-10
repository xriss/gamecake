-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(state,keys)

	keys=keys or {}
	

	local canvas=state.canvas.child()
	canvas.layout=state.rebake("wetgenes.gamecake.mods.layout").keys

	function keys.setup()
	

		keys.master=state.rebake("wetgenes.gamecake.widgets").setup({font="Vera",text_size=24})
		
	
		local hooks={}
		function hooks.click(widget)
--	print(widget.id)
			local ascii=widget.text
			local code=0
			local name=""
			
			if ascii=="<" then
				ascii=""
				code=0
				name="backspace"
			elseif ascii==">" then
				ascii=""
				code=0
				name="delete"
			end
			
			local mstack=state.win.msgstack
			mstack[#mstack+1]={
				time=os.time(),
				class="key",
				action=1,
				ascii=ascii,
				keycode=code,
				keyname=name,
			}
			mstack[#mstack+1]={
				time=os.time(),
				class="key",
				action=-1,
				ascii=ascii,
				keycode=code,
				keyname=name,
			}
			
		end

--		keys.master=state.rebake("wetgenes.gamecake.widgets").setup({hx=320,hy=160})

		keys.master:clean_all()
		keys.master.ids={}

		keys.top=keys.master:add({py=0,hx=320,hy=160,mx=320,my=240,class="flow",ax=0,ay=0})

		local function key_line(ks)
			local t=keys.top:add({sx=320,sy=32,mx=320,my=32,class="flow"})
			for i=1,#ks do
				local k=ks:sub(i,i)
				t:add({sx=320/11,sy=32,color=0xffcccccc,text=k,id="key",hooks=hooks})
			end
		end
		
		key_line("1234567890")
		key_line("qwertyuiop")
		key_line("asdfghjkl ")
		key_line(" zxcvbnm:#")
		key_line("<     ,./>")
		
			
		keys.master:layout()

	end

	function keys.clean()
	
	end
	
	

	function keys.update()
	
		if canvas.layout.active then


			if keys.top.hx~=canvas.layout.w or keys.top.hy~=canvas.layout.h then -- change rez
				keys.top.hx=canvas.layout.w
				keys.top.hy=canvas.layout.h
				
				keys.master.text_size=math.floor(canvas.layout.h/6)
				keys.master:layout()
			end

			keys.master:update()
		
		end
		
	end
	
	function keys.draw()
	
		local win=state.win
		local cake=state.cake
		local gl=cake.gl
		local font=canvas.font

		if canvas.layout.active then

		canvas.viewport() -- did our window change?
		canvas.project23d(canvas.layout.w,canvas.layout.h,1/4,canvas.layout.h*4)
		canvas.gl_default() -- reset gl state

		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( canvas.pmtx )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		gl.Translate(-canvas.layout.w/2,-canvas.layout.h/2,-canvas.layout.h*2) -- top left corner is origin
		gl.PushMatrix()


		keys.master:draw()
			
		gl.PopMatrix()

		end

		
	end
		
	function keys.msg(m)
		if canvas.layout.active then
			if m.xraw and m.yraw then	-- we need to fix raw x,y numbers
				m.x,m.y=canvas.xyscale(m.xraw,m.yraw)	-- local coords, 0,0 is center of screen
				m.x=m.x+(canvas.layout.w/2)
				m.y=m.y+(canvas.layout.h/2)
			end
			keys.master:msg(m)
		end
	end

	return keys
end
