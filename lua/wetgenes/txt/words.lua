--[[#lua.wetgenes.txt.words

(C) 2023 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local wtxtwords=require("wetgenes.txt.words")

See https://github.com/xriss/engrish for source of words and possible 
alternative licenses.
  
]]

local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require
     =coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs, load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.langs={} -- loaded data

-- dumbass if not ending with then add this list
M.inflects={
	eng={
		j={"er","ist"},							-- adjective
		n={"s"},								-- noun
		v={"s","ed","en","ing"},				-- verb
		all={"s","ed","en","ing","er","ist"}	-- all
	},
}



--[[#lua.wetgenes.txt.words.load

	wtxtwords.load()
	wtxtwords.load(filename,fmt,lang)
	
Load language file.

If not called explicitly when will be called on first use of other 
functions in this module to load a defaults.

filename is the location of the file and defaults to 
"lua/wetgenes/txt/words/eng.tsv" this default data is available embeded 
in the gamecake executable so should always work.

lang is a 3 letter iso 639-3 language code and defaults to "eng"

Format is type of file that defaults to our "tsv" format of " word \t 
weight \t classes \n " where classes is then space separated as one 
word may belong to many.

]]
M.load=function(filename,fmt,lang)
	filename=filename or "lua/wetgenes/txt/words/eng.tsv"
	fmt=fmt or "tsv"
	lang=lang or "eng"
	
	M.lang={}
	M.langs[lang]=M.lang
	M.lang.words={}
	M.lang.inflects=M.inflects[lang] or {}

	local maxweight=0

	local wzips=require("wetgenes.zips")
	local lines=assert(wzips.readfile(filename),"file not found: "..filename)
	--print(words)
	local weight=0
	for line in lines:gmatch("[^\n]+") do
		weight=weight+1
		local cols={} ; for col in line:gmatch("%S+") do cols[#cols+1]=col end
		local word=string.lower(cols[1]) or ""
		local classes=cols[2] and string.lower(cols[2])
		if weight>maxweight then maxweight=weight end
		if word~="" then M.lang.words[word]={weight,classes} end
--[[
		for i=3,#cols do
			local class=cols[i]
			if not M.lang[class] then M.lang[class]={} end -- class lists
			M.lang[class][ #(M.lang[class])+1 ]=word
		end
]]
	end
	if maxweight>0 then
		for n,w in pairs(M.lang.words) do
			M.lang.words[n][1]=1-(w[1]/maxweight) -- weights are between 0 and 1 higher is better
		end
	end

end

--[[#lua.wetgenes.txt.words.load

	yes = wtxtwords.check(word)

This is a fast check if the word exists.

May call wtxtwords.load() to auto load data.

]]
M.check=function(word)
	if not M.lang then M.load() end
	if not word or word:find("%s") then return false end -- fast fail
	if M.lang.words[word] then return true end -- fast win
	for _,en in pairs(M.lang.inflects.all or {}) do
		if en == word:sub(#word+1-#en) then
			local v=word:sub(1,#word-#en)
			local t=M.lang.words[v]
			if t then return true end
		end
	end
	return false
end


--[[#lua.wetgenes.txt.words.transform

	words = wtxtwords.transform(word,maxdepth,words)

This is an internal function used when finding possible transforms from 
one word to another.

Transform one word into a map of word->depth for possible miss 
spellings using a subtractive transform ( we simply remove one letter 
per depth step ) 

The word you pass in will have a depth of 1 and then all the words that 
can be formed by taking away one letter will have a depth of 2 and so 
on.

]]
M.transform=function(word,maxdepth,words)
	words=words or {}
	maxdepth=maxdepth or #word
	local depth=1
	local new={[word]=true}
	if ( not words[word] ) or ( depth < words[word] ) then words[word]=depth end
	depth=depth+1
	while depth<maxdepth do
		local weight=depth -- (maxdepth-depth)/maxdepth
		local newnew={}
		for word in pairs(new) do
			for i=1,#word do
				local a=word:sub(0,i-1)
				local b=word:sub(i+1)
				local c=a..b
				if ( not words[c] ) or ( weight < words[c] ) then
					words[c]=weight
					newnew[c]=true
				end
			end
		end
		new=newnew
		depth=depth+1
	end
	return words
end


--[[#lua.wetgenes.txt.words.transform

	list = wtxtwords.transform(word,count,addletters,subletters)

Returns a table of upto count correctly spelled words that you may have 
miss spelt given the input word ordered by probability.

If the input word is spelled correctly then it will probably be the 
first word in this list but that is not guaranteed.

addletters is the maximum number of additive transforms, the higher 
this number the slower this function and it defaults to 4.

subletters is the maximum number of subtractive transforms and will not 
have much impact on speed, this defaults to the same value as 
addletters.

We run subletters subtractive transforms on our starting word and then 
we scan all possible words and perform addletters number of subtractive 
transforms on them and see if they match any of the transforms we built 
from our starting word. A match then means we can add up the number of 
transforms on both sides and that is how many steps it would take to 
get from one word to another by adding and subtracting letters.

]]
M.spell=function(_word,count,addletters,subletters)

	if not M.lang then M.load() end

	if not count then count=10 end
	if not addletters then addletters=3 end
	if not subletters then subletters=addletters+1 end

	local word=string.lower(_word)
	local m={}
	local ws=M.transform(word,subletters)
	for v,n in pairs(M.lang.words) do
		local ens={[""]=true}
		local classs=n[2] or ""
		for i=1,#classs do -- scan classes
			local class=classs:sub(i,i)
			local inflects=M.lang.inflects[class]
			for _,en in pairs(inflects or {}) do
				ens[en]=true
			end
		end
		for en in pairs(ens) do
			if #en==0 or v:sub(-#en)~=en then -- not already ending with
				local w=n[1]-(#en/10)
				local ven=v..en
				if ( #ven+1-addletters <= #word ) and ( #word+1-subletters <= #ven ) then -- must be this long for possible match
					for t,f in pairs( M.transform(ven,addletters) ) do
						if ws[t] then -- hit
							local weight=(1+n[1])/(f+ws[t])
							if ( not m[ven] ) or ( m[ven]<weight ) then
								m[ven]=weight
							end
						end
					end
				end
			end
		end
	end
	local t={}
	for a,b in pairs(m) do
		t[#t+1]={a,b}
	end
	table.sort(t,function(a,b) if a[2]==b[2] then return a[1]>b[1] end return a[2]>b[2] end)
	local ret={}
	for i=1,count do
		if t[i] then
			ret[#ret+1]=t[i][1]
		end
	end
	return ret
end
