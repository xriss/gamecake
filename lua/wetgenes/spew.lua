--
-- (C) 2022 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local log,dump=require("wetgenes.logs"):export("log","dump")

local wjson = require("wetgenes.json")

--[[#lua.wetgenes.spew

	local spew=require("wetgenes.spew").connect(oven.tasks)

You must pass in a previously created tasks object that you will be 
calling update on regually so that our coroutines can run. eg oven.tasks

Connect and talk to the wetgenes spew server using wetgenes.tasks 
task for meta state and a thread for lowlevel sockets.

You push messages in and can pull messages out or, setup a hook 
function to auto pull and process messages in our coroutine as they are 
received.

]]

-- module
local M={ modname = (...) } package.loaded[M.modname] = M 

M.id=0 -- unique incremental id for new tasks etc

--[[#lua.wetgenes.spew.connect

	local spew=require("wetgenes.spew").connect(oven.tasks)
	local spew=require("wetgenes.spew").connect(oven.tasks,host)
	local spew=require("wetgenes.spew").connect(oven.tasks,host,port)

You must pass in an active tasks object.

Create task and thread connection to the host. host and port 
default to wetgenes.com and 5223 so can be left blank unless you want 
to connect to a local host for debugging.

There are now 3 ways to handle msgs

	spew.push(msg)

To send a msg

	local msg,s=spew.pull()

To receive a message, will return nil if no messages available. The 
second return value is the input string packet for this decoded 
message.

	spew.hook=function(msg,s) print(msg) end

Set a hook functions to auto pull all available messages durring tasks 
update. Note that if you do this spew.pull() will no longer work as all 
messages are auto pulled and sent to this hook function.

Note when receiving a msg you must not alter or cache the table you 
are given as it is internal data and is reused. You must duplicate it 
if you want to keep it arround.

]]

