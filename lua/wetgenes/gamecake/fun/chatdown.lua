--
-- (C) 2017 kriss@wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local chatdown=M

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.parse

	rawsubjects = chatdown.parse(text)

Parse text from flat text chatdown format into heirachical chat data, 
which we refer to as rawsubjects, something that can be output easily 
as json.

This gives us a readonly rawsubjects structure that can be used to control 
what text is displayed during a chat session.

This is intended to be descriptive and logic less, any real logic 
should be added using a real language that operates on this rawsubjects and 
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

	local subjects={}
	local subject={}

	local gotos={}
	local goto={}

	local topics={}
	local topic={}

	local tags={}

	local ignore=true -- ignore first lines until we see a code

	for i,v in ipairs(lines) do

		local name

		local code=v:sub(1,1)

		if code=="#" then -- #subject begin new subject
			ignore=false -- end long comments

			local c=v:sub(2,2)
			
			if c=="#" or c=="<" or c=="=" or c==">" or c=="-" then -- escape codes

				v=v:sub(2) -- just remove hash from start of line
			
			else

				name,v=v:match("%#(%S*)%s*(.*)$")

				text={}
				topics={}
				gotos={}
				tags={}
				subject={text=text,gotos=gotos,tags=tags,topics=topics,name=name}

				if name~="" then -- ignore empty names
				
					assert( not subjects[name] , "subject name used twice on line "..i.." : "..name )
					subjects[name]=subject
				
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
			tags={}
			topic={text=text,name=name,gotos=gotos,tags=tags}

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
			tags={}
			goto={text=text,name=name,tags=tags}

			gotos[#gotos+1]=goto

		elseif code=="=" then -- =tag
		
			name,v=v:match("%=(%S*)%s*(.*)$")
			
			if v and v~="" then -- a single line assignment, no state change
			
				ignore=false -- end long comments

				assert( not tags[name] , "tag name used twice on line "..i.." : "..name )
				tags[name]={v}
				
				v=nil

			else

				ignore=false -- end long comments

				text={}

				assert( not tags[name] , "tag name used twice on line "..i.." : "..name )
				tags[name]=text
			
			end
			
		elseif code=="-" then -- -comment
		
			if v:sub(1,2)=="--" then -- a multi-line long comment
				ignore=true -- ignore many lines until the next code
			else
				v=nil	-- just ignore this line
			end
			
		end
		
		if v and v~="" and not ignore then

			text[#text+1]=v

		end

		
	end

	-- cleanup output

	local cleanup_tags=function(tags)

		local empty=true
		for n,v in pairs(tags) do
			empty=false
			tags[n]=table.concat(v,"\n"):match("^%s*(.-)%s*$")
		end
		if empty then return end

		return tags
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


	for name,subject in pairs(subjects) do

		subject.text=cleanup_text(subject.text)
		subject.tags=cleanup_tags(subject.tags)

		for id,goto in pairs(subject.gotos) do

			goto.text=cleanup_text(goto.text)
			goto.tags=cleanup_tags(goto.tags)
		end

		for id,topic in pairs(subject.topics) do

			topic.text=cleanup_text(topic.text)
			topic.tags=cleanup_tags(topic.tags)

			for id,goto in pairs(topic.gotos) do

				goto.text=cleanup_text(goto.text)
				goto.tags=cleanup_tags(goto.tags)
			end
		end

	end

	return subjects

end



-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.setup_chat

	chat = chatdown.setup_chat(chats,subject_name)

Setup the initial chat state for a subject. This is called 
automatically by chatdown.setup and probably should not be used 
elsewhere.

]]
-----------------------------------------------------------------------------
chatdown.setup_chat=function(chats,subject_name)

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
	
	local chat={}
	
	chat.chats=chats -- parent
	chat.subject_name=subject_name
	chat.tags={}
	chat.viewed={}
	
	chat.get_tag=function(text)
		return chats.get_tag(text,chat.subject_name)
	end
	
	chat.set_tag=function(text,val)
		return chats.set_tag(text,val,chat.subject_name)
	end

	chat.replace_tags=function(text)
		return chats.replace_tags(text,chat.subject_name)
	end

	chat.set_tags=function(tags)
		for n,v in pairs(tags or {}) do
			chat.set_tag(n,v)
		end
    end
	
	chat.subject={}
	chat.topics={}

	chat.set_topic=function(name)
	
		chat.viewed[name]=(chat.viewed[name] or 0) + 1 -- keep track of what topics have been viewed
	
		chat.topic_name=name
		chat.topic={}
		chat.gotos={}
		
		local merged_tags={}

		local goto_names={} -- keep track of previously seen exit nodes

		for n in dotnames(name) do -- inherit topics data
			local v=chat.topics[n]
			if v then
				for n2,v2 in pairs(v) do -- merge base settings
					chat.topic[n2]=chat.topic[n2] or v2
				end 
				for np,vp in pairs(v.tags or {}) do -- merge set changes
					merged_tags[np]=merged_tags[np] or vp
				end
				for n2,v2 in ipairs(v.gotos or {}) do -- join all gotos
					local r={}
					for n3,v3 in pairs(v2) do r[n3]=v3 end -- copy

					if not r.text then -- use text from subject prototype gotos
						for i,p in ipairs(chat.subject.gotos or {} ) do -- search
							if r.name==p.name then r.text=p.text break end -- found and used
						end
					end
					
					local result=true
					if r.name:find("?") then -- query string
						r.name,r.query=r.name:match("(.+)?(.+)")
						
						local t={}
						r.query:gsub("([^&|!=<>]*)([&|=<>!]*)",function(a,b) if a~="" then t[#t+1]=a end if b~="" then t[#t+1]=b end end)
						
						local do_test=function(a,b,c)

							local a=chat.get_tag(a)

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
					
					r.name=chat.replace_tags(r.name) -- can use tags in name
					
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

		chat.set_tags(merged_tags)

	end

	for n in dotnames(subject_name) do -- inherit subjects data
		local v=chats.rawsubjects[n]
		if v then
			for n2,v2 in pairs(v) do -- merge base settings into subject table
				chat.subject[n2]=chat.subject[n2] or v2
			end 
			for n2,v2 in pairs(v.topics or {}) do -- merge topics
				chat.topics[n2]=chat.topics[n2] or v2
			end
		end
	end

--	chat.set_topic( chat.get_tag("welcome") or "welcome") -- {welcome} or welcome is the first topic of conversation?
	
	return chat
end


-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.setup

	chats = chatdown.setup_chats(chat_text,changes)

parse and initialise state data for every subjects chunk creating a 
global chats with a chat object for each subject.

]]
-----------------------------------------------------------------------------
chatdown.setup_chats=function(chat_text,changes)

	local chats={}

	chats.rawsubjects=chatdown.parse(chat_text) -- parse static data
	
	chats.subject_names={}
	
	chats.set=function(subject_name)
		chats.subject_name=subject_name
		chats.changes(chats.get(),"subject")
		return chats.get()
	end

	chats.get=function(subject_name)
		return chats.subject_names[subject_name or chats.subject_name]
	end
	
	chats.get_tag=function(s,default_root)
		local root,tag=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,tag=default_root,s end -- no root use full string as tag name
		local tags=(chats.get(root) or {}).tags or {} -- get root tags or empty table
		return tags[tag]
	end

	chats.set_tag=function(s,v,default_root)
		local root,tag=s:match("(.+)/(.+)") -- is a root given?
		if not root then root,tag=default_root,s end -- no root use full string as tag name

		local chat=chats.get(root)
		if not chat then return end -- unknown chat name
		
		chat.tags=chat.tags or {} -- make sure we have a tags table

