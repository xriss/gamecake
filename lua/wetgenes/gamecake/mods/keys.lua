--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,keys)

	local skeys=oven.rebake("wetgenes.gamecake.spew.keys") -- use spew keys always

	keys=keys or {}

	local win=oven.win
	local cake=oven.cake
	local canvas=cake.canvas
--	local layout=oven.rebake("wetgenes.gamecake.mods.layout").keys

-- push a keyboard widget into a master
	function keys.setup_keyboard_widgets(master)
		local shift=false
		local top
		local hooks=function(act,widget)
		if act=="click" then

--	print(widget.id)
			local ascii=widget.text
			local code=0
			local name=""
			
			if ascii=="<" then
				ascii=""
				code=0
				name="back"
			elseif ascii==">" then
				ascii=""
				code=0
				name="delete"
			elseif ascii=="^" then -- toggle caps
			
				shift=not shift
			
				for i,t in ipairs(top) do -- each row of keys
					for i,v in ipairs(t) do -- each key
					
						local otext=v.text
						local ntext
						if otext then
							if shift then
								ntext=string.upper(otext)
							else
								ntext=string.lower(otext)
							end
							if otext~=ntext then
								v.text=ntext
								v:set_dirty()
							end
						end
					end
				end
				
				return

			end
			
			local mstack=oven.win.msgstack
			mstack[#mstack+1]={
				time=os.time(),
				class="text",
				text=ascii,
				softkey=true,
			}
		end
		end

		master:clean_all()
		master.ids={}

		top=master:add({hx=320,hy=160,class="fill",fbo=true})

		local function key_line(ks)
			local t=top:add({hx=320,hy=32,class="fill"})
			for i=1,#ks do
				local k=ks:sub(i,i)
				local w=t:add({hx=320/10,hy=32,color=0,text=k,id="key",class="button",hooks=hooks})
				w.never_set_focus_edit=true
			end
		end
		
		key_line("1234567890")
		key_line("qwertyuiop")
		key_line("asdfghjkl ")
		key_line("^zxcvbnm:#")
		key_line("<    _,./>")
					
	end

	function keys.setup()
--print("setup")
--		skeys.setup(1)
	end
	function keys.clean()
--print("clean")
		skeys.clean()
	end
	function keys.msg(m)
--print("msg",m)
		skeys.msg(m)
		return m
	end
	function keys.update()
--print("update")
		skeys.update()
	end


--[[
	function keys.setup()
	

		keys.master=oven.rebake("wetgenes.gamecake.widgets").setup({font="Vera",text_size=24})
		
		keys.setup_keyboard_widgets(keys.master)
	
		keys.master:layout()

	end

	function keys.clean()
	
	end
	
	

	function keys.update()
	
		if layout.active then


			if keys.top.hx~=layout.w or keys.top.hy~=layout.h then -- change rez
				keys.top.hx=layout.w
				keys.top.hy=layout.h
				
				keys.master.text_size=math.floor(layout.h/6)
				keys.master:layout()
			end

			keys.master:update()
		
		end
		
	end
	
	function keys.draw()
	
		local win=oven.win
		local cake=oven.cake
		local gl=cake.gl
		local font=canvas.font

		if layout.active then

		layout.apply()
--		layout.viewport() -- did our window change?
--		layout.project23d(layout.w,layout.h,1/4,layout.h*4)
		canvas.gl_default() -- reset gl state

--		gl.MatrixMode(gl.PROJECTION)
--		gl.LoadMatrix( layout.pmtx )

--		gl.MatrixMode(gl.MODELVIEW)
--		gl.LoadIdentity()
--		gl.Translate(-layout.w/2,-layout.h/2,-layout.h*2) -- top left corner is origin
		gl.PushMatrix()


		keys.master:draw()
			
		gl.PopMatrix()

		end

		
	end
		
	function keys.msg(m)
	
		if layout.active then
			if m.xraw and m.yraw then	-- we need to fix raw x,y numbers
				m.x,m.y=canvas.xyscale(m.xraw,m.yraw)	-- local coords, 0,0 is center of screen
				m.x=m.x+(layout.w/2)
				m.y=m.y+(layout.h/2)
			end
			keys.master:msg(m)
		end
		return m
		
	end
]]

	return keys
end
