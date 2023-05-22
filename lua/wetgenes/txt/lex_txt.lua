--[[#lua.wetgenes.txt.lex_lua

(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

local wtxtwords=require("wetgenes.txt.words")

local wtxtlex=require("wetgenes.txt.lex")

-- add an index map to an array of strings
local keyval=function(t) for i=1,#t do t[ t[i] ]=i end return t end


-- single char state map for output array ( be careful, this must be unique per state )
M.MAP={
	["white"]="w",
	["number"]="0",
	["punctuation"]="p",
	["none"]="n",
	["none_spell"]="N",
	["first"]="f",
}
local MAP=M.MAP


-- these wild card tests for non explicit tokens should all be anchored at start of string by beginning with a ^
M.wild_tokens={
	"^[a-zA-Z]+",							-- a human word ( no numbers )
	"^[ \t]+",								-- multiple tabs or spaces but not newlines
	"^%d*%.?%d*[eE][%+%-]?%d*",				-- floating point number with exponent
	"^%d*%.?%d*",							-- floating point number or int
	"^0x[0-9a-fA-F]+",						-- hex number
}
-- test if a pattern matches entire string
local fullmatch=function(s,p)
	local fs,fe = string.find(s,p)
	if fs==1 and fe==#s then return true end
	return false
end

M.token={}

--M.token.punctuation=keyval{"!","\"","#","$","%","&","'","(",")","*","+",",","-",".","/",":",";","<","=",">","?","@","[","\\","]","^","_","`","","{","|","}","~"}


-- we only need to find lengths of 2 or more that are not matched by the wildcards
M.token_search={}
M.token_max=0
for _,t in pairs(M.token) do
	for i,v in ipairs(t) do
		local found=false
		for _,search in ipairs( M.wild_tokens ) do
			if fullmatch(v,search) then
				found=true
				break
			end
		end
		if not found then -- no need to include if found by wild_tokens search
--print(v)
			M.token_search[v]=true
			if #v>M.token_max then M.token_max=#v end
		end
	end
end


--[[

generate a starting valid state that can be given to parse

]]
M.create=function(state)
	state=state or {}

	state.terminator=state.terminator or {} -- the string we are looking for to terminate the current state (if it is that sort of state)
	state.stack=state.stack or {MAP.first} -- first line is special

	return state
end

--[[

given an input string write an output table of states for each byte any 
changes made to state will be cached and passed back into this function 
again to handle the next line of input

input should be full lines rather than random chunks of string. 

Characters >=128 are considered valid in any strings/comments so utf8 
safe but also ignored, we do not try to be clever.

]]
M.parse=function(state,input,output)

	local push=function(t,v) t[#t+1]=v end
	local poke=function(t,v) local l=#t if l<1 then l=1 end t[l]=v end
	local peek=function(t) return t[#t] end
	local pull=function(t) local v=t[#t] ; t[#t]=nil ; return v end

--	print(input)

	local nexttoken=function(str,start)
		local len=1
		for i=M.token_max,2,-1 do -- longest string has priority
			local s=string.sub(str,start,start+i-1)
			if M.token_search[s] then -- found a match
				len=i
				break
			end
		end
		for _,search in ipairs( M.wild_tokens ) do
			local fs,fe = string.find(str,search,start)
			if fs==start then
				if 1+fe-fs > len then -- only replace if longer match
					len=1+fe-fs
				end
			end
		end
		return string.sub(str,start,start+len-1)
	end


	local outidx=0
	local pump=function(token)
		
		local tokenidx=0
		local last=peek(state.stack)

		local push_output=function(value)
			tokenidx=tokenidx+#token
			outidx=outidx+#token
			while #output < outidx do push(output,value) end
			token=""
		end

		local check_hashbang=function()
			if token=="#!" then -- this is a special comment at start of file
				push(state.terminator,"\n")
				poke(state.stack,MAP.comment)
				return true
			end
			token="" -- do not advance
			poke(state.stack,MAP.white) -- switch to white space
			return true
		end

--[[
		local check_punctuation=function()
			if M.token.punctuation[token] then
				poke(state.stack,MAP.punctuation)
				return true
			end
		end
]]
		
		local check_number=function()
			if fullmatch(token,"^%d+%.?%d*[eE][%+%-]?%d*") then-- float or int with exponent
				poke(state.stack,MAP.number)
				return true
			end
			if fullmatch(token,"^%d+%.?%d*") then-- float or int
				poke(state.stack,MAP.number)
				return true
			end
			if fullmatch(token,"^%.%d+[eE][%+%-]?%d*") then-- floating point number with exponent that starts with .
				poke(state.stack,MAP.number)
				return true
			end
			if fullmatch(token,"^%.%d+") then-- floating point number that starts with .
				poke(state.stack,MAP.number)
				return true
			end
			if fullmatch(token,"^0x[0-9a-fA-F]+") then -- hex
				poke(state.stack,MAP.number)
				return true
			end
		end

		local check_white=function()
			if fullmatch(token,"^%s+") then
				poke(state.stack,MAP.white)
				return true
			end
		end

		local check_spell=function()
			local ok=true
			local s=string.lower(token)
			if #s>1 then -- ignore short words
				if s:match("[^a-z]") then -- ignore if not just letters
					-- ignore
				else
					ok=wtxtwords.check(s) -- check spelling
				end
			end
			return ok
		end
		
		local goto_none=function()
			if token~="" then -- if empty string then no change
				if check_spell() then -- good spelling
					poke(state.stack,MAP.none)
				else -- bad spelling
					poke(state.stack,MAP.none_spell)
				end
			end
			return true -- but always return true
		end
		

		local check_list={
			[MAP.white]={
				check_number,
				check_white,
--				check_punctuation,
				goto_none,
			},
			[MAP.punctuation]={
				check_number,
				check_white,
--				check_punctuation,
				goto_none,
			},
			[MAP.number]={
				check_number,
				check_white,
--				check_punctuation,
				goto_none,
			},
			[MAP.none]={
				check_number,
				check_white,
--				check_punctuation,
				goto_none,
			},
			[MAP.none_spell]={
				check_number,
				check_white,
--				check_punctuation,
				goto_none,
			},
			[MAP.first]={
				check_hashbang,
			},
		}

		local check=check_list[last] or { goto_none }
		local done=false
		local checkidx=1
		while not done do done=(check[checkidx])() ; checkidx=checkidx+1 end

		push_output( peek(state.stack) )

		return tokenidx
	end
	
	local i=1
	local len=#input
	while i<=len do
		i=i+pump( nexttoken(input,i) ) -- advance a number of characters
	end

	return output

end
