--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
local gcinfo=gcinfo

local hex=function(str) return tonumber(str,16) end
local dprint=function(a) print(require("wetgenes.string").dump(a)) end

local pack=require("wetgenes.pack")
local wzips=require("wetgenes.zips")

local wstr=require("wetgenes.string")

local tardis=require("wetgenes.tardis")	-- matrix/vector math

local win=require("wetgenes.win")

local XLT=require("wetgenes.tongues").translate


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

function M.bake(oven,escmenu)

	escmenu=escmenu or {}
	
	local opts={
		width=480,
		height=480,
	}

	local gl=oven.gl
	local cake=oven.cake
	local canvas=cake.canvas
	local layout=cake.layouts.create{}

	local skeys=oven.rebake("wetgenes.gamecake.spew.keys")


	function escmenu.setup()
	
		oven.cake.fonts.loads({4}) -- always load builtin font number 4 a basic 8x16 font		
		opts.font=4
		
--[[
		if wzips.exists("data/fonts/Vera.ttf") then -- we got us better font to use :)
			oven.cake.fonts.loads({"Vera"})
			opts.font="Vera"
		end
]]

		escmenu.show=false

		escmenu.master=oven.rebake("wetgenes.gamecake.widgets").setup({})
		
		local hooks={}
		function hooks.click(act,widget)
			local id=widget.id
			if id=="layout" then
				local mlayout=oven.mods["wetgenes.gamecake.mods.layout"]
				if mlayout then
					mlayout.cycle_mode()
					escmenu.layout_widget.text=XLT"Layout: "..mlayout.mode
				end
			elseif id=="continue" then
			
				escmenu.show=false
				
			elseif id=="restart" then
			
				oven.next=oven.now
				escmenu.show=false
				
			elseif id=="quit" then
			
				oven.next=true
				escmenu.show=false
				
			end
		end
		local top=escmenu.master:add({hx=480,hy=480,class="fill",font=opts.font,text_size=32})
		top:add({hx=480,hy=80})
		
		local mlayout=oven.mods["wetgenes.gamecake.mods.layout"]
		if mlayout then		
			escmenu.layout_widget=top:add({hx=480,hy=80,text=XLT"Layout: "..mlayout.mode,color=0xffcccccc,id="layout",hooks=hooks,text_size=32})
		else
			top:add({hx=480,hy=40})
		end
		
		top:add({class="button",hx=480,hy=80,text=XLT"Continue",color=0x7f00ff00,id="continue",hooks=hooks})
		top:add({class="button",hx=480,hy=80,text=XLT"Restart",color=0x7fffff00,id="restart",hooks=hooks})
		top:add({class="button",hx=480,hy=80,text=XLT"Quit",color=0x7fff0000,id="quit",hooks=hooks})
		if not mlayout then		
			top:add({hx=480,hy=40})
		end
		top:add({hx=480,hy=80})
		
		escmenu.master:layout()

	end

	function escmenu.clean()
	
	end
	
	

	function escmenu.update()


		if escmenu.show then

			escmenu.master:update()
		
		end
		
	end
	
	function escmenu.draw()
	
		local cake=oven.cake
--		local gl=cake.gl
--		local canvas=oven.canvas
		local font=canvas.font

		if escmenu.show then

		layout.viewport() -- did our window change?
		layout.project23d(opts.width,opts.height,1/4,opts.height*4)
		canvas.gl_default() -- reset gl state


		gl.MatrixMode(gl.PROJECTION)
		gl.LoadMatrix( layout.pmtx )

		gl.MatrixMode(gl.MODELVIEW)
		gl.LoadIdentity()
		gl.Translate(-opts.width/2,-opts.height/2,-opts.height*2) -- top left corner is origin
		gl.PushMatrix()

		escmenu.master:draw()
			
		gl.PopMatrix()

		end

		
	end
		
	function escmenu.msg(m)
--dprint(m)
		if escmenu.show then
			if m.class=="key" or m.class=="mouse" or m.class=="joykey" or m.class=="joystick" then

				if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys

				if m.xraw and m.yraw then	-- we need to fix raw x,y numbers
					m.x,m.y=layout.xyscale(m.xraw,m.yraw)	-- local coords, 0,0 is center of screen
					m.x=m.x+(opts.width/2)
					m.y=m.y+(opts.height/2)
				end

				if m.class=="key" and m.action==-1 and m.keyname=="escape" then
					escmenu.show=not escmenu.show
					return nil
				end
				
				if ( m.class=="joykey" and m.action==-1 and (m.keycode==4 or m.keycode==0) ) then -- back button
					escmenu.show=not escmenu.show
					return nil
				end

				escmenu.master:msg(m)

				return nil
			end
		else
			if ( m.class=="joykey" and m.action==-1 and m.keycode==0 ) then -- back button
				escmenu.show=not escmenu.show
				return nil
			end
		end
		if ( m.class=="key" and m.keyname=="escape" ) then
			if m.action==-1 then
				escmenu.show=not escmenu.show
			end
			return nil
		end
		return m
	end

	return escmenu
end
