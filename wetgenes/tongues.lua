--
-- (C) 2015 wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs ; pcall( function() lfs=require("lfs") end ) -- may not have a filesystem


local wwin=require("wetgenes.win") -- system independent helpers
local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local tongues={}

tongues.loadfilename="data/tongues.tsv"
tongues.savefilename=wwin.files_prefix.."tongues.new.tsv"

tongues.lang="english"

-- english or id -> whatever
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
-- load tsv of translations and store in lookup
tongues.load=function()

print("Loading "..tongues.lang.." from "..tongues.loadfilename)

	local str=wzips.readfile(tongues.loadfilename)
	
	local head
	local lang_idx=0
	local id_idx=1
	tongues.lookup={}
	local count=0
	if str then
		for line in str:gmatch("[^\r\n]+") do
			if not head then -- first line
				head=wstr.split(line,"\t")
				for i,v in ipairs(head) do
					if v=="id" then id_idx=i end
					if v==tongues.lang then lang_idx=i end
				end
			else
				local ts=wstr.split(line,"\t")
				for i,v in ipairs(ts) do
					local sa,sb=v:sub(1,1),v:sub(-1,-1)
					if #v>=2 and sa==sb and ( sa=='"' or sa=="'" ) then -- handle dumb quoting that we do not want
						ts[i]=v:sub(2,-2)
					end
				end
				local id=ts[id_idx]
				local ss=ts[lang_idx]
				if (not ss) or (ss=="") then ss=id end -- the id is "probably" the english translation
				if id then
					tongues.lookup[id]=ss
					count=count+1
				end
			end
		end
	end

print("Loaded "..count.." translation strings")

end

-- call at shutdown
-- save strings that where untranslated for later translatiosn
-- we actually do a load / add / save of data here to make sure 
-- that the new strings file is accurate
-- the idea is we hand translate these strings and add them to the .tsv
tongues.save=function()

		if lfs then
print("Loading "..tongues.savefilename)
			pcall(function() -- file might not exist
				for line in io.lines(tongues.savefilename) do -- check all these strings as well
					tongues.used[line]=true
				end
			end)
print("Saving "..tongues.savefilename)
			local fp=io.open(tongues.savefilename,"w")
			if fp then
				for s,b in pairs(tongues.used) do
					if ( not tongues.lookup[s] ) then -- only save if string is not available in lookup
						fp:write(s)
						fp:write("\n")
					end
				end
				fp:close()
			end
		end


end

-- translate a string to tongues.lang
tongues.translate=function(str)
	tongues.used[str]=true
	return tongues.lookup[str] or str
end


return tongues
