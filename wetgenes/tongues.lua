--
-- (C) 2015 wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local tongues={}

tongues.file="tongues.tsv"

tongues.lang="en"

-- en -> whatever
tongues.lookup={}

-- strings we looked up this run
tongues.used={}

-- set the language (do this before load)
tongues.set=function(lang)
	tongues.lang=lang
end
tongues.get=function(lang)
	return tongues.lang
end

-- make sure the langauge you want is set before calling
-- we only keep track of one translation
-- load csv of translations and store in lookup
tongues.load=function()
end

-- call at shutdown
-- save strings that where untranslated for later translatiosn
-- we actually do a load / add / save of data here to make sure 
-- that the unknown strings file is accurate
-- the idea is we hand translate strings that are listed as unknown
tongues.save=function()
end

-- translate a string to tongues.lang
tongues.translate=function(str)
	tongues.used[str]=true
	return str
end


return tongues
