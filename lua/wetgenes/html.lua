--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require


-- my string functions
local wstr=require("wetgenes.string")

module("wetgenes.html")

--
-- use the replace function from wetgenes.string
--
replace=wstr.replace


-----------------------------------------------------------------------------
--
-- build a string from a template,  with a table to be used as its environment
--
-- this environment will not get modified by the called function as it is wrapped here
--
-- even though the calling function is free to modify the table it gets
--
-----------------------------------------------------------------------------
get=function(html,src,env)

	local new_env={}
	if env then setmetatable(new_env,{__index=env})	end -- wrap to protect

	if html[src] then src=html[src] end
	
	if type(src)=="function" then return src(new_env) end
	
	if type(src)=="string" and env then return replace(src,new_env) end

	return tostring(src)
end


-----------------------------------------------------------------------------
--
-- very basic html esc to stop tags and entities from doing bad things
-- running text submitted from a user through this function should stop it from doing
-- anything other than just being text, it doesnt guarantee that it is valid xhtml / whatever
-- We just turn a few important characters into entities.
--
-----------------------------------------------------------------------------
function esc(s)
	local escaped = { ['<']='&lt;', ['>']='&gt;', ["&"]='&amp;' }
	return (s:gsub("[<>&]", function(c) return escaped[c] end))
end

-----------------------------------------------------------------------------
--
-- basic url escape, so as not to trigger url get params or anything else by mistake 
-- so = & # % ? " ' are bad and get replaced with %xx
--
-----------------------------------------------------------------------------
function url_esc(s)
	return string.gsub(s, "([&=%%%#%?%'%\" ><])", function(c)
		return string.format("%%%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- a url escape, that only escapes the string deliminators ' and " 
--
-----------------------------------------------------------------------------
function url_esc_string(s)
	return string.gsub(s, "(['%\" ])", function(c)
		return string.format("%%%02x", string.byte(c))
	end)
end

-----------------------------------------------------------------------------
--
-- convert any %xx into single chars
--
-----------------------------------------------------------------------------
function url_unesc(s)
	return string.gsub(s, "%%(%x%x)", function(hex)
		return string.char(tonumber(hex, 16))
	end)
end



-----------------------------------------------------------------------------
--
-- get a html output given some simple waka style text
--
-- \n are turned into <br/> tags
-- and words that look like links are turned into links
-- any included html should get escaped so this is "safe" to use on user input
-- any indented paragraphs will have the indention removed and will be
-- treated as code / preformated text.
--
-- aditional opts
--
-- we need to know the base_url of this page when building links, if this is not given
-- then relative links may bork?
--
-- setting escape_html to true prevents any html from getting through by escaping special characters
--
-----------------------------------------------------------------------------
function waka_to_html(input,opts)
	opts=opts or {}

local base_url=opts.base_url or ""
local escape_html=opts.escape_html or false

	local r={}
	local esc_html
	
	if escape_html then -- simple html escape
		esc_html=function(s) 
			local escaped = { ['{']='&#123;',['}']='&#125;',['<']='&lt;', ['>']='&gt;', ["&"]='&amp;' }
			return (s:gsub("[{}<>&]", function(c) return escaped[c] or c end))
		end
	else -- no escape just convert \n to <br/>
		esc_html=function(s) return s end
	end
	
	local esc_br=function(s)
		local escaped = { ["\n"]='<br/>\n' }
		return (s:gsub("[\n]", function(c) return escaped[c] or c end))
	end

	local esc=function(s)
		return esc_br(esc_html(s))
	end

	local function link( url , str )
		table.insert(r,"<a href=\""..esc(url).."\">"..esc(str).."</a>")
	end
	local function text( str )
		table.insert(r,esc(str))
	end
	local function code( str )
		table.insert(r,esc_html(str))
	end


	local lines=wstr.split_lines(input)
	
	for _,line in ipairs(lines) do

		local tokens=wstr.split_whitespace(line)
		
		for i2=1,#tokens do local token=tokens[i2]
		
			local done=false
			
			local len=token:len()
			
			if len>=2 then -- too short to be a link
			
				local c1=token:sub(1,1) -- some chars to check
				
				if c1 == "/" and (not opts.no_slash_links) then -- a very simple link relative to where we are
				
					local chars="[%w/%-%+_#%.:]+"
					
	--				if token:sub(1,3)=="///" then chars="[%w/%-%+_#%.:]+" end -- allow common domain chars
				
					local s=token:sub(2) -- skip this first char
					
					local f1,f2=s:find(chars)
					if f1 then -- must find a word
						local s1=s:sub(f1,f2)
						if s1:sub(-1)=="." then -- trim trailing dot
							f2=f2-1
							s1=s:sub(f1,f2)
						end
						local ss=wstr.split_words(s1,"/")
						local tail=ss[#ss] 
						link(s1,tail or s1)
						if f2<s:len() then -- some left over string
							text(s:sub(f2+1))
						end
						done=true
					end
					
				elseif token:sub(1,7)=="http://" then
						link(token,token)
						done=true

				elseif token:sub(1,8)=="https://" then
						link(token,token)
						done=true
				end
				
			end
			
			
			if not done then -- unhandled token, just add it
				text(token)				
			end
		
		end
		
	end
	
	return table.concat(r)
end


