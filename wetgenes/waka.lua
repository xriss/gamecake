--
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--

--
-- a waka is made of chunks
-- chunks are made of text that contains links
-- a word that begins with / or http: is a link
-- there is no waka markup, if you want markup then use xhtml
--

local table=table
local string=string


local ipairs=ipairs
local pairs=pairs

local string=string
local type=type
local tostring=tostring

local require=require
local loadstring=loadstring
local setfenv=setfenv
local setmetatable=setmetatable
local pcall=pcall
local error=error

-- my string functions
local str=require("wetgenes.string")
local sbox=require("wetgenes.sandbox")


module("wetgenes.waka")

local split_lines		=str.split_lines
local split_words		=str.split_words
local split_whitespace	=str.split_whitespace
local split_equal		=str.split_equal

-----------------------------------------------------------------------------
--
-- turn a string into a tag array
--
-----------------------------------------------------------------------------
function text_to_tags(text)
	local r={}
	
	if text then
		if string.find(text,",") then -- coma seperated
			local a=split_words(text,",")
			for i,v in ipairs(a) do
				local s=str.trim(v)
				if #s>0 then r[s]=true end
			end
		else -- white space seperated
			local a=split_words(text)
			for i,v in ipairs(a) do
				local s=str.trim(v)
				if #s>0 then r[s]=true end
			end
		end
	end
	
	return r
end

-----------------------------------------------------------------------------
--
-- take some text and break it into named chunks
-- returns a lookup table of chunks and numerical list of these chunks in the order they where first defined
-- body is the default chunk name
--
-- a chunk is a line that begins with #
-- the part after the # and ending with whitespace is the chunk name
-- all text following this line is part of that chunk
-- the default section if none is give is "body", so any whitespace at the start of the file
-- before the first # line will be assigned to this chunk
-- data may follow this chunk name, if multiple chunks of the same name
-- are defined they are simple merged into one
-- and each #chunk line is combined into one chunk data
--
-- use option=value after the section name to provide options, so somthing like this
--
-- #name opt=val opt=val opt=val
-- # opt=val
-- here is some text
-- # opt=val
-- here is some more text
-- ## special comment, this line is ignored
-- ## comments are just a line that begins with two hashes
--
-- is a valid chunk, all of the opt=val will be assigned to the same chunk
-- and all the other text will be joined as that chunks body
--
-- pass in chunks and you can merge multiple texts into one chunk
--
-----------------------------------------------------------------------------
function text_to_chunks(text,chunks)

local chunkend -- special end of chunk test

	chunks=chunks or {}

	local function manifest_chunk(line,oldchunk)
		local opts=split_words( line:sub(2) ) -- skip # at start of line
		local name=string.lower( opts[1] or "body" )
		local chunk
		local c2=line:sub(2,2)
				
		if c2:find("%s") then -- if first char after # is whitespace, then use the old chunk 
			chunk=oldchunk
		end
		
		if not chunk then
			chunk=chunks[name] -- do we already have this chunk?
		end
		
		if chunk then -- update an old chunk
		
			for i=1,#opts do local v=opts[i]
				table.insert( chunk.opts , v ) -- add extra opts
				local a,b=split_equal(v)
				if a then chunk.opts[a]=b end
			end
			
		else -- create a new chunk
		
			chunk={} -- make default chunk

-- set some default options depending on the chunk name

			if name:sub(1,4)=="body" then -- all chunks begining with "body" are waka format by default
				opts.form="waka"
			end

			if name:sub(1,5)=="title" then -- all chunks begining with "title" are trimed by default
				opts.trim="ends"
			end

			if name:sub(1,3)=="css" then -- all chunks begining with "css" append children by default
				opts.append="on"
			end
		
			if name:sub(1,3)=="lua" then -- all chunks begining with "lua" are lua code by default
				opts.form="lua"
			end
			if name:sub(1,4)=="opts" then -- all chunks begining with "opts" are also lua code by default
				opts.form="opts"
			end
			
