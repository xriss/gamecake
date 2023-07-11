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
	["keyword"]="k",
	["number"]="0",
	["punctuation"]="p",
	["string"]="s",
	["string_spell"]="S",
	["comment"]="c",
	["comment_spell"]="C",
	["global"]="g",
	["none"]="n",
	["first"]="f",
}
local MAP=M.MAP


-- these wild card tests for non explicit tokens should all be anchored at start of string by beginning with a ^
M.wild_tokens={
	"^[0-9a-zA-Z_]+",						-- a variable or function name
	"^[0-9a-zA-Z_]+[%.%:][0-9a-zA-Z_]+",	-- a variable or function name containing a single . or :
	"^[ \t]+",								-- multiple tabs or spaces but not newlines
	"^%d*%.?%d*[eE][%+%-]?%d*",				-- floating point number with exponent
	"^%d*%.?%d*",							-- floating point number or int
	"^0x[0-9a-fA-F]+",						-- hex number
	"^%-%-%[=*%[",							-- comment start
	"^%[=*%[",								-- string start
	"^%]=*%]",								-- comment or string end
}
-- test if a pattern matches entire string
local fullmatch=function(s,p)
	local fs,fe = string.find(s,p)
	if fs==1 and fe==#s then return true end
	return false
end

M.token={}

M.token.punctuation=keyval{"<",">","==","<=",">=","~=","-","+","*","/","%","^","=","#",";",",","..","...","(",")","[","]","{","}"}

M.token.comment=keyval{"--","#!"}

M.token.string=keyval{"\\\\","\\\"","\"","'","[[","]]"}

M.token.keyword=keyval{

	"and",
	"break",
	"do",
	"else",
	"elseif",
	"end",
	"false",
	"for",
	"function",
	"if",
	"in",
	"local",
	"nil",
	"not",
	"or",
	"repeat",
	"return",
	"then",
	"true",
	"until",
	"while",

}

