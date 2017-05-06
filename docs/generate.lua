#!/usr/bin/env gamecake

local wbake	=require("wetgenes.bake")
local wstr	=require("wetgenes.string")

local markdown	=require("markdown")

local dprint=function(...) print(wstr.dump(...)) end


local basedir="../lua/"

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
		if aa[#aa]~="source" then
			local name
			for i=1,#aa do
				if not name then name=aa[i] else name=name.."."..aa[i] end
				htmls[name]=htmls[name] or {}
				htmls[name][n]=v
			end
			name="index" -- everything always goes into index.html
			htmls[name]=htmls[name] or {}
			htmls[name][n]=v
		end
	end
end


local function html(content,index)

	return [[
<html>
	<head>
		<link rel="shortcut icon" href="data/gcdoc.png" />
		<link rel='stylesheet' href='data/dox.css' />
		<link rel="stylesheet" href="//yandex.st/highlightjs/8.0/styles/sunburst.min.css">
		<script src="data/jquery.min.js"></script>
		<script src="data/dox.js"></script>
		<script src="//yandex.st/highlightjs/8.0/highlight.min.js"></script>
		<script>hljs.initHighlightingOnLoad();</script>
	</head>
	<body>
		<div class="content">
]]..(content)..[[
			<a href="#" class="top">Back to top</a>
		</div>
		<div class="index">
			<a href="index.html" class="meca" title="To Docs Cave!"></a>
]]..(index)..[[
		</div>
	</body>
</html>
]]

end

local function disqus(n)

	do return "" end -- disable discus

	return [[
<div id="disqus_thread"></div>
<script type="text/javascript">
	var disqus_shortname = 'gamecake';
	var disqus_identifier = ']]..n..[[';
	var disqus_title=disqus_identifier;
	(function() {
		var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;
		dsq.src = '//' + disqus_shortname + '.disqus.com/embed.js';
		(document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);
	})();
</script>
]]

end

--[[ spit out single html chunks for simple linking to
for n,v in pairs(chunks) do
	if n~="__flags" then
		if n:sub(-7)~=".source" then
			wbake.writefile( "./"..n..".html",html("<h1>"..n.."</h1>\n<div>"..markdown(v).."</div>"..disqus(n)))
		end
	end
end
]]


local index="" -- all ul within ul links to files

do
-- heirachical index
	t={}

	for n,v in pairs(htmls) do

		for n,s in pairs(v) do
		
			if n:sub(-7)~=".source" then

				local aa=wstr.split(n,".")
				
				local level=t
				for idx=1,#aa do
					local name=aa[idx]
					
					level[name]=level[name] or {}
					
					level=level[name]
				end

			end
			
		end

	end

	local sorted={}
	do
		local recurse
		recurse=function(parent)
		
			local tab={}

			local names={}
			for n,v in pairs(parent) do names[#names+1]=n end
			table.sort(names)
			
			for i,name in ipairs(names) do
			
				tab[i]={name,recurse(parent[name])}
				
			end
			
			return tab
		end

		sorted=recurse(t)
	end
	

	local lines={}
	local push=function(s) lines[#lines+1]=s end
	do
		local recurse
		recurse=function(parent,pname,proot)
			
			push("<ul>")
			
			for i,v in ipairs(parent) do

				local name=v[1]
				local children=v[2]
				local root=(proot=="") and name or proot.."."..name
				
				push("<li>")
				
				local class=" class=\"ul-link\" "
				if children[1] then
					class=" class=\"ul-expand\" "
				end

				push("<a href=\""..root..".html\""..class..">"..name.."</a>")
				
				if children[1] then
					recurse(children,name, root )
				end

				push("</li>")
				
			end
			
			push("</ul>")

		end
		recurse(sorted,"","")
	end

	index=table.concat(lines,"\n")
end


-- spit out pages containing many html chunks
for n,v in pairs(htmls) do
	t={}
	for n,s in pairs(v) do
		if n:sub(-7)~=".source" then
			t[#t+1]={n,s}
		end
	end
	table.sort(t,function(a,b)
	
--		local ac=0 for n,v in pairs(htmls[ a[1] ]) do ac=ac+1 end
--		local bc=0 for n,v in pairs(htmls[ b[1] ]) do bc=bc+1 end

		local aa=wstr.split(a[1],".")
		local bb=wstr.split(b[1],".")
		local l=#aa > #bb and #aa or #bb
		for i=1,l do
			-- a shorter name always has higher priority
			if not aa[i] then return true end
			if not bb[i] then return false end

			if aa[i]<bb[i] then return true end
			if aa[i]>bb[i] then return false end

		end
		return false
	end)
	for n,s in pairs(t) do
		t[n]="<h1><a href=\""..s[1]..".html\">"..s[1].."</a></h1>\n<div>"..markdown(s[2]).."</div>"
	end
	
	local links={}
	links[#links+1]="<div class=\"crumbs_wrap\">"
	links[#links+1]="<div class=\"crumbs\"><a href=\"index.html\">home</a> &gt; </div>"
	local aa=wstr.split(n,".")
	local name
	for i=1,#aa-1 do
		if not name then name=aa[i] else name=name.."."..aa[i] end
		links[#links+1]="<div class=\"crumbs\"><a href=\""..name..".html\">"..name.."</a> &gt; </div>"
	end
	links[#links+1]="</div>"
	wbake.writefile( "./"..n..".html",html(table.concat(links)..table.concat(t,"<hr/>\n")..disqus(n),index))
end



--print( markdown( table.concat(chunks,"\n") ) )