-- the actual options will overide the defaults

			for i=1,#opts do local v=opts[i]
				local a,b=split_equal(v)
				if a then opts[a]=b end
			end
			
			chunk.id=#chunks+1
			chunk.name=name
			chunk.opts=opts
			chunk.lines={}
			
			chunks[chunk.id]=chunk		-- save chunk in chunks as numbered id
			chunks[chunk.name]=chunk	-- and as name
		end
		
		return chunk
	end
		
	local lines=split_lines(text)
	
	local chunk
	
	for i=1,#lines do local v=lines[i] -- ipairs
		
		local c=v:sub(1,1) -- the first char is special
		
		if c=="#" then -- start of chunk
		
			if chunkend then -- waiting for special end everything is inserted
			
				if chunkend==v:sub(1,#chunkend) then -- got it
					chunkend=nil
				else
					if not chunk then chunk=manifest_chunk("#body") end --sanity				
					table.insert(chunk.lines , v)
				end
				
			else

				if "#[["==v:sub(1,3) then -- special open
				
					chunkend="#]]"..v:sub(4) -- any special hash we need to close
				
				elseif v:sub(2,2)~="#" then -- skip all comments

					chunk=manifest_chunk(v,chunk)

				end
				
			end
			
		else -- normal lime add to the current chunk
		
			if not chunk then chunk=manifest_chunk("#body") end --sanity
			
			table.insert(chunk.lines , v)
		end
	
	end
	
	for i=1,#chunks do local v=chunks[i] -- perform some final actions on all chunks
	
		v.text=table.concat(v.lines) -- merge the split lines back together into one string
		
	end
	
	return chunks
	
end

-----------------------------------------------------------------------------
--
-- merge source data into dest data, dest data may be nil in which case this 
-- works like a copy. Return the dest chunk. It is intended that you have a
-- a number of chunks and then merge them together into a final data chunk
-- using this function, the first merge creates a new dest chunk. the final result
-- will have a new ordering depending on the merged chunks but the numerical array
-- can still be used to loop through chunks
--
-----------------------------------------------------------------------------
function chunks_merge(dest,source)

	local dest=dest or {}
	
	local locked=dest.opts and dest.opts.opts.lock=="on" -- parent chunk locked
			
	for i,v in ipairs(source) do
	
		local c=dest[v.name] -- merge or
		
		local function set_data()
			if c.opts.append=="on" then -- add new lines to the end of the chunk rather than replace
				c.text=(c.text or "") .. ( v.text or "" )
				c.lines=split_lines(c.text) -- also need to build lines?
			else -- just replace
				c.lines=v.lines -- set or override
				c.text=v.text -- set or override
			end
			for ii,vv in pairs(v.opts) do
				c.opts[ii]=vv
			end
		end
		
		if not c then -- make a new chunk
			c={}
			c.id=#dest+1
			c.name=v.name
			c.opts={}
			dest[c.id]=c -- link it into dest by array
			dest[c.name]=c -- and by name
			
			set_data()
		else
			if not locked then -- skip lock
				set_data()
			end
		end

	end

	return dest
end


-----------------------------------------------------------------------------
--
-- get a html given some simple waka text
--
-- \n are turned into <br/> tags
-- and words that look like links are turned into links
-- any included html should get escaped so this is "safe" to use on user input
--
-- aditional opts
--
-- we need to know the base_url of this page when building links, if this is not given
-- then relative links may bork?
--
-- setting escape_html to true prevents any html from getting through
--
-----------------------------------------------------------------------------
function waka_to_html(input,opts)
	opts=opts or {}

local base_url=opts.base_url or ""
local escape_html=opts.escape_html or false

	local r={}
	local esc
	if escape_html then -- simple html escape
		esc=function(s) 
			local escaped = { ['<']='&lt;', ['>']='&gt;', ["&"]='&amp;' , ["\n"]='<br/>\n' }
			return (s:gsub("[<>&\n]", function(c) return escaped[c] or c end))
		end
	else -- no escape just convert \n to <br/>
		esc=function(s) 
			local escaped = { ["\n"]='<br/>\n' }
			return (s:gsub("[\n]", function(c) return escaped[c] or c end))
		end
	end
	
	local function link( url , str )
		table.insert(r,"<a href=\""..esc(url).."\">"..esc(str).."</a>")
	end
	local function text( str )
		table.insert(r,esc(str))
	end

	local tokens=split_whitespace(input)
	
	for i2=1,#tokens do local token=tokens[i2]
	
		local done=false
		
		local len=token:len()
		
		if len>=2 then -- too short to be a link
		
			local c1=token:sub(1,1) -- some chars to check
			
			if c1 == "/" then -- a very simple link relative to where we are
			
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
					local ss=split_words(s1,"/")
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
	
	return table.concat(r)
end

-----------------------------------------------------------------------------
--
-- turn some chunks into their prefered form, escape, trim and expand
--
-- i need a naming convention that make sense so this is now calledm
-- refine_chunks with coarse meaning unrefined
--
-----------------------------------------------------------------------------
function form_chunks(srv,chunks,opts) return refine_chunks(srv,chunks,opts) end
function refine_chunks(srv,chunks,opts)

	opts=opts or {}

	local refined=opts.refined or {} -- can pass in a table to refine into

	refined.opts=refined.opts or {}

-- start by compiling any lua chunks and running its pageopts hook
	for i,v in ipairs(chunks) do
	
		if v.opts.form=="lua" or v.opts.form=="opts" then -- we have some lua code for this page
			local a,b=pcall( function() return sbox.ini(v.text) end )
			if a then v.env=b end -- success
			if not a then v.text=b end -- fail, set text to error text
	
			if v.env and v.env.hook_opts then
				local a,b = pcall(function() v.env.hook_opts(refined.opts) end) -- update pageopts?
			end

		end
		
	end

	
	for i,v in ipairs(chunks) do -- do basic process of all of the page chunks into their prefered form 
		local s=v.env or v.text
		
		local format=v.opts.form
		local trim=v.opts.trim
		
		if trim=="ends" then s=str.trim(s) end -- trim whitespace, useful for one word chunks?

		if format=="nohtml" then -- like normal but all html is escaped

			s=waka_to_html(s,{base_url=opts.baseurl,escape_html=true}) 

		elseif format=="import" then -- very special import, treat as chunk of lua import opts/code
		
			local e=sbox.make_env()
			local f,err=loadstring(s)
			if f then
				setfenv(f, e)
				local a,b=pcall(f)
				if not a then s=b e={} end -- error running chunk, set chunk to error string and clear chunk data
			else
				s=err
			end

-- this should be split off into a plugin system...
			
			if e.import=="dimeload" then -- include some dimeload info
			
				e.hook   	= e.hook 	  or opts.hook
				e.command   = e.command   or opts.command
				e.plate  	= e.plate 	  or opts.plate
				
				if not opts.nodimeload then -- prevent recursions
					local dl=require("dimeload")
					s=dl.chunk_import(srv,e)
				end
				
			elseif e.import=="blog" then
			
				e.hook   = e.hook   or opts.hook
				e.limit=e.limit or 5
				e.layer=e.layer or 0
				e.sort=e.sort or "pubdate"
				
				if not opts.noblog then -- prevent recursions
					local blog=require("blog")
					s=blog.chunk_import(srv,e)
				end
				
			elseif e.import=="note" then
				e.hook   = e.hook   or opts.hook
				e.limit=e.limit or 5
				e.sort=e.sort or "pubdate"
				
				if not opts.nonote then -- prevent recursions
					local note=require("note")
					s=note.chunk_import(srv,e)
				end
				
			elseif e.import=="comic" then
			
				e.hook   = e.hook   or opts.hook
				e.limit=e.limit or 5
				e.sort=e.sort or "pubdate"
				
				if not opts.nocomic then -- prevent recursions
					local comic=require("comic")
					s=comic.chunk_import(srv,e)
				end
				
			elseif e.import=="gsheet" then -- we need to grab some json from google
			
				local gsheet=require("waka.gsheet")
				e.offset = e.offset or opts.offset -- can choose new pages
				e.limit  = e.limit  or opts.limit -- can choose new pages
				e.query  = e.query  or opts.query
				e.plate  = e.plate  or opts.plate
				e.key    = e.key    or opts.key
				e.hook   = e.hook   or opts.hook -- callback function to fixup data
				s=gsheet.getwaka(srv,e) -- get a string
				
			elseif e.import=="csv" then -- we need to grab some spreadsheet data from somewhere
			
				local gsheet=require("waka.csv")
				e.random = e.random or opts.random -- import this many random values please
				e.offset = e.offset or opts.offset -- import values starting at
				e.limit  = e.limit  or opts.limit  -- and only this many
				e.url    = e.url    or opts.url    -- where to get data from
				e.plate  = e.plate  or opts.plate  -- how to render
				e.hook   = e.hook   or opts.hook   -- callback function to fixup data
				s=gsheet.chunk_import(srv,e) -- return a table to render
				
				if e.hook then -- call hook to fix table and return it
					s=e.hook(s)
				end

			elseif e.import=="wikipedia" then -- we need to import some xml from wikipedia
			
				local wikipedia=require("waka.wikipedia")
				e.offset = e.offset or opts.offset -- can choose new pages
				e.limit  = e.limit  or opts.limit -- can choose new pages
				e.name  = e.name  or opts.name -- get this page
				e.search  = e.search  or opts.search -- search for
				e.hook   = e.hook   or opts.hook -- callback function to fixup data
				
				s=wikipedia.getwaka(srv,e) -- get a string
				
			elseif e.import=="picasa" then -- we need to import some imagedata from picasssa
			
				local picassa=require("waka.picasa")
				e.offset = e.offset or opts.offset -- can choose new pages
				e.limit  = e.limit  or opts.limit -- can choose new pages
				e.user  = e.user  or opts.user -- user name
				e.album  = e.album  or opts.album -- album name
				e.authkey  = e.authkey  or opts.authkey -- authkey if needed
				e.hook   = e.hook   or opts.hook -- callback function to fixup data
				e.plate   = e.plate   or opts.plate -- display plate
				
				s=picassa.getwaka(srv,e) -- get a string or tab
				
			elseif e.import=="json" then -- we need to import some json from somewhere
			
				local waka_json=require("waka.json")
				e.url  = e.url  or opts.url -- what to get
				e.cachetime  = e.cachetime  or opts.cachetime -- how long to cache for
				e.hook   = e.hook   or opts.hook -- callback function to fixup data
				e.plate   = e.plate   or opts.plate -- display plate
				
				s=waka_json.getwaka(srv,e) -- get a string or tab
			
			else -- raw
			
				if e.chunk then
					local chunk=str.table_lookup(e.chunk,refined)
--error(str.dump(chunk))
					if chunk then
						local meta={__index=chunk}
						setmetatable(e, meta)
					end
				end

				s=e
			
			end
		
		elseif format=="waka" then -- basic waka format, html allowed but links are upgraded and line ends are <br/>

			s=waka_to_html(s,{base_url=opts.baseurl,escape_html=false})

		end

		if v.opts.form=="opts" then -- opts is a special chunk, full of, opts so we merge all values
			refined[v.name]=refined[v.name] or {}
			for n,s in pairs(v.opts) do
				refined[v.name][n]=s
			end
			if type(v.env)=="table" then
				for n,s in pairs(v.env) do
					refined[v.name][n]=s
				end
			end
		else
			refined[v.name]=s
		end
		

	end
	
-- end by running any refined lua hooks
	for i,v in ipairs(chunks) do
--		if v.opts.form=="lua" then -- we have some lua code for this page
		if v.env and v.env.hook_refined then
			pcall(function() v.env.hook_refined(refined) end) -- update refined data
		end
--		end
	end

	
	return refined
end