M.token.global=keyval{

	"assert",
	"dofile",
	"error",
	"getfenv",
	"getmetatable",
	"ipairs",
	"load",
	"loadfile",
	"loadstring",
	"next",
	"pairs",
	"pcall",
	"print",
	"rawequal",
	"rawget",
	"rawset",
	"select",
	"setfenv",
	"setmetatable",
	"tonumber",
	"tostring",
	"type",
	"unpack",
	"xpcall",
	"module",
	"require",
	"_G",
	"_VERSION",

	"coroutine",
	"coroutine.create",
	"coroutine.isyieldable",
	"coroutine.resume",
	"coroutine.running",
	"coroutine.status",
	"coroutine.wrap",
	"coroutine.yield",

	"debug",
	"debug.debug",
	"debug.gethook",
	"debug.getinfo",
	"debug.getlocal",
	"debug.getmetatable",
	"debug.getregistry",
	"debug.getupvalue",
	"debug.getuservalue",
	"debug.sethook",
	"debug.setlocal",
	"debug.setmetatable",
	"debug.setupvalue",
	"debug.setuservalue",
	"debug.traceback",
	"debug.upvalueid",
	"debug.upvaluejoin",

	"io",
	"io.close",
	"io.flush",
	"io.input",
	"io.lines",
	"io.open",
	"io.output",
	"io.popen",
	"io.read",
	"io.stderr",
	"io.stdin",
	"io.stdout",
	"io.tmpfile",
	"io.type",
	"io.write",

	"math",
	"math.abs",
	"math.acos",
	"math.asin",
	"math.atan",
	"math.ceil",
	"math.cos",
	"math.deg",
	"math.exp",
	"math.floor",
	"math.fmod",
	"math.huge",
	"math.log",
	"math.max",
	"math.maxinteger",
	"math.min",
	"math.mininteger",
	"math.modf",
	"math.pi",
	"math.rad",
	"math.random",
	"math.randomseed",
	"math.sin",
	"math.sqrt",
	"math.tan",
	"math.tointeger",
	"math.type",
	"math.ult",

	"os",
	"os.clock",
	"os.date",
	"os.difftime",
	"os.execute",
	"os.exit",
	"os.getenv",
	"os.remove",
	"os.rename",
	"os.setlocale",
	"os.time",
	"os.tmpname",

	"package",
	"package.config",
	"package.cpath",
	"package.loaded",
	"package.loadlib",
	"package.path",
	"package.preload",
	"package.searchers",
	"package.searchpath",

	"string",
	"string.byte",
	"string.char",
	"string.dump",
	"string.find",
	"string.format",
	"string.gmatch",
	"string.gsub",
	"string.len",
	"string.lower",
	"string.match",
	"string.pack",
	"string.packsize",
	"string.rep",
	"string.reverse",
	"string.sub",
	"string.unpack",
	"string.upper",

	"table",
	"table.concat",
	"table.insert",
	"table.move",
	"table.pack",
	"table.remove",
	"table.sort",
	"table.unpack",

	"utf8",
	"utf8.char",
	"utf8.charpattern",
	"utf8.codepoint",
	"utf8.codes",
	"utf8.len",
	"utf8.offset",
	
	"jit",
	"jit.on",
	"jit.off",
	"jit.flush",
	"jit.status",
	"jit.version",
	"jit.version_num",
	"jit.os",
	"jit.arch",
	"jit.opt",
	"jit.opt.start",
	"jit.util",
	
	"ffi",
	"ffi.cdef",
	"ffi.load",
	"ffi.new",
	"ctype",
	"ffi.typeof",
	"ffi.cast",
	"ffi.metatype",
	"ffi.gc",
	"ffi.C",
	"ffi.C.free",
	"ffi.sizeof",
	"ffi.alignof",
	"ffi.offsetof",
	"ffi.istype",
	"ffi.errno",
	"ffi.string",
	"ffi.copy",
	"ffi.fill",
	"ffi.abi",
	"ffi.os",
	"ffi.arch",

}

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

		local check_token=function()
			if M.token.keyword[token] then
				poke(state.stack,MAP.keyword)
				return true
			end
			if M.token.global[token] then
				poke(state.stack,MAP.global)
				return true
			end
		end

		local check_punctuation=function()
			if M.token.punctuation[token] then
				poke(state.stack,MAP.punctuation)
				return true
			end
		end
		
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
		
		local check_string=function()
			if last==MAP.string or last==MAP.string_spell then
				if token==peek(state.terminator) then 
					pull(state.terminator)
					push_output(MAP.string)
					poke(state.stack,MAP.punctuation)
					return true
				end
				if check_spell() then -- good spelling
					poke(state.stack,MAP.string)
				else -- bad spelling
					poke(state.stack,MAP.string_spell)
				end
				return true -- we are trapped in a string
			elseif token=="\"" then 
				push(state.terminator,"\"")
				poke(state.stack,MAP.string)
				return true
			elseif token=="'" then 
				push(state.terminator,"'")
				poke(state.stack,MAP.string)
				return true
			end			
			local fs,fe,ee=string.find(token,"^%[(=*)%[")
			if fs==1 and fe==#token then -- full match
				push(state.terminator,"]"..(ee or "").."]")
				poke(state.stack,MAP.string)
				return true
			end

		end

		local check_comment=function()
			if last==MAP.comment or last==MAP.comment_spell then
				if token==peek(state.terminator) then 
					pull(state.terminator)
					push_output(MAP.comment)
					poke(state.stack,MAP.white)
					return true
				end
				if check_spell() then -- good spelling
					poke(state.stack,MAP.comment)
				else -- bad spelling
					poke(state.stack,MAP.comment_spell)
				end
				return true -- we are trapped in a string
			elseif token=="--" then 
				push(state.terminator,"\n")
				poke(state.stack,MAP.comment)
				return true
			end
			local fs,fe,ee=string.find(token,"^%-%-%[(=*)%[")
			if fs==1 and fe==#token then -- full match
				push(state.terminator,"]"..(ee or "").."]")
				poke(state.stack,MAP.comment)
				return true
			end
		end

		local goto_none=function()
			if token~="" then -- if empty string then no change
				poke(state.stack,MAP.none)
			end
			return true -- but always return true
		end
		

		local check_list={
			[MAP.white]={
				check_number,
				check_token,
				check_comment,
				check_string,
				check_white,
				check_punctuation,
				goto_none,
			},
			[MAP.keyword]={
				check_comment,
				check_string,
				check_white,
				check_punctuation,
				goto_none,
			},
			[MAP.punctuation]={
				check_number,
				check_token,
				check_comment,
				check_string,
				check_white,
				check_punctuation,
				goto_none,
			},
			[MAP.global]={
				check_comment,
				check_white,
				check_punctuation,
				goto_none,
			},
			[MAP.number]={
				check_number,
				check_comment,
				check_white,
				check_punctuation,
				goto_none,
			},
			[MAP.string]={
				check_string,
			},
			[MAP.string_spell]={
				check_string,
			},
			[MAP.comment]={
				check_comment,
			},
			[MAP.comment_spell]={
				check_comment,
			},
			[MAP.none]={
				check_comment,
				check_white,
				check_punctuation,
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
