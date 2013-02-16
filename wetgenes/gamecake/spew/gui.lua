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

	gui.clicks={} -- put onclick function in here
	gui.pages={} -- put functions to fill in pages in here

	local cake=oven.cake
	local sounds=cake.sounds



	function gui.setup(parent)
		gui.offset=1
		
		gui.parent=parent
		gui.master=parent.master
--		gui.page()
	end
	
	local wdata=oven.rebake("wetgenes.gamecake.widgets.data")
	local mkeys=oven.rebake("wetgenes.gamecake.mods.keys")
	local sprofiles=oven.rebake("wetgenes.gamecake.spew.profiles")
	local sscores=oven.rebake("wetgenes.gamecake.spew.scores")

	local sounds=oven.rebake("wetgenes.gamecake.sounds")
	
--	local bpages=oven.rebake("bulb.pages")
	
	gui.ids={}
	gui.data={}
	function gui.initdata() -- call this later
		gui.data.mode=1
		gui.data.score=0
		gui.data.name=wdata.new_data({class="string",hooks=gui.hooks})
		gui.data.vol_music=wdata.new_data({id="vol_music",class="number",hooks=gui.hooks,num=100,min=0,max=100,step=1})
		gui.data.vol_sfx=wdata.new_data({id="vol_sfx",class="number",hooks=gui.hooks,num=100,min=0,max=100,step=1})
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

		if act=="value" then
			if     id=="vol_music" then
				sounds.vol_stream=widget.num/100
			elseif id=="vol_sfx" then
				sounds.vol_sfx=widget.num/100
			end
		end

		if act=="click" then
		
print("click",id)

			if gui.clicks[id] then
				return gui.clicks[id](act,widget)
			end


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

			elseif id=="score_list" then

				gui.offset=1
				gui.page("score_list")
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

		top:add({sx=320,sy=20})
		top:add({sx=320,sy=40,text_color=0xffffffff,text="My name is"})
		top:add({sx=320,sy=20})

		top:add({sx=20,sy=40})
		top:add({sx=280,sy=40,color=0xffcccccc,text=gui.data.name:value(),id="profile_name_edit",hooks=gui.hooks})
		top:add({sx=20,sy=40})

		top:add({sx=320,sy=40*8})

		top:add({sx=110,sy=40,color=0xffcccccc,text="OK",id="profile_return",hooks=gui.hooks})
		top:add({sx=100,sy=40})
		top:add({sx=110,sy=40,color=0xffcccccc,text="Cancel",id="profile_goto",hooks=gui.hooks,user="profiles"})

	end

	function gui.pages.profile_name_edit(master)
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})
		
		gui.data.name:value( sprofiles.get("name") )

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

		top:add({sx=110,sy=40,color=0xffcccccc,text="OK",id="profile_name_set",hooks=gui.hooks})
		top:add({sx=100,sy=40})
		top:add({sx=110,sy=40,color=0xffcccccc,text="Cancel",id="profile_goto",hooks=gui.hooks,user="profile"})

		top:add({sx=320,sy=40})

		local m=top:add({sx=320,sy=160})		
		mkeys.setup_keyboard_widgets(m)
		
	end

	function gui.pages.score(master)

		local score=gui.data.score or 0
		local best=sscores.list({})[1]			best=(best and best.score) or 0
		local mine=sscores.get_best_score({})	mine=(mine and mine.score) or 0
		
		if best<score then best=score end
		if mine<score then mine=score end
		
		local best_pct=0
		local mine_pct=0
		if best<1 then best_pct=0 else best_pct=math.floor(100*score/best) end
		if mine<1 then mine_pct=0 else mine_pct=math.floor(100*score/mine) end

		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})
		top:add({sx=320,sy=40})

		top:add({sx=320,sy=80,text_color=0xffffffff,text="You scored!!"})
		
		top:add({sx=20,sy=40})
		top:add({sx=280,sy=40,color=0xffcccccc,text=tostring(score)})
		top:add({sx=20,sy=40})
		
		top:add({sx=320,sy=40})

		top:add({sx=20,sy=40})
		top:add({sx=130,sy=40,color=0xffcccccc,text="Brag",id="score_brag",hooks=gui.hooks})
		top:add({sx=20,sy=40})
		top:add({sx=130,sy=40,color=0xffcccccc,text="List",id="score_list",hooks=gui.hooks})
