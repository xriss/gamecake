--
-- (C) 2017 kriss@wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local chatdown=M


-----------------------------------------------------------------------------
--[[#chatdown.parse

	chats = chatdown.parse(text)

Parse text from flat text chatdown format into heirachical chat data, 
something that can be output easily as json.

This gives us a readonly data structure that can be used to control 
what text is displayed during a chat session.

This is intended to be descriptive and logic less, any decision logic 
should be added using a real language that operates on this data and 
gets triggered by the names used. EG, filter out decisions unless 
certain conditions are met or change responses to redirect to an 
alternative.


]]
-----------------------------------------------------------------------------
chatdown.parse=function(chat_text)

	local function text_to_trimed_lines(text)

		local lines={}
		local i=1
		
		for s in string.gmatch(text, "([^\n]*)\n?") do
			s=s:match("^%s*(.-)%s*$")
			lines[i]=s
			i=i+1
		end
				
		return lines
	end

	local last_name=""
	local last_count=1

	local lines=text_to_trimed_lines(chat_text)

	local text={}

	local chats={}
	local chat={}

	local decisions={}
	local decision={}

	local responses={}
	local response={}

	local proxies={}
	local proxy={}

	for i,v in ipairs(lines) do

		local name

		local code=v:sub(1,1)

		if code=="#" then -- #description

			local c=v:sub(2,2)
			
			if c=="#" or c=="<" or c=="=" or c==">" or c=="-" then -- escape codes

				v=v:sub(2) -- just remove hash from start of line
			
			else

				name,v=v:match("%#(%S*)%s*(.*)$")
				
				text={}
				responses={}
				decisions={}
				proxies={}
				chat={text=text,decisions=decisions,proxies=proxies,responses=responses}
				
				
				chat.name=name
				chat.text=text

				if name~="" then -- ignore empty names
				
					assert( not chats[name] , "description name used twice on line "..i.." : "..name )
					chats[name]=chat
				
				end
				
			end

		elseif code=="<" then -- <response

			name,v=v:match("%<(%S*)%s*(.*)$")
			
			-- if name is empty then we use an auto reverence to the last noname decision
			if name=="" then name=last_name.."__"..last_count	-- use reference 
			else last_name=name last_count=1 end -- reset reference

			text={}
			decisions={}
			proxies={}
			response={text=text,name=name,decisions=decisions,proxies=proxies}

			if name~="" then -- ignore empty names
			
				assert( not responses[name] , "response name used twice on line "..i.." : "..name )
				responses[name]=response
			
			end

		elseif code==">" then -- >decision
		
			name,v=v:match("%>(%S*)%s*(.*)$")

			-- if name is empty then we use an auto reverence to the next nonmame response
			if name=="" then last_count=last_count+1 name=last_name.."__"..last_count end -- increment reference
		
			text={}
			proxies={}
			decision={text=text,name=name,proxies=proxies}

			decisions[#decisions+1]=decision

		elseif code=="=" then -- =proxy
		
			name,v=v:match("%=(%S*)%s*(.*)$")

			text={}
			proxy=text

			if name~="" then -- ignore empty names
			
				assert( not proxies[name] , "proxy name used twice on line "..i.." : "..name )
				proxies[name]=proxy
			
			end
			
		elseif code=="-" then -- -comment
		
			v=nil

		end
		
		if v then

			text[#text+1]=v

		end

		
	end

	-- cleanup output

	local cleanup_proxies=function(proxies)

		local empty=true
		for n,v in pairs(proxies) do
			empty=false
			proxies[n]=table.concat(v,"\n"):match("^%s*(.-)%s*$")
		end
		if empty then return end

		return proxies
	end

	local cleanup_text=function(text)

		local t={""}

		for i,v in ipairs(text) do
			if v=="" then
				if t[#t]~="" then t[#t+1]="" end -- start a new string?
			else
				t[#t]=(t[#t].." "..v):match("^%s*(.-)%s*$")
			end
		end
		
		while t[1]=="" do table.remove(t,1) end
		while t[#t]=="" do table.remove(t,#t) end
		
		if not t[1] then return nil end -- empty text
		if t[2] then return t end -- return an array
		return t[1] -- just the first line
	end


	for name,chat in pairs(chats) do

		chat.text=cleanup_text(chat.text)
		chat.proxies=cleanup_proxies(chat.proxies)

		for id,decision in pairs(chat.decisions) do

			decision.text=cleanup_text(decision.text)
			decision.proxies=cleanup_proxies(decision.proxies)
		end

		for id,response in pairs(chat.responses) do

			response.text=cleanup_text(response.text)
			response.proxies=cleanup_proxies(response.proxies)

			for id,decision in pairs(response.decisions) do

				decision.text=cleanup_text(decision.text)
				decision.proxies=cleanup_proxies(decision.proxies)
			end
		end

	end

	return chats

end



-----------------------------------------------------------------------------
--[[#chatdown.setup_chat

	chat = chatdown.setup_chat(chat,chats,chat_name,response_name)

Setup the state for a chat using this array of chats as text data to be 
displayed.

We manage proxy data and callbacks from decisions here.

]]
-----------------------------------------------------------------------------
chatdown.setup_chat=function(chat,chats,chat_name,response_name)

	local dotnames=function(name)
		local n,r=name,name
		local f=function(a,b)
			r=n -- start with the full string
			n=n and n:match("^(.+)(%..+)$") -- prepare the parent string
			return r
		end
		return f
	end
	--for n in dotnames("control.colson.2") do print(n) end

	local chat=chat or {}
	
	chat.chats=chats
	chat.name=chat_name
	chat.data=chats.data
	chat.proxies={}
	chat.viewed={}

-- hook, replace to be notified of changes, by default we print debuging information
	chat.changes=function(change,...)
		local a,b=...

		if     change=="description" then			print("description",a.name)
		elseif change=="response"    then			print("response   ",a.name)
		elseif change=="decision"    then			print("decision   ",a.name)
		elseif change=="proxy"       then			print("proxy      ",a,b)
		end
		
	end
	
	chat.get_proxy=function(text)
		return chats.get_proxy(text,chat.name)
	end
	
	chat.set_proxy=function(text,val)
		return chats.set_proxy(text,val,chat.name)
	end

	chat.replace_proxies=function(text)
		return chats.replace_proxies(text,chat.name)
	end

	chat.set_proxies=function(proxies)
		for n,v in pairs(proxies or {}) do
			chat.changes("proxy",n,v)
			chat.set_proxy(n,v)
		end
    end
	
	chat.set_description=function(name)
	
		chat.description_name=name	
		chat.description={} -- chat.chats[name]
		chat.responses={} -- chat.description.responses
		
		for n in dotnames(name) do -- inherit chunks data
			local v=chat.data[n]
			if v then
				for n2,v2 in pairs(v) do -- merge base settings
					chat.description[n2]=chat.description[n2] or v2
				end 
				for n2,v2 in pairs(v.responses or {}) do -- merge responses
					chat.responses[n2]=chat.responses[n2] or v2
				end
			end
		end

		chat.changes("description",chat.description)

		chat.set_proxies(chat.description.proxies)

	end

	chat.set_response=function(name)
	
		chat.viewed[name]=(chat.viewed[name] or 0) + 1 -- keep track of what responses have been viewed
	
		chat.response_name=name
		chat.response={} -- chat.responses[name]
		chat.decisions={} -- chat.response and chat.response.decisions
		
		local merged_proxies={}

		local decision_names={} -- keep track of previously seen exit nodes

		for n in dotnames(name) do -- inherit responses data
			local v=chat.responses[n]
			if v then
				for n2,v2 in pairs(v) do -- merge base settings
					chat.response[n2]=chat.response[n2] or v2
				end 
				for np,vp in pairs(v.proxies or {}) do -- merge proxy changes
					merged_proxies[np]=merged_proxies[np] or vp
				end
				for n2,v2 in ipairs(v.decisions or {}) do -- join all decisions
					local r={}
					for n3,v3 in pairs(v2) do r[n3]=v3 end -- copy

					if not r.text then -- use text from description prototype decisions
						for i,p in ipairs(chat.description.decisions or {} ) do -- search
							if r.name==p.name then r.text=p.text break end -- found and used
						end
					end
					
					local result=true
					if r.name:find("?") then -- query string
						r.name,r.query=r.name:match("(.+)?(.+)")
						
						local t={}
						r.query:gsub("([^&|!=<>]*)([&|=<>!]*)",function(a,b) if a~="" then t[#t+1]=a end if b~="" then t[#t+1]=b end end)
						
						local do_test=function(a,b,c)

							local a=chat.get_proxy(a)

							if     b=="<" then					return ( tonumber(a) < tonumber(c) )
							elseif b==">" then					return ( tonumber(a) > tonumber(c) )
							elseif b=="<=" then					return ( tonumber(a) <= tonumber(c) )
							elseif b==">=" then					return ( tonumber(a) >= tonumber(c) )
							elseif b=="=" or b=="==" then		return ( tostring(a) == c )
							elseif b=="!=" then					return ( tostring(a) ~= c )
							elseif not b then					return a and true or false
							end
							
							return false
						end
						
						local test={"|"}
						local tests={test}
						for i,v in ipairs(t) do
							if v=="&" or v=="|" then
								test={v}
								tests[#tests+1]=test
							elseif v=="&!" or v=="|!" then
								test={v:sub(1,1),v:sub(2,2)}
								tests[#tests+1]=test
							else
								test[#test+1]=v
							end
						end

						result=false
						for i,v in ipairs(tests) do
						
							local t
							if v[2]=="!" then t= not do_test(v[3],v[4],v[5]) else t=do_test(v[2],v[3],v[4]) end
							
							if v[1]=="|" then result=result or  t end
							if v[1]=="&" then result=result and t end
						
						end

					end
					
					r.name=chat.replace_proxies(r.name) -- can use proxies in name
					
					if not decision_names[r.name] then -- only add unique decisions
						if result then -- should we show this one?
							chat.decisions[#chat.decisions+1]=r
						end
					end
					decision_names[r.name]=true
				end 
			end

		end
		
		chat.changes("response",chat.response)

		chat.set_proxies(merged_proxies)

	end

	
	chat.set_description(chat_name)
	chat.set_response(response_name)
	
	chat.get_menu_items=function()
		local items={cursor=1,cursor_max=0}
		
		items.title=chat.description_name
		
		local ss=chat.response and chat.response.text or {} if type(ss)=="string" then ss={ss} end
		for i,v in ipairs(ss) do
			if i>1 then
				items[#items+1]={text="",chat=chat} -- blank line
			end
			items[#items+1]={text=chat.replace_proxies(v)or"",chat=chat}
		end

		for i,v in ipairs(chat.decisions or {}) do

			items[#items+1]={text="",chat=chat} -- blank line before each decision

			local ss=v and v.text or {} if type(ss)=="string" then ss={ss} end

			local color=30
			if chat.viewed[v.name] then color=28 end -- we have already seen the response to this decision
			
			local f=function(item,menu)

				if item.decision and item.decision.name then

					chat.changes("decision",item.decision)

					chat.set_response(item.decision.name)

					chat.set_proxies(item.decision.proxies)

					menu.show(chat.get_menu_items())

				end
			end
			
			items[#items+1]={text=chat.replace_proxies(ss[1])or"",chat=chat,decision=v,cursor=i,call=f,color=color} -- only show first line
			items.cursor_max=i
		end

		return items
	end

	return chat
end


-----------------------------------------------------------------------------
--[[#chatdown.setup

	chats = chatdown.setup(chat_text)

parse and initialise state data for every chat chunk

]]
-----------------------------------------------------------------------------
chatdown.setup=function(chat_text)

	local chats={}

	chats.data=chatdown.parse(chat_text)
	
	chats.names={}
	
	chats.get=function(name)
		return chats.names[name]
	end
	
	chats.get_menu_items=function(name)
	
		return chats.get(name).get_menu_items()
	end
	
	chats.get_proxy=function(s,default_root)
		local root,proxy=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,proxy=default_root,s end -- no root use full string as proxy name
		local proxies=(chats.get(root) or {}).proxies or {} -- get root proxies or empty table
		return proxies[proxy]
	end

	chats.set_proxy=function(s,v,default_root)
		local root,proxy=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,proxy=default_root,s end -- no root use full string as proxy name
		local proxies=(chats.get(root) or {}).proxies or {} -- get root proxies or empty table

-- add inc/dec operators here?
		local t=v:sub(1,1)
		if t=="-" then
			local n=tonumber(v:sub(2))
			proxies[proxy]=(proxies[proxy] or 0 ) + n
		elseif t=="+" then
			local n=tonumber(v:sub(2))
			proxies[proxy]=(proxies[proxy] or 0 ) - n
		else
			proxies[proxy]=v
		end
		
		return proxies[proxy]
	end

	chats.replace_proxies=function(text,default_root)

		if not text then return nil end
--		if not proxies then return text end

		local ret=text
		for sanity=0,100 do
			local last=ret
			ret=ret:gsub("{([^}%s]+)}",function(a)
				return chats.get_proxy(a,default_root) or "{"..a.."}"
			end)
			if last==ret then break end -- no change
		end

		return ret
	end


	for n,v in pairs(chats.data) do -- setup each chat
	
		local chat={}
		chats.names[n]=chat
		chatdown.setup_chat(chat,chats,n,"welcome")
		
	end

	return chats
end

