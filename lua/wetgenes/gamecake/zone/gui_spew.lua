-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wjson = require("wetgenes.json")

local log,dump=require("wetgenes.logs"):export("log","dump")


--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc



B.setup=function(zgui)
	log("setup",M.modname)
	
	local datas=zgui.master.datas
	
	datas.new({id="chat_input",class="string",hooks=zgui.hooks,str=""})
	local list={
		{ str="Chat", },
		{ str="Room", },
	}
	for i,v in ipairs(list) do v.num=i end
	datas.new({id="chat_display",class="list",hooks=zgui.hooks,num=1,list=list})

	zgui.confirm["window_chat_input"]=function(it)
		local master=zgui.master
		local s=datas.get("chat_input"):value()
		print("confirm",it.id,s)
		datas.get("chat_input"):value("")
		master.set_focus(it)
		oven.spew.push({cmd="cmd",txt=s})
	end

	local spew_hook=function(spew,msg,str)
--		print(str)
--		dump(msg)
--[[
		local t={}
		local n="cmd"
		while n and msg[n] do
			n=msg[n]
			t[#t+1]=n
		end
		print(unpack(t))
]]

		local ss=zgui.master.grid_size
		local scroll=zgui.master.ids.window_chat_scroll
		
		local rgb=tostring(msg.rgb or "FFF")
		do
			local r=tonumber(rgb:sub(1,1),16) or 15
			local g=tonumber(rgb:sub(2,2),16) or 15
			local b=tonumber(rgb:sub(3,3),16) or 15
			rgb=0xff000000+r*0x00100000+g*0x00001000+b*0x00000010
		end
		
--print(scroll.hy,scroll.pan.hy)
		
		if msg.cmd=="say" then
			scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
			text_align="wrap",color=0,text=msg.frm..": "..msg.txt,
			})
		elseif msg.cmd=="act" then
			scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
			text_align="wrap",color=0,text="**"..msg.frm.." "..msg.txt.."**",
			})
		elseif msg.cmd=="lnk" then
			scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
			text_align="wrap",color=0,text=msg.lnk,
			})
		elseif msg.cmd=="note" then
			if msg.note=="act" then
				scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
				text_align="wrap",color=0,text="-*"..msg.arg1.."*-",
				})
			elseif msg.note=="notice" or msg.note=="welcome" or msg.note=="warning" or msg.note=="error" then
				scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
				text_align="wrap",color=0,text="-="..msg.arg1.."=-",
				})
			elseif msg.note=="ban" then
				scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
				text_align="wrap",color=0,text="-= "..msg.arg2.." has been banned by "..msg.arg1.." =-",
				})
			elseif msg.note=="gag" then
				scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
				text_align="wrap",color=0,text="-= "..msg.arg2.." has been gagged by "..msg.arg1.." =-",
				})
			elseif msg.note=="dis" then
				scroll.pan:add({class="paragraph",size="fullx",hy=ss,text_color=rgb,
				text_align="wrap",color=0,text="-= "..msg.arg2.." has been disemvoweled by "..msg.arg1.." =-",
				})
			end
		end
		
		while #scroll.pan>128 do -- limit scrollback
			table.remove(scroll.pan,1)
		end
		
		scroll.pan:set_dirty()
		zgui.master.request_layout=true
		zgui.master.request_refresh=true

-- auto scroll to bottom?
		if	scroll.daty.num==scroll.daty.max then
			zgui.master.later_append( function()
				local scroll=zgui.master.ids.window_chat_scroll
				scroll:resize()
				scroll:layout()
				scroll.daty:value(scroll.daty.max)
			end )
		end
	end
	
	zgui.spew_connect=function()
		if not oven.spew then -- first connection
			oven.spew=require("wetgenes.spew").connect(oven.tasks)
			oven.spew.hook=spew_hook
		else -- reconnect
			oven.spew.connect()
			oven.spew.hook=spew_hook
		end

		oven.spew.push({cmd="note",note="playing",arg1="unzone",arg2="",arg3="",arg4=""})
		oven.spew.push({cmd="session" , name=datas.get("user_name"):value() , sess=datas.get("user_session"):value() })

--		oven.spew.push({cmd="join",room="public.tv"})

	end

	local plan_windows=function()

		local def=require("wetgenes.gamecake.widgets.defs").create(zgui.master.grid_size)

		def.set({
			class="*",
			hooks=zgui.hooks,
			hx=12,
			hy=1,
		})

		local win=def.add(zgui.screen.windows,{

			class="window",px=16,py=2,id="window_chat",title="Chat",hidden=false,
			panel_mode="fill",
			hy=16,
			{
				class="three",size="full",three_axis="y",
				{ hy=1, size="fullx",
					{class="menudrop",id="chat_display",data="chat_display",color=0,hx=5},
				},
				{ class="scroll",id="window_chat_scroll", },
				{ hy=1, size="fullx",
					{ size="full", class="textedit", color=0, id="window_chat_input", data="chat_input", },
				},
			},

		})

	end

	zgui.plan_windows_list.spew=plan_windows

		
end


return B
end