M.connect=function(tasks,host,port,url)

	M.id=M.id+1

	local spew={}
	
	spew.id=M.id
	
	spew.push_stack={}
	spew.pull_data=""
	spew.pull_state={}
	
	local url_decode=function(str)
		return string.gsub(str, "%%(%x%x)", function(hex)
			return string.char(tonumber(hex, 16))
		end)
	end

	local url_encode=function(str)
		return string.gsub(str, "([&=%%])", function(c)
			return string.format("%%%02x", string.byte(c))
		end)
	end

	local strmsg=function(it)
		local s="&"
		for n,v in pairs(it) do
			s=s..url_encode(n).."="..url_encode(v).."&"
		end
		return s.."\0"
	end

	spew.connect=function(_host,_port)
	end
	
	spew.push=function(m)
		spew.push_stack[#spew.push_stack+1]=strmsg(m)
	end

	spew.pull=function()
		local a=string.find(spew.pull_data,"\0")
		if a then
			local cs=string.sub(spew.pull_data,1,a) -- command string
			spew.pull_data=string.sub(spew.pull_data,a+1) -- remainder
			
			for ps in string.gmatch(cs,"([^&]+)") do -- all the bits between &
				local eq=string.find(ps,"=") -- which must have an =
				if eq then
					local sa=url_decode(ps:sub(1,eq-1))
					local sb=url_decode(ps:sub(eq+1))
					spew.pull_state[sa]=sb
				end
			end

			return spew.pull_state,cs -- return the current state and the string that triggered it
		end
	end

	local js_eval -- function call into javascript if we are an emcc build
	do
		local ok,lib=pcall(function() return require("wetgenes.win.emcc") end )
		if ok and lib then js_eval=lib.js_eval end
	end

if js_eval then -- need special js codes

	local js_call=function(script,opts)
		local js=[[
(function(opts){
	var ret={};
]]..script..[[
	return JSON.stringify(ret);
})(]]..wjson.encode(opts or {})..[[);
]]
		local rets=js_eval(js)
		return wjson.decode( rets or "{}" ) or {}
	end
	
	js_call([[

globalThis.wetgenes_tasks=globalThis.wetgenes_tasks || {};
globalThis.wetgenes_tasks[opts.data_id]=globalThis.wetgenes_tasks[opts.data_id] || {};

var data=globalThis.wetgenes_tasks[opts.data_id];
data.send=[];
data.recv=[];

data.onmessage=function(e){
	data.recv.push(e.data);
}
data.onopen=function(e){
}
data.onclose=function(e){
}
data.onerror=function(e){
}

data.sock=new WebSocket(opts.url);
data.sock.onmessage=data.onmessage;
data.sock.onopen=data.onopen;
data.sock.onclose=data.onclose;
data.sock.onerror=data.onerror;

console.log(data.sock);

]],{data_id="spew_thread_"..spew.id,url=(url or "wss://wetgenes.com/_websocket")})

	spew.task=tasks:add_task({
		code=function(linda,task_id,task_idx)

			while true do
				if #spew.push_stack>0 then
					js_call([[
var data=globalThis.wetgenes_tasks[opts.data_id];

for(var i=0;i<opts.datas.length;i++)
{
	data.send.push(opts.datas[i]);
}

]],{data_id="spew_thread_"..spew.id,datas=spew.push_stack})

					while #spew.push_stack>0 do spew.push_stack[#spew.push_stack]=nil end
				end

				local ret=js_call([[
var data=globalThis.wetgenes_tasks[opts.data_id];

ret.data=[]

if(data.sock)
{
	if(data.sock.readyState==1)
	{
		while(data.send.length>0)
		{
			data.sock.send(data.send.shift());
		}
		while(data.recv.length>0)
		{
			ret.data.push(data.recv.shift());
		}
	}
}

]],{data_id="spew_thread_"..spew.id})
				for i,v in ipairs(ret.data) do
					spew.pull_data=spew.pull_data..v.."\0"
				end
				
				if spew.hook then -- we will auto pull
					for m,s in spew.pull do
						spew.hook(spew,m,s)
					end
				end
				
				coroutine.yield()
			end

		end,
	})
	
else -- normal sockets

	spew.thread=tasks:add_thread({
		count=1,
		id="spew_thread_"..spew.id,
		globals={ client_host=(host or "wetgenes.com") , client_port=(port or 5223 ) , client_url=(url or "wss://wetgenes.com/_websocket") },
--		globals={ client_host=(host or "127.0.0.1") , client_port=(port or 5223 ) , client_url=(url or "ws://127.0.0.1:5223") },
--		globals={ client_host=(host or "wetgenes.com") , client_port=(port or 5223 ) , client_url=(url or "ws://localhost:7071") },
		code=tasks.client_code,
	})

	spew.task=tasks:add_task({
		code=function(linda,task_id,task_idx)

			while true do
			
				if spew.push_stack[1] then -- send and recv
					while spew.push_stack[1] do
						local r=assert(tasks:client({
							data=table.remove(spew.push_stack,1),
							task="spew_thread_"..spew.id,
						}))
						if r.data then spew.pull_data=spew.pull_data..r.data end
					end
				else -- just recv
					local r=assert(tasks:client({
						task="spew_thread_"..spew.id,
					}))
					if r.data then spew.pull_data=spew.pull_data..r.data end
				end
				
				if spew.hook then -- we will auto pull
					for m,s in spew.pull do
						spew.hook(spew,m,s)
					end
				end
				
				coroutine.yield()
			end

		end,
	})

end

	return spew
end


--[[#lua.wetgenes.spew.test


test

]]

M.test=function()

	local tasks=require("wetgenes.tasks").create()
	local spew=require("wetgenes.spew").connect(tasks)

	spew.hook=function(spew,msg,str) -- will be called with new data from within the coroutine during tasks:update
		print(str)
--		dump(msg)
	end

	spew.push({cmd="note",note="playing",arg1="unzone",arg2="",arg3="",arg4=""})
	spew.push({cmd="login",name="tester"})
	spew.push({cmd="join",room="public.tv"})

	while true do
		tasks:update()
	end

end