-- add inc/dec operators here?
		local t
		if type(v)=="string" then
			t=v:sub(1,1)
		end
		local n=tonumber(v:sub(2))
		if t=="-" and n then
			chat.tags[tag]=(tonumber(chat.tags[tag]) or 0 ) - n
		elseif t=="+" and n then
			chat.tags[tag]=(tonumber(chat.tags[tag]) or 0 ) + n
		else
			chat.tags[tag]=v
		end
		
		chats.changes(chat,"tag",tag,v) -- could adjust value of ( chat.tags[tag] ) in callback

		return chat.tags[tag]
	end

	chats.replace_tags=function(text,default_root)

		if not text then return nil end

		local ret=text
		for sanity=0,100 do
			local last=ret
			ret=ret:gsub("{([^}%s]+)}",function(a)
				return chats.get_tag(a,default_root) or "{"..a.."}"
			end)
			if last==ret then break end -- no change
		end

		return ret
	end

-- hook, replace to be notified of changes, by default we print debuging information
-- replace with your own empty changes hook to prevent this
	chats.changes=changes or function(chat,change,...)
		local a,b=...
		
		if     change=="subject" then print( "subject" , chat.subject_name              )
		elseif change=="topic"   then print( "topic"   , chat.subject_name , a.name     )
		elseif change=="goto"    then print( "goto"    , chat.subject_name , a.name     )
		elseif change=="tag"     then print( "tag"     , chat.subject_name , a      , b )
		end
		
	end

	for n,v in pairs(chats.rawsubjects) do -- setup each chat
		chats.subject_names[n]=chatdown.setup_chat(chats,n) -- init state
		if not chats.subject_name then chats.set(n) end -- default to the first subject
	end

	for n,chat in pairs(chats.subject_names) do -- set starting tag values
		chat.set_tags(chat.subject.tags)
	end

	return chats
