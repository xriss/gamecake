-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


local function print(...) _G.print(...) end

local wstr=require("wetgenes.string")
local pack=require("wetgenes.pack")

local snames=require("wetgenes.gamecake.spew.names")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


M.bake=function(oven,gui)

	gui=gui or {} 
	gui.modname=M.modname

	gui.pages={} -- put functions to fill in pages in here

	local cake=oven.cake
	local sounds=cake.sounds


	function gui.setup(parent)
		gui.parent=parent
		gui.master=parent.master
--		gui.page()
	end
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local mkeys=oven.rebake("wetgenes.gamecake.mods.keys")
	local sprofiles=oven.rebake("wetgenes.gamecake.spew.profiles")
	
--	local bpages=oven.rebake("bulb.pages")
	
	gui.ids={}
	gui.data={}
	function gui.initdata() -- call this later
		gui.data.mode=1
		gui.data.score=0
		gui.data.name=wdata.new_data({class="string",hooks=gui.hooks})
	end
	
	function gui.set_all_values()
		local ids=gui.master.ids
		local d=gui.data
--		if ids.menu_gamemode then ids.menu_gamemode.text=d.modes[d.mode].title end
	end
	
	function gui.hooks(act,widget)
		if not gui.master then return end
		local ids=gui.master.ids
		local d=gui.data
		local id=widget and widget.id
		

		if act=="click" then
print("click",id)
			if id=="profiles_select" then
				sprofiles.select(widget.user)
				gui.page("profile")
				
			elseif id=="profile_name_clear" then

				gui.data.name:value( "" )
				
			elseif id=="profile_name_rand" then

				gui.data.name:value( snames.random() )

			elseif id=="profile_name_edit" then
			
				gui.page("profile_name_edit")
				
			elseif id=="profile_name_set" then
			
				sprofiles.set("name",gui.data.name:value())
				gui.page("profile")

			elseif id=="profile_return" then
			
				if gui.returnpage then gui.returnpage() end -- callback to return to original menu			
					
			elseif id=="profile_goto" then

				gui.page(widget.user)
			
			elseif id=="profile_cancel" then
			
					if widget.user then
						gui.page(widget.user)
					else
						if gui.returnpage then gui.returnpage() end -- callback to return to original menu			
					end

			end
		end
	
	end

	
	function gui.add_part(top,name)
--[[
		if name=="profile_bar" then
			local bback
			if gui.page_name=="profile" then bback="profiles" end
			top:add({sx=110,sy=40,color=0xffcccccc,text="OK",id="profile_ok",hooks=gui.hooks,user=bback})
			top:add({sx=100,sy=40})
			top:add({sx=110,sy=40,color=0xffcccccc,text="Cancel",id="profile_cancel",hooks=gui.hooks,user=bback})
		end
]]
	end


	function gui.pages.profiles(master)

		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		top:add({sx=320,sy=110,text="Choose profile.",text_color=0xffffffff})

		for i,v in sprofiles.ipairs() do
			top:add({sx=320,sy=20})
			top:add({sx=20,sy=50})
			top:add({sx=280,sy=50,color=0xffcccccc,text=v.name,id="profiles_select",hooks=gui.hooks,user=i})
			top:add({sx=20,sy=50})
		end
		top:add({sx=320,sy=20})

	end
		
	function gui.pages.profile(master)
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})
		
		gui.data.name:value( sprofiles.get("name") )

		top:add({sx=110,sy=40,color=0xffcccccc,text="OK",id="profile_return",hooks=gui.hooks})
		top:add({sx=100,sy=40})
		top:add({sx=110,sy=40,color=0xffcccccc,text="Cancel",id="profile_goto",hooks=gui.hooks,user="profiles"})

		top:add({sx=320,sy=20})
		top:add({sx=320,sy=40,text_color=0xffffffff,text="My name is"})
		top:add({sx=320,sy=20})

		top:add({sx=20,sy=40})
		top:add({sx=280,sy=40,color=0xffcccccc,text=gui.data.name:value(),id="profile_name_edit",hooks=gui.hooks})
		top:add({sx=20,sy=40})

		top:add({sx=320,sy=40*8})

	end

	function gui.pages.profile_name_edit(master)
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})
		
		gui.data.name:value( sprofiles.get("name") )

		top:add({sx=110,sy=40,color=0xffcccccc,text="OK",id="profile_name_set",hooks=gui.hooks})
		top:add({sx=100,sy=40})
		top:add({sx=110,sy=40,color=0xffcccccc,text="Cancel",id="profile_goto",hooks=gui.hooks,user="profile"})

		top:add({sx=320,sy=80,text_color=0xffffffff,text="Type your name"})

		top:add({sx=20,sy=40})
		top:add({sx=280,sy=40,color=0xffcccccc,data=gui.data.name,id="profile_name",hooks=gui.hooks,class="textedit"})
		top:add({sx=20,sy=40})

		top:add({sx=20,sy=40})
		top:add({sx=120,sy=40,color=0xffcccccc,text="Clear",id="profile_name_clear",hooks=gui.hooks})
		top:add({sx=40,sy=40})
		top:add({sx=120,sy=40,color=0xffcccccc,text="Random",id="profile_name_rand",hooks=gui.hooks})
		top:add({sx=20,sy=40})
		
		top:add({sx=320,sy=40})
		top:add({sx=320,sy=40})
		top:add({sx=320,sy=40})

		local m=top:add({sx=320,sy=160})		
		mkeys.setup_keyboard_widgets(m)
		
	end

	function gui.pages.score(master)
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		local w=top:add({sx=320,sy=480,mx=320,my=480,class="flow"})

		gui.add_part(w,"profile_bar")
		w:add({sx=320,sy=40})
		w:add({sx=320,sy=40,color=0xffcccccc,text="A new High!!"})
		w:add({sx=320,sy=40,color=0xffcccccc,text=tostring(gui.data.score)})
		w:add({sx=100,sy=40,color=0xffcccccc,text="Brag",id="profile_score_brag",hooks=gui.hooks})
		w:add({sx=10,sy=40})
		w:add({sx=100,sy=40,color=0xffcccccc,text="List",id="profile_score_list",hooks=gui.hooks})
		w:add({sx=10,sy=40})
		w:add({sx=100,sy=40,color=0xffcccccc,text="Submit",id="profile_score_submit",hooks=gui.hooks})
		w:add({sx=320,sy=40})
		
	end


	function gui.page(pname)
		
		if not gui.master then
			gui.master=oven.rebake("wetgenes.gamecake.widgets").setup({hx=320,hy=480})
		end
	
		gui.master:clean_all()
		gui.master.ids={}
		
		if pname then
			local f=gui.pages[pname]
			if f then
				f(gui.master) -- pass in the master so we could fill up other widgets
			end
		end

		gui.page_name=pname

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
	
	gui.initdata()
	return gui
end