--		top:add({sx= 5,sy=40})
--		top:add({sx=90,sy=40,color=0xffcccccc,text="Send",id="profile_score_send",hooks=gui.hooks})
		top:add({sx=20,sy=40})

		top:add({sx=320,sy=40})
		top:add({sx=320,sy=60,text_color=0xffffffff,text=""..best_pct.."% success"})
		top:add({sx=320,sy=60,text_color=0xffffffff,text=""..mine_pct.."% effort"})
		top:add({sx=320,sy=40})
		top:add({sx=320,sy=40})
		
	end

	function gui.pages.score_list(master)

		gui.clicks.score_list_less=function()
			gui.offset=gui.offset-5
			if gui.offset<1 then
				gui.offset=1
				if gui.returnpage then gui.returnpage() end -- callback to return to original menu			
			else
				gui.page("score_list")
			end
		end
		gui.clicks.score_list_more=function()
			gui.offset=gui.offset+5
			gui.page("score_list")
		end
	
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})
		
		local tab={}
		local sc=sscores.list({offset=gui.offset})
		local nomore=not sc[6]
		for i=1,5 do
			local v=sc[i]
			if v then
				tab[i]={
					idx=v.idx,
					name=v.name,
					score=v.score,
				}
			end
		end

		
		top:add({sx=320,sy=5})

		top:add({sx=20,sy=30})
		top:add({sx=280,sy=30,text_color=0xffffffff,text="High Scores"})
		top:add({sx=20,sy=30})

		top:add({sx=320,sy=5})


		for i=1,5 do
			local v=tab[i]

			top:add({sx=5,sy=60})

			local s=top:add({sx=310,sy=60,hx=310,hy=60,mx=310,my=60,class="flow",ax=0,ay=0,font="Vera",text_size=24})			
			if v then
				s:add(
					{sx=100,sy=30,color=0xffcccccc,text=wstr.str_append_english_number_postfix(v.idx),hooks=gui.hooks},
					{sx=210,sy=30,color=0xffcccccc,text=wstr.str_insert_number_commas(v.score),hooks=gui.hooks},
					{sx=310,sy=30,color=0xffcccccc,text=v.name,hooks=gui.hooks})
			else
			end

			top:add({sx=5,sy=60})

			top:add({sx=320,sy=10})

		end
		
		if nomore then
			top:add({sx=100,sy=40,color=0xffcccccc,text="Back",id="score_list_less",hooks=gui.hooks})
			top:add({sx=220,sy=40})
		else
			top:add({sx=100,sy=40,color=0xffcccccc,text="Back",id="score_list_less",hooks=gui.hooks})
			top:add({sx=120,sy=40})
			top:add({sx=100,sy=40,color=0xffcccccc,text="More",id="score_list_more",hooks=gui.hooks})
		end

	end

	function gui.pages.settings(master)
	
		gui.clicks.settings_return=function()
			gui.returnpage()
		end
		gui.clicks.settings_quit=function()
			gui.page("quit")
		end


		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		top:add({sx=320,sy=40,text_color=0xffffffff,text="Music volume"})
		top:add({class="slide",color=0xffcccccc,sx=320,sy=40,datx=gui.data.vol_music,hooks=gui.hooks})
		top:add({sx=320,sy=40,text_color=0xffffffff,text="Sound effects volume"})
		top:add({class="slide",color=0xffcccccc,sx=320,sy=40,datx=gui.data.vol_sfx,hooks=gui.hooks})

		top:add({sx=320,sy=40*9})

		top:add({sx=120,sy=40,color=0xffcccccc,text="Back",id="settings_return",hooks=gui.hooks})
		top:add({sx=80,sy=40})
		top:add({sx=120,sy=40,color=0xffcc4444,text="Quit",id="settings_quit",hooks=gui.hooks})


	end
	
	function gui.pages.quit(master)

		gui.clicks.quit_back=function()
			gui.page("settings")
		end
		gui.clicks.quit_exit=function()
			oven.next=true -- really quit
		end
				
		local top=master:add({hx=320,hy=480,mx=320,my=480,class="flow",ax=0,ay=0,font="Vera",text_size=24})

		top:add({sx=320,sy=40*3})

		top:add({sx=320,sy=40*2,text_color=0xffffffff,text="Make your time!"})

		top:add({sx=20,sy=40*2})
		top:add({sx=120,sy=40*2,color=0xffcccccc,text="Back",id="quit_back",hooks=gui.hooks})
		top:add({sx=40,sy=40*2})
		top:add({sx=120,sy=40*2,color=0xffcc4444,text="Quit",id="quit_exit",hooks=gui.hooks})
		top:add({sx=20,sy=40*2})

		top:add({sx=320,sy=40*5})
		
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
