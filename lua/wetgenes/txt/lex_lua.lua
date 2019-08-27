
--
-- (C) 2019 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M


local keyval=function(t) for i=1,#t do t[ t[i] ]=i end return t end

M.token_all=keyval{"and","break","do","else","elseif","end","false","for","function","if","in","local","nil","not","or","repeat","return","then","true","until","while",
"-","+","*","/","%","^","=","#",";",":",",",".","..","...","<",">","==","<=",">=","~=","--","--[[","]]","\\\"","[","]","(",")","{","}","--[","'","\""," ","\t","\n","\r"}
M.token_max=0
for i,v in ipairs(M.token_all) do if #v>M.token_max then M.token_max=#v end end

M.token_keyword=keyval{"and","break","do","else","elseif","end","false","for","function","if","in","local","nil","not","or","repeat","return","then","true","until","while"}
M.token_punctuation=keyval{"<",">","==","<=",">=","~=","-","+","*","/","%","^","=","#",";",":",",",".","..","...","(",")","[","]","{","}"}
M.token_white=keyval{" ","\t","\n","\r"}

--[[

generate a starting valid state that can be given to parse

]]
M.create=function(state)
	state=state or {}

	state.terminator=state.terminator or {} -- the string we are looking for to terminate the current state (if it is that sort of state)
	state.stack=state.stack or {"white"} -- start from a whitespace state

	return state
end

--[[

given an input string write an output table of states for each byte any 
chnages made to state will be cached and passed back into this function 
again to handle the next line of input

input may be a full line or a partial to generate the state at a given 
point

Characters >=128 are considered valid in any strings/comments so utf8 
safe but also ignored, we do not try to be clever.

]]
M.parse=function(state,input,output)

	local push=function(t,v) t[#t+1]=v end
	local poke=function(t,v) local l=#t if l<1 then l=1 end t[l]=v end
	local peek=function(t) return t[#t] end
	local pull=function(t) local v=t[#t] ; t[#t]=nil ; return v end

--	print(input)

	local istoken=function(str,b)
		for i=M.token_max,1,-1 do -- longest string has priority
			local s=string.sub(str,b,b+i-1)
			if M.token_all[s] then return s end -- found a match
		end
	end

	local idx=0

	local pump=function(token)

		local last=peek(state.stack)

		local push_output=function(value)
			idx=idx+#token
			while #output < idx do push(output,value) end
			token=""
		end

		local check_keyword=function()
			if M.token_keyword[token] then
				poke(state.stack,"keyword")
				return true
			end
		end
		
		local check_punctuation=function()
			if M.token_punctuation[token] then
				poke(state.stack,"punctuation")
				return true
			end
		end
		
		local check_white=function()
			if M.token_white[token] then
				poke(state.stack,"white")
				return true
			end
		end
		
		local check_last=function()
			if token~="" then -- if empty string then no change
				poke(state.stack,"none")
			end
			return true -- but always return true
		end
		
		local check_string=function()
			if last=="string" then
				if token==peek(state.terminator) then 
					pull(state.terminator)
					push_output("string")
					poke(state.stack,"punctuation")
					return true
				end
				return true -- we are trapped in a string
			elseif token=="\"" then 
				push(state.terminator,"\"")
				poke(state.stack,"string")
				return true
			elseif token=="'" then 
				push(state.terminator,"'")
				poke(state.stack,"string")
				return true
			elseif token=="[[" then 
				push(state.terminator,"]]")
				poke(state.stack,"string")
				return true
			else -- check for [==[ style strings
			end
		end

		local check_comment=function()
			if last=="comment" then
				if token==peek(state.terminator) then 
					pull(state.terminator)
					push_output("comment")
					poke(state.stack,"white")
					return true
				end
				return true -- we are trapped in a string
			elseif token=="--" then 
				push(state.terminator,"\n")
				poke(state.stack,"comment")
				return true
			elseif token=="--[[" then 
				push(state.terminator,"]]")
				poke(state.stack,"comment")
				return true
			else -- check for --[==[ style comments
			end
		end

		local check_list={
			["white"]={
				check_comment,
				check_string,
				check_white,
				check_keyword,
				check_punctuation,
				check_last,
			},
			["keyword"]={
				check_comment,
				check_string,
				check_white,
				check_punctuation,
				check_last,
			},
			["punctuation"]={
				check_comment,
				check_string,
				check_white,
				check_keyword,
				check_punctuation,
				check_last,
			},
			["string"]={
				check_string,
			},
			["comment"]={
				check_comment,
			},
			["none"]={
				check_comment,
				check_white,
				check_punctuation,
				check_last,
			},
		}

		local check=check_list[last] or { check_last }
		local done=false
		local idx=1
		while not done do done=(check[idx])() ; idx=idx+1 end

		push_output( peek(state.stack) )
	end
	
	local f=1
	local i=1
	local l=#input
	while i<=l do
		local s=istoken(input,i)
		if s then
			if f<i then
				pump(string.sub(input,f,i-1))
			end
			pump(s)
			i=i+#s
			f=i
		else
			i=i+1
		end
	end
	if f<l then
		pump(string.sub(input,f,l))
	end

	return output

end