end

-----------------------------------------------------------------------------
--[[#lua.wetgenes.gamecake.fun.chatdown.text

Here is some example chatdown formatted text full of useful 
information, it it is intended to be a self documented example.

```

- This is a single line comment
-- This is the start of a multi-line comment

All lines are now comment lines until we see a line that begins with a 
control character leading white space is ignored. If for some reason 
you need to start a text line with a special character then it can be 
escaped by preceding it with a #

What follows is a list of these characters and a brief description 
about the parser state they switch to.

	1. - (text to ignore)
	
		A single line comment that does not change parser state and 
		only this line will be ignored so it can be inserted inside 
		other chunks without losing our place.

	2. -- (text to ignore)
	
		Begin a comment chunk, this line and all lines that follow this 
		line will be considered comments and ignored until we switch to 
		a new parser state.

	3. #subject_name

		Begin a new subject chunk, all future topic,goto or tag chunks will 
		belong to this subject.
		
		The text that follows this until the next chunk is the long 
		description intended for when you examine the character.
		
		Although it makes sense for a subject chunk to belong to one 
		character it could also a group conversation with tags being 
		set to change the current talker as the conversation flows.
		
		subject names have simple cascading inheritance according to their 
		name with each level separated by a dot. A chunk named a.b.c 
		will inherit data from any chunks defined for a.b and a in that 
		order of priority.

	4. >topic_name
	
		Begin a topic chunk, all future goto or tag chunks will belong 
		to this topic, the lines that follow are how the character 
		responds when questioned about this topic followed by one or 
		more gotos as possible responses that will lead you onto 
		another topic.
		
		Topics can be broken into parts, to create a pause, by using an 
		unnamed goto followed by an unnamed topic which will both 
		automatically be given the same generated name and hence linked 
		together.
		
	5. <goto_topic_name
	
		Begin a goto chunk, all future tag chunks will belong to this 
		goto, this is probably best thought of as a question that will 
		get a reply from the character. This is a choice made by the 
		player that causes a logical jump to another topic.
		
		Essentially this means GOTO another topic and there can be 
		multiple GOTO options associated with each topic which the 
		reader is expected to choose between.
		
	6. =set_tag_name to this value
	
		If there is any text on the rest of this line then it will be 
		assigned to the tag without changing the current parse state so 
		it can be used in the middle of another chunk without losing 
		our place.
		
		This single line tag assignment is usually all you need. 
		Alternatively, if there is no text on the rest of this first 
		line, only white space, then the parse state will change and 
		text on all following lines will be assigned to the named tag.
		
		This assignment can happen at various places, for instance if 
		it is part of the subject then it will be the starting 
		value for a tag but if it is linked to a topic or goto then the 
		value will be a change as the conversation happens. In all 
		cases the tags are set in a single batch as the state changes 
		so the placement and order makes no difference.
		
		Tags can be used inside text chunks or even GOTO labels by 
		tightly wrapping with {} eg {name} would be replaced with the 
		value of name. Tags from other subjects can be referenced by 
		prefixing the tag name with the subject name followed by a forward 
		slash like so {subject/tag}
				

The hierarchy of these chunks can be expressed by indentation as all 
white space is ignored and combined into a single space. Each SUBJECT will 
have multiple TOPICs associated with it and each TOPIC will have 
multiple GOTOs as options to jump between TOPICs. TAGs can be 
associated with any of these 3 levels and will be evaluated as the 
conversation flows through these points.

So the chunk hierarchy expressed using indentation to denote children 
of.

	SUBJECT
		TAG
		GOTO
			TAG
		TOPIC
			TAG
			GOTO
				TAG

The GOTO chunks in the root SUBJECT chunk are used as prototypes so if a 
GOTO is used in multiple topics its text can be placed within a GOTO 
inside the main SUBJECT chunk rather than repeated constantly. This will 
then be inherited by a GOTO with no text. An alternative to this 
shorthand is to assign an oft-used piece of text to a tag and reference 
that in each topic instead.

SUBJECTs and TOPICs also have simple inheritance based on their names this 
enables the building of a prototype which is later expanded. Each 
inheritance level is separated by a dot so aa.bb.cc will inherit from 
aa.bb and aa with the data in the longer names having precedence. This 
inheritance is additive only so for instance a TAG can not be unset and 
would have to be changed to another value by aa.bb.cc if it existed in 
aa.bb or aa.

In practise this means


```
]]
-----------------------------------------------------------------------------
