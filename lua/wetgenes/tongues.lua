--
-- (C) 2015 wetgenes.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local lfs ; pcall( function() lfs=require("lfs") end ) -- may not have a filesystem


local wwin=require("wetgenes.win") -- system independent helpers
local wzips=require("wetgenes.zips")
local wstr=require("wetgenes.string")

local tongues={}

-- flag, (set by default) that removes high UTF8 characters from the translated strings
tongues.strip_UTF8=true


local chars={}
tongues.chars=chars

chars.french={
["À"]="A",--Capital A-grave
["à"]="a",--Lowercase a-grave
["Â"]="A",--Capital A-circumflex
["â"]="a",--Lowercase a-circumflex
["Æ"]="A",--Capital AE Ligature
["æ"]="a",--Lowercase AE Ligature
["Ç"]="C",--Capital C-cedilla
["ç"]="c",--Lowercase c-cedilla
["È"]="E",--Capital E-grave
["è"]="e",--Lowercase e-grave
["É"]="E",--Capital E-acute
["é"]="e",--Lowercase e-acute
["Ê"]="E",--Capital E-circumflex
["ê"]="e",--Lowercase e-circumflex
["Ë"]="E",--Capital E-umlaut
["ë"]="e",--Lowercase e-umlaut
["Î"]="I",--Capital I-circumflex
["î"]="i",--Lowercase i-circumflex
["Ï"]="I",--Capital I-umlaut
["ï"]="i",--Lowercase i-umlaut
["Ô"]="O",--Capital O-circumflex
["ô"]="o",--Lowercase o-circumflex
["Œ"]="O",--Capital OE ligature
["œ"]="o",--Lowercase oe ligature
["Ù"]="U",--Capital U-grave
["ù"]="u",--Lowercase u-grave
["Û"]="U",--Capital U-circumflex
["û"]="u",--Lowercase U-circumflex
["Ü"]="U",--Capital U-umlaut
["ü"]="u",--Lowercase U-umlaut
["«"]="<",--Left angle quotes
["»"]=">",--Right angle quotes
["€"]="E",--Euro
["₣"]="F",--Franc
}
chars.german={
["Ä"]="A",--Capital A-umlaut
["ä"]="a",--Lowercase a-umlaut
["É"]="E",--Capital E-acute
["é"]="e",--Lowercase E-acute
["Ö"]="O",--Capital O-umlaut
["ö"]="o",--Lowercase o-umlaut
["Ü"]="U",--Capital U-umlaut
["ü"]="u",--Lowercase u-umlaut
["ß"]="B",--SZ ligature
["«"]="<",--Left angle quotes
["»"]=">",--Right angle quotes
["„"]='"',--Left lower quotes
["“"]='"',--Left quotes
["”"]='"',--Right quotes
["°"]='"',--Degree sign (Grad)
["€"]="E",--Euro
["£"]="P",--Pound Sterling
}

chars.italian={
["À"]="A",--Capital A-grave
["à"]="a",--Lowercase a-grave
["Á"]="A",--Capital A-acute
["á"]="a",--Lowercase A-acute
["È"]="E",--Capital E-grave
["è"]="e",--Lowercase e-grave
["É"]="E",--Capital E-acute
["é"]="e",--Lowercase e-acute
["Ì"]="I",--Capital I-grave
["ì"]="i",--Lowercase i-grave
["Í"]="I",--Capital I-acute
["í"]="i",--Lowercase I-acute
["Ò"]="O",--Capital O-grave
["ò"]="o",--Lowercase o-grave
["Ó"]="O",--Capital O-acute
["ó"]="o",--Lowercase o-acute
["Ù"]="U",--Capital U-grave
["ù"]="u",--Lowercase u-grave
["Ú"]="U",--Capital U-acute
["ú"]="u",--Lowercase U-acute
["«"]="<",--Left angle quotes
["»"]=">",--Right angle quotes
["€"]="E",--Euro
["₤"]="L",--Lira
}

chars.spanish={
["Á"]="A",--Capital A-acute
["á"]="a",--Lowercase a-acute
["É"]="E",--Capital E-acute
["é"]="e",--Lowercase e-acute
["Í"]="I",--Capital I-acute
["í"]="i",--Lowercase i-acute
["Ñ"]="N",--Capital N-tilde
["ñ"]="n",--Lowercase n-tilde
["Ó"]="O",--Capital O-acute
["ó"]="o",--Lowercase o-acute
["Ú"]="U",--Capital U-acute
["ú"]="u",--Lowercase u-acute
["Ü"]="U",--Capital U-umlaut
["ü"]="u",--Lowercase u-umlaut
["«"]="<",--Left angle quotes
["»"]=">",--Right angle quotes
["¿"]="?",--Inverted question mark
["¡"]="!",--Inverted exclamation point
["€"]="E",--Euro
["₧"]="P",--Peseta
}

local all={}
for i=1,127 do local b=string.char(i) all[b]=b end -- all 7bit chars map to themselves
for n,v in pairs(chars) do
	for c1,c2 in pairs(v) do
		all[c1]=c2
	end
end
chars.all=all -- merge them all together
-- now we can use this table for a very simple utf8 -> 7bit string lookup conversion


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
	local s=tongues.lookup[str] or str
-- remove high UTF8 codes preferably by stripping diacritics
	if tongues.strip_UTF8 then
		s=s:gsub("([%z\1-\127\194-\244][\128-\191]*)",function(su)
			return chars.all[su] or ""
		end)
	end
	
	return s
end


return tongues
