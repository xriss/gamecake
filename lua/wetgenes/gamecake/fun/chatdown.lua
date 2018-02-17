--
-- (C) 2017 kriss@wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local chatdown=M


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.parse

	chats = chatdown.parse(text)

Parse text from flat text chatdown format into heirachical chat data, 
something that can be output easily as json.

This gives us a readonly data structure that can be used to control 
what text is displayed during a chat session.

This is intended to be descriptive and logic less, any real logic 
should be added using a real language that operates on this data and 
gets triggered by the names used. EG, filter out gotos unless certain 
complicated conditions are met or change topics to redirect to an 
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

	local gotos={}
	local goto={}

	local topics={}
	local topic={}

	local sets={}
	local set={}

	local ignore=true -- ignore first lines until we see a code

	for i,v in ipairs(lines) do

		local name

		local code=v:sub(1,1)

		if code=="#" then -- #chat set description
			ignore=false -- end long comments

			local c=v:sub(2,2)
			
			if c=="#" or c=="<" or c=="=" or c==">" or c=="-" then -- escape codes

				v=v:sub(2) -- just remove hash from start of line
			
			else

				name,v=v:match("%#(%S*)%s*(.*)$")
				
				text={}
				topics={}
				gotos={}
				sets={}
				chat={text=text,gotos=gotos,sets=sets,topics=topics}
				
				
				chat.name=name
				chat.text=text

				if name~="" then -- ignore empty names
				
					assert( not chats[name] , "chat name used twice on line "..i.." : "..name )
					chats[name]=chat
				
				end
				
			end

		elseif code=="<" then -- <topic
			ignore=false -- end long comments

			name,v=v:match("%<(%S*)%s*(.*)$")
			
			-- if name is empty then we use an auto reverence to the last noname goto
			if name=="" then name=last_name.."__"..last_count	-- use reference 
			else last_name=name last_count=1 end -- reset reference

			text={}
			gotos={}
			sets={}
			topic={text=text,name=name,gotos=gotos,sets=sets}

			if name~="" then -- ignore empty names
			
				assert( not topics[name] , "topic name used twice on line "..i.." : "..name )
				topics[name]=topic
			
			end

		elseif code==">" then -- >goto
			ignore=false -- end long comments
		
			name,v=v:match("%>(%S*)%s*(.*)$")

			-- if name is empty then we use an auto reverence to the next nonmame topic
			if name=="" then last_count=last_count+1 name=last_name.."__"..last_count end -- increment reference
		
			text={}
			sets={}
			goto={text=text,name=name,sets=sets}

			gotos[#gotos+1]=goto

		elseif code=="=" then -- =set
			ignore=false -- end long comments
		
			name,v=v:match("%=(%S*)%s*(.*)$")

			text={}
			set=text

			if name~="" then -- ignore empty names
			
				assert( not sets[name] , "set name used twice on line "..i.." : "..name )
				sets[name]=set
			
			end
			
		elseif code=="-" then -- -comment
		
			if v:sub(1,2)=="--" then -- a multi-line long comment
				ignore=true -- ignore many lines until the next code
			else
				v=nil	-- just ignore this line
			end
		end
		
		if v and not ignore then

			text[#text+1]=v

		end

		
	end

	-- cleanup output

	local cleanup_sets=function(sets)

		local empty=true
		for n,v in pairs(sets) do
			empty=false
			sets[n]=table.concat(v,"\n"):match("^%s*(.-)%s*$")
		end
		if empty then return end

		return sets
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
		chat.sets=cleanup_sets(chat.sets)

		for id,goto in pairs(chat.gotos) do

			goto.text=cleanup_text(goto.text)
			goto.sets=cleanup_sets(goto.sets)
		end

		for id,topic in pairs(chat.topics) do

			topic.text=cleanup_text(topic.text)
			topic.sets=cleanup_sets(topic.sets)

			for id,goto in pairs(topic.gotos) do

				goto.text=cleanup_text(goto.text)
				goto.sets=cleanup_sets(goto.sets)
			end
		end

	end

	return chats

end



-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.setup_chat

	chat = chatdown.setup_chat(chat,chats,chat_name,topic_name)

Setup the state for a chat using this array of chats as text data to be 
displayed.

We manage set data and callbacks from gotos here.

]]
-----------------------------------------------------------------------------
chatdown.setup_chat=function(chat,chats,chat_name,topic_name)

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
	chat.sets={}
	chat.viewed={}
	
	chat.get_set=function(text)
		return chats.get_set(text,chat.name)
	end
	
	chat.set_set=function(text,val)
		return chats.set_set(text,val,chat.name)
	end

	chat.replace_sets=function(text)
		return chats.replace_sets(text,chat.name)
	end

	chat.set_sets=function(sets)
		for n,v in pairs(sets or {}) do
			chat.set_set(n,v)
		end
    end
	
	chat.set_description=function(name)
	
		chat.description={}
		chat.topics={}
		
		for n in dotnames(name) do -- inherit chunks data
			local v=chat.data[n]
			if v then
				for n2,v2 in pairs(v) do -- merge base settings
					chat.description[n2]=chat.description[n2] or v2
				end 
				for n2,v2 in pairs(v.topics or {}) do -- merge topics
					chat.topics[n2]=chat.topics[n2] or v2
				end
			end
		end

		chats.changes(chat,"chat",chat.description)

		chat.set_sets(chat.description.sets)

	end

	chat.set_topic=function(name)
	
		chat.viewed[name]=(chat.viewed[name] or 0) + 1 -- keep track of what topics have been viewed
	
		chat.topic_name=name
		chat.topic={}
		chat.gotos={}
		
		local merged_sets={}

		local goto_names={} -- keep track of previously seen exit nodes

		for n in dotnames(name) do -- inherit topics data
			local v=chat.topics[n]
			if v then
				for n2,v2 in pairs(v) do -- merge base settings
					chat.topic[n2]=chat.topic[n2] or v2
				end 
				for np,vp in pairs(v.sets or {}) do -- merge set changes
					merged_sets[np]=merged_sets[np] or vp
				end
				for n2,v2 in ipairs(v.gotos or {}) do -- join all gotos
					local r={}
					for n3,v3 in pairs(v2) do r[n3]=v3 end -- copy

					if not r.text then -- use text from description prototype gotos
						for i,p in ipairs(chat.description.gotos or {} ) do -- search
							if r.name==p.name then r.text=p.text break end -- found and used
						end
					end
					
					local result=true
					if r.name:find("?") then -- query string
						r.name,r.query=r.name:match("(.+)?(.+)")
						
						local t={}
						r.query:gsub("([^&|!=<>]*)([&|=<>!]*)",function(a,b) if a~="" then t[#t+1]=a end if b~="" then t[#t+1]=b end end)
						
						local do_test=function(a,b,c)

							local a=chat.get_set(a)

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
					
					r.name=chat.replace_sets(r.name) -- can use sets in name
					
					if not goto_names[r.name] then -- only add unique gotos
						if result then -- should we show this one?
							chat.gotos[#chat.gotos+1]=r
						end
					end
					goto_names[r.name]=true
				end 
			end

		end
		
		chats.changes(chat,"topic",chat.topic)

		chat.set_sets(merged_sets)

	end

	chat.get_menu_items=function()
		return chats.chat_to_menu_items(chat)
	end
	
	chat.set_description(chat_name)
	chat.set_topic(topic_name)
	
	return chat
end


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.setup

	chats = chatdown.setup(chat_text)

parse and initialise state data for every chat chunk

]]
-----------------------------------------------------------------------------
chatdown.setup=function(chat_text,changes)

	local chats={}

	chats.data=chatdown.parse(chat_text) -- parse static data
	
	chats.names={}
	
	chats.get=function(name)
		return chats.names[name]
	end
	
	chats.get_menu_items=function(name)
	
		return chats.chat_to_menu_items(chats.get(name))
	end
	
	chats.get_set=function(s,default_root)
		local root,set=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,set=default_root,s end -- no root use full string as set name
		local sets=(chats.get(root) or {}).sets or {} -- get root sets or empty table
		return sets[set]
	end

	chats.set_set=function(s,v,default_root)
		local root,set=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,set=default_root,s end -- no root use full string as set name

		local chat=chats.get(root)
		if not chat then return end -- unknown chat name
		
		chat.sets=chat.sets or {} -- make sure we have a sets table

-- add inc/dec operators here?
		local t
		if type(v)=="string" then
			t=v:sub(1,1)
		end
		local n=tonumber(v:sub(2))
		if t=="-" and n then
			chat.sets[set]=(tonumber(chat.sets[set]) or 0 ) - n
		elseif t=="+" and n then
			chat.sets[set]=(tonumber(chat.sets[set]) or 0 ) + n
		else
			chat.sets[set]=v
		end
		
		chats.changes(chat,"set",set,v) -- could adjust value of ( chat.sets[set] ) in callback

		return chat.sets[set]
	end

	chats.replace_sets=function(text,default_root)

		if not text then return nil end
--		if not sets then return text end

		local ret=text
		for sanity=0,100 do
			local last=ret
			ret=ret:gsub("{([^}%s]+)}",function(a)
				return chats.get_set(a,default_root) or "{"..a.."}"
			end)
			if last==ret then break end -- no change
		end

		return ret
	end

-- examp[le, replace to build your own menus
	chats.chat_to_menu_items=function(chat)
		local items={cursor=1,cursor_max=0}
		
		items.title=chat.name
		
		local ss=chat.topic and chat.topic.text or {} if type(ss)=="string" then ss={ss} end
		for i,v in ipairs(ss) do
			if i>1 then
				items[#items+1]={text="",chat=chat} -- blank line
			end
			items[#items+1]={text=chat.replace_sets(v)or"",chat=chat}
		end

		for i,v in ipairs(chat.gotos or {}) do

			items[#items+1]={text="",chat=chat} -- blank line before each goto

			local ss=v and v.text or {} if type(ss)=="string" then ss={ss} end

			local color=30
			if chat.viewed[v.name] then color=28 end -- we have already seen the topic to this goto
			
			local f=function(item,menu)

				if item.goto and item.goto.name then

					chats.changes(chat,"goto",item.goto)

					chat.set_topic(item.goto.name)

					chat.set_sets(item.goto.sets)

					menu.show(chats.chat_to_menu_items(chat))

				end
			end
			
			items[#items+1]={text=chat.replace_sets(ss[1])or"",chat=chat,goto=v,cursor=i,call=f,color=color} -- only show first line
			items.cursor_max=i
		end

		return items
	end

-- hook, replace to be notified of changes, by default we print debuging information
	chats.changes=changes or function(chat,change,...)
		local a,b=...

		if     change=="chat"  then print("chat", chat.name,a.name)
		elseif change=="topic" then print("topic",chat.name,a.name)
		elseif change=="goto"  then print("goto", chat.name,a.name)
		elseif change=="set"   then	print("set",  chat.name,a,b)
		end
		
	end

	for n,v in pairs(chats.data) do -- setup each chat
	
		local chat={}
		chats.names[n]=chat -- lookup by name
		chatdown.setup_chat(chat,chats,n,"welcome") -- init state
		
	end

	return chats
end

