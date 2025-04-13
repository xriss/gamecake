-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local wjson = require("wetgenes.json")
local snames=require("wetgenes.gamecake.spew.names")

local log,dump=require("wetgenes.logs"):export("log","dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
M.bake=function(oven,B) B=B or {} -- bound to oven for gl etc

local genes_api="https://wetgenes.com/genes/"


B.setup=function(zgui)
	log("setup",M.modname)
	
	local datas=zgui.master.datas

	local data_load=function()
		local ssettings=oven.rebake("wetgenes.gamecake.spew.settings")

		datas.get("user_name"):value( ssettings.get("wetgenes_user_name") )
		if datas.get("user_name"):value()=="" then
			datas.get("user_name"):value( snames.random() )
		end
		datas.get("user_email"):value( ssettings.get("wetgenes_user_email") )
		datas.get("user_password"):value( ssettings.get("wetgenes_user_password") )
		datas.get("user_password_remember"):value( ssettings.get("wetgenes_user_password_remember") )
		datas.get("user_auto_login"):value( ssettings.get("wetgenes_user_auto_login") )
		datas.get("user_session"):value( ssettings.get("wetgenes_user_session") )
		datas.get("user_ip"):value( ssettings.get("wetgenes_user_ip") )
	end

	local data_save=function()
		local ssettings=oven.rebake("wetgenes.gamecake.spew.settings")

		ssettings.set("wetgenes_user_name",datas.get("user_name"):value() )
		ssettings.set("wetgenes_user_email",datas.get("user_email"):value() )
		if datas.get("user_password_remember"):value()==1 then
			ssettings.set("wetgenes_user_password",datas.get("user_password"):value() ) -- remember (: very unsafe please no :)
		else
			ssettings.set("wetgenes_user_password","" ) -- forget
		end
		ssettings.set("wetgenes_user_password_remember",datas.get("user_password_remember"):value() )
		ssettings.set("wetgenes_user_auto_login",datas.get("user_auto_login"):value() )
		ssettings.set("wetgenes_user_session",datas.get("user_session"):value() )
		ssettings.set("wetgenes_user_ip",datas.get("user_ip"):value() )
		ssettings.save()
	end
	
	datas.new({id="user_status",class="string",hooks=zgui.hooks,str="Checking"})
	datas.new({id="user_error",class="string",hooks=zgui.hooks,str="..."})
	datas.new({id="user_name",class="string",hooks=zgui.hooks,str=""})
	datas.new({id="user_email",class="string",hooks=zgui.hooks,str=""})
	datas.new({id="user_password",class="string",hooks=zgui.hooks,str=""})
	datas.new({id="user_password_remember",class="number",hooks=zgui.hooks,num=0})
	datas.new({id="user_auto_login",class="number",hooks=zgui.hooks,num=1})
	datas.new({id="user_session",class="string",hooks=zgui.hooks,str=""})
	datas.new({id="user_ip",class="string",hooks=zgui.hooks,str=""})
	datas.new({id="user_flags",class="number",hooks=zgui.hooks,num=0})

	data_load()

	local refresh=function(mode)
		local pages=zgui.master.ids.window_user_pages
		if pages and pages.data then
			pages.data:value(mode or "Status")
			zgui.master.request_redraw=true
		end
	end

	zgui.click["user_name_reroll"]=function(it)
		datas.get("user_name"):value( snames.random() )
	end

	local login_head=function(s)
		datas.get("user_status"):value(s)
		datas.get("user_error"):value("...")
--		datas.get("user_session"):value("")
		datas.get("user_ip"):value("")
		datas.get("user_flags"):value(0)
	end
	local login_foot=function(body)
		datas.get("user_error"):value("OK")
		if body.session then datas.get("user_session"):value(body.session) end
		datas.get("user_email"):value(body.email)
		datas.get("user_name"):value(body.name)
		datas.get("user_ip"):value(body.ip)
		datas.get("user_flags"):value(1)
		data_save()
	end
	
	zgui.click["user_login_password"]=function(it)
		local name=datas.get("user_name"):value()
		local pass=datas.get("user_password"):value()
		
		B.task=oven.tasks:add_task(function(linda,task_id,task_idx,task)
		
			login_head("Checking password")
			refresh()

			local ret=oven.tasks:http({
				url=genes_api.."user/login",
				post={
					name=name,
					pass=pass,
				}
			})			
			if B.task~=task then return end -- are we an old task?

			if ret.error then -- request failed
				datas.get("user_error"):value(ret.error)
				refresh()
				return
			end

			local body=wjson.decode(ret.body)
			
			if body.error then -- something went wrong
				datas.get("user_error"):value(body.error)
				refresh()
				return
			end

			login_foot(body)
			refresh()

			if zgui.spew_connect then zgui.spew_connect() end

			return

		end)
	end

	zgui.click["user_login_session"]=function(it,mode)
		local session=datas.get("user_session"):value()
		
		B.task=oven.tasks:add_task(function(linda,task_id,task_idx,task)
		
			login_head("Checking session")
			refresh()

			local ret=oven.tasks:http({
				url=genes_api.."user/session",
				post={
					session=session,
				}
			})			
			if B.task~=task then return end -- are we an old task?

			if ret.error then -- request failed
				datas.get("user_error"):value(ret.error)
				refresh()
				return
			end

			local body=wjson.decode(ret.body)
			
			if body.error then -- something went wrong
				datas.get("user_error"):value(body.error)
				refresh()
				return
			end

			login_foot(body)
			
			if mode=="auto" then
				refresh("Login")
				zgui.master.ids.window_user.hidden=true
			else
				refresh()
				zgui.master.ids.window_user.hidden=false
			end
			
			if zgui.spew_connect then zgui.spew_connect() end

			return

		end)
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

			class="window",px=2,py=2,hy=8,size="fit",id="window_user",title="User",hidden=true,
			{
				class="tabpages",hy=8,color=0,list={"Login","Create","Reset","Status"},id="window_user_pages",
				{
					hy=7,class="fill",
					{
						class="tabpages",hy=7,color=0,list={"Password","Session"},
						{
							hy=6,class="fill",
							{class="text",text="Name",text_align="left"},
							{class="textedit",data="user_name",color=0},
							{class="text",text="Password",text_align="left"},
							{class="textedit",data="user_password",color=0,text_password="*"},
							{class="text",text="Remember password?",hx=10},
							{class="checkbox",data="user_password_remember",hx=2,text_false="no",text_true="yes"},
							{class="button",id="user_login_password",text="Login with password",color=0},
						},
						{
							hy=6,class="fill",
							{class="text",text="Locked to IP",text_align="left"},
							{class="textedit",data="user_ip",color=0},
							{class="text",text="Session ID",text_align="left"},
							{class="textedit",data="user_session",color=0},
							{class="text",text="Auto Login?",hx=10},
							{class="checkbox",data="user_auto_login",hx=2,text_false="no",text_true="yes"},
							{class="button",id="user_login_session",text="Login with session",color=0},
						}
					}
				},
				{
					hy=7,class="fill",
					{class="text",text="Name",text_align="left"},
					{class="textedit",data="user_name",color=0,hx=10},
					{class="button",id="user_name_reroll",text="reroll",color=0,hx=2},
					{class="text",text="Email",text_align="left"},
					{class="textedit",data="user_email",color=0},
					{class="text",text="Password",text_align="left"},
					{class="textedit",data="user_password",color=0,text_password="*"},
					{class="button",id="user_create",text="Create new account",color=0},
				},
				{
					hy=7,class="fill",
					{},
					{},
					{},
					{},
					{class="text",text="Email",text_align="left"},
					{class="textedit",data="user_email",color=0},
					{class="button",id="user_reset",text="Reset password",color=0},
				},
				{
					hy=7,class="fill",
					{},
					{class="text",data="user_status",text_align="left"},
					{class="text",data="user_error"},
					{},
					{},
					{},
					{},
				},
			},
		})

	end

	zgui.plan_windows_list.user=plan_windows

	
end


return B
end
