--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- this uses math.random so remember to seed it if you want it to really be "random"

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,names)

	names=names or {}
	
	names.random=M.random

	return names
end

function M.random_adjective()
	return M.txt_adjectives[ math.random(1,#M.txt_adjectives) ]
end

function M.random_noun()
	return M.txt_nouns[ math.random(1,#M.txt_nouns) ]
end

function M.random()
	return M.random_adjective().."_"..M.random_noun()
end


M.txt_adjectives={}
M.txt_nouns={}

do
	local wzips=require("wetgenes.zips")
	local filename="lua/wetgenes/txt/words/eng.tsv"
	local lines=assert(wzips.readfile(filename),"file not found: "..filename)
	--print(words)
	for line in lines:gmatch("[^\n]+") do
		local cols={} ; for col in line:gmatch("%S+") do cols[#cols+1]=col end
		local word=string.lower(cols[1]) or ""
		local classs=cols[2] or ""
		for i=1,#classs do
			local class=classs:sub(i,i)
			if class=="m" or class=="n" then
				M.txt_nouns[#M.txt_nouns+1]=word
			elseif class=="j" then
				M.txt_adjectives[#M.txt_adjectives+1]=word
			end
		end
	end
end
