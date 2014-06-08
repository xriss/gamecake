#!/usr/bin/env gamecake

local wbake	=require("wetgenes.bake")
local wstr	=require("wetgenes.string")

local markdown	=require("markdown")

local dprint=function(...) print(wstr.dump(...)) end


local basedir="../../bin/lua/"

local files=wbake.findfiles{basedir=basedir,dir="",filter="."}.ret

dprint(files)

	local chunks={}
	chunks.__flags={}

for _,filename in ipairs(files) do

	local txt=wbake.readfile(basedir..filename)
	local lines=wstr.split(txt,"\n")

	print(filename,#txt)
	

	local mode="none"
	local chunkname
	local chunk
	for i,v in pairs(lines) do
	
		if v:sub(1,5)=="--[[#" then -- special start of flag

			local splits=wstr.split(v:sub(6)," ")
			if splits[1] and splits[1]~="" then
				mode="dox"
				chunkname=splits[1]
				for i=2,#splits do
					local v=splits[i]
					local aa=wstr.split(v,"=")
					if aa[1] and aa[2] then
						chunks.__flags[chunkname]=chunks.__flags[chunkname] or {}
						chunks.__flags[chunkname][ aa[1] ]=aa[2] -- assign flag
					end
				end
				chunks[chunkname]=chunks[chunkname] or {}
				chunk=chunks[chunkname]
			end

		elseif mode=="dox" and v:sub(1,2)=="]]" then

			chunkname=chunkname..".source"
			chunks[chunkname]=chunks[chunkname] or {}
			chunk=chunks[chunkname]
			mode="source"

		else
			if mode=="dox" then
				table.insert(chunk,v)
			end
			if mode=="source" then
				table.insert(chunk,v)
				if v:sub(1,3)=="end" then mode="none" end
			end
		end


	end

end

for n,v in pairs(chunks) do
	if n~="__flags" then
		v[#v+1]="" chunks[n]=table.concat(v,"\n")
	end
end

--dprint(chunks)

local htmls={}

for n,v in pairs(chunks) do
	if n~="__flags" then
		local aa=wstr.split(n,".")
		local name
		for i=1,#aa-1 do
			if not name then name=aa[i] else name=name.."."..aa[i] end
			htmls[name]=htmls[name] or {}
			htmls[name][n]=v
		end
		name="index" -- everything always goes into index.html
		htmls[name]=htmls[name] or {}
		htmls[name][n]=v
	end
end


local function html(v)

	return [[
<html>
	<head>
	</head>
	<body>
]]..markdown(v)..[[
	</body>
</html>
]]

end

-- spit out single html chunks for simple linking to
for n,v in pairs(chunks) do
	if n~="__flags" then
		if n:sub(-7)~=".source" then
			wbake.writefile( "html/"..n..".html",html(v))
		end
	end
end

-- sit out pages containing many html chunks
for n,v in pairs(htmls) do
	t={}
	for n,s in pairs(v) do
		if n:sub(-7)~=".source" then
			t[#t+1]={n,s}
		end
	end
	table.sort(t,function(a,b)
		local aa=wstr.split(a[1],".")
		local bb=wstr.split(b[1],".")
		local l=#aa > #bb and #aa or #bb
		for i=1,#bb do
			if not aa[i] then return true end
			if not bb[i] then return false end

			if aa[i+1] and not bb[i+1] then return false end
			if bb[i+1] and not aa[i+1] then return true end

			if aa[i]=="init" then return true end -- init is the main module documentation
			if bb[i]=="init" then return false end
			if aa[i]<bb[i] then return true end
			if aa[i]>bb[i] then return false end
		end
		return false
	end)
	for n,s in pairs(t) do
		t[n]="<h1>"..s[1].."</h1>\n<div>"..markdown(s[2]).."</div>"
	end
	wbake.writefile( "html/"..n..".html",html(table.concat(t,"<hr/>\n")))
end



--print( markdown( table.concat(chunks,"\n") ) )

