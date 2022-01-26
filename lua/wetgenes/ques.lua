
--[[


Turn shallow tables too and from query strings

Ie a ?name=value&other=value style string with auto escaped characters

]]


local M={ modname=(...) } ; if M.modname then package.loaded[M.modname]=M end

local ques=M


--[[

Unescape all %00 values into their original chars

]]
ques.decode=function(s)
	return s:gsub("%%(%x%x)", function(x) return string.char(tonumber( x , 16 )) end )
end

--[[

escape all special chars into %00 values, so ? & = are removed. Everything else is fine.

]]
ques.encode=function(s)
	return s:gsub("([%?%&%=])", function(c) return string.format( "%%%02X", string.byte(c) ) end )
end

--[[

Parse a string into a table, we use numbered entries to hold extra info, eg q[0] is the base part of the string before the ?
or "" if there was no ?

]]
ques.parse=function(s,q)
	q=q or {} -- can pass in a table to reuse
	
	local idx=string.find(s, "?") -- remove the ? from the string and store the left bit in q[0]
	
	if not idx then -- no ?
		q[0]=""
		idx=0
	else
		q[0]=string.sub(s,1,idx-1)
	end

	while idx do
		local f=string.find(s, "&",idx+1)
		local b=""
		if f then
			b=string.sub(s,idx+1,f-1)
			idx=f
		else
			b=string.sub(s,idx+1)
			idx=nil
		end
		if b and b~="" then -- something to save
			local e=string.find(b,"=")
			if e then
				local n=ques.decode( string.sub(b,1,e-1) )
				local v=ques.decode( string.sub(b,e+1)   )
				q[n]=v
			else	--  a name with no value is automatically set to 1
				local n=ques.decode( b )
				q[n]=1	-- note this is an actual number not a string
			end
		end
	end
	
	return q
end

--[[

Build a string from a table of values

]]
ques.build=function(q)
	local t={}
	local s="?"
	
	t[1]=q[0] or ""
	
	for n,v in pairs(q) do
		if type(n)=="string" then -- only save strings
			t[#t+1]=s ; s="&"
			t[#t+1]=ques.encode(n)
			t[#t+1]="="
			t[#t+1]=ques.encode(tostring(v)) -- might be a number
		end
	end

	return table.concat(t)
end


--[[

Test function

]]
ques.test=function()
	for i,v in ipairs({

	"poop?test=1",
	"?test=1",
	"test=1",
	"poop?test=a&other=b&this=that&bool&not&&",
	"?space%20space=dot%2Edot&eq%3Deq",

	}) do

		print()
		print(v)
		local q=ques.parse( v )
		for n,v in pairs(q) do print(n,v) end
		print( ques.build( q ) )
	end
end
--ques.test()
