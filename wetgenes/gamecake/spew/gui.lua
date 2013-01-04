-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local function print(...) _G.print(...) end

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(state,gui)

	gui=gui or {} 
	gui.modname=M.modname

	gui.pages={} -- put functions to fill in pages in here

	local cake=state.cake
	local sounds=cake.sounds


	function gui.setup(parent)
		gui.parent=parent
		gui.master=parent.master
--		gui.page()
	end
	
	local wdata=state.rebake("wetgenes.gamecake.widgets.data")
	local mkeys=state.rebake("wetgenes.gamecake.mods.keys")
	
--	local bpages=state.rebake("bulb.pages")
	
	gui.ids={}
	gui.data={}
	gui.data.mode=1
	
	function gui.set_all_values()
		local ids=gui.master.ids
		local d=gui.data
--		if ids.menu_gamemode then ids.menu_gamemode.text=d.modes[d.mode].title end
	end
	
	function gui.hooks(act,widget)
		local ids=gui.master.ids
		local d=gui.data
		local id=widget and widget.id
		

		if act=="click" then
print("click",id)
			if id=="profiles_select" then
				gui.page("profile")
			end
		end
	
	end

	

	function gui.pages.profiles(master)

		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		top:add({sx=320,sy=200})

		top:add({sx=20,sy=80})
		top:add({sx=320,sy=80,color=0xffcccccc,text="1: abcdef",id="profiles_select",hooks=gui.hooks,user=1})
		top:add({sx=20,sy=80})
		top:add({sx=320,sy=80,color=0xffcccccc,text="2: abcdef",id="profiles_select",hooks=gui.hooks,user=2})
		top:add({sx=20,sy=80})
		top:add({sx=320,sy=80,color=0xffcccccc,text="3: abcdef",id="profiles_select",hooks=gui.hooks,user=3})
		top:add({sx=20,sy=80})
		top:add({sx=320,sy=80,color=0xffcccccc,text="4: abcdef",id="profiles_select",hooks=gui.hooks,user=4})
		top:add({sx=20,sy=80})
		top:add({sx=320,sy=80,color=0xffcccccc,text="5: abcdef",id="profiles_select",hooks=gui.hooks,user=5})
		top:add({sx=20,sy=80})

		top:add({sx=320,sy=200})
		
	end
	
	function gui.pages.profile(master)
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		top:add({sx=320,sy=80})
		top:add({sx=80,sy=80,color=0xffcccccc,text="Name"})
		top:add({sx=240,sy=80,color=0xffcccccc,text="abcdef",id="profile_name",hooks=gui.hooks,class="textedit"})

		top:add({sx=320,sy=80,color=0xffcccccc,text="Random?",id="profile_name_rand",hooks=gui.hooks})
		top:add({sx=320,sy=80})

		local m=top:add({sx=320,sy=200})
		
		mkeys.setup_keyboard_widgets(m)
	end


	function gui.page(pname)
	
		if not gui.master then
			gui.master=state.rebake("wetgenes.gamecake.widgets").setup({hx=320,hy=480})
		end
	
		gui.master:clean_all()
		gui.master.ids={}
		
		if pname then
			local f=gui.pages[pname]
			if f then
				f(gui.master) -- pass in the master so we could fill up other widgets
			end
		end

		gui.set_all_values()

		gui.master:layout()
		
	end

	function gui.clean()

		gui.master=nil
	
	end
	
	function gui.update()
	
		gui.master:update()

	end
	
	function gui.msg(m)

		gui.master:msg(m)

	end

	function gui.draw()
	
		gui.master:draw()

	end
	
	return gui
end
