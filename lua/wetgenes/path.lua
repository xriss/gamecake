--
-- (C) 2020 Kriss@XIXs.com and released under the MIT license,
-- see http://opensource.org/licenses/MIT for full license text.
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

--[[#lua.wetgenes.path

Manage file paths under linux or windows, so we need to deal with \ or 
/ and know the root difference between / and C:\

	local wpath=require("wetgenes.path")

]]
local M={} ; package.loaded[(...)]=M ; local wpath=M

-- a soft require of lfs so lfs can be nil
local lfs=select(2,pcall( function() return require("lfs_any") end ))


--[[#lua.wetgenes.path.setup

setup for windows or linux style paths, to force one or the other use

	wpath.setup("win")
	wpath.setup("nix")

We automatically call this at startup and make a best guess, you can 
revert to this best guess with

	wpath.setup()

This is a global setting, so be careful with changes. Mostly its 
probably best to stick with the best guess unless we are mistakenly 
guessing windows.

]]
wpath.setup=function(flavour)

-- try and guess if we are dealing with linux or windows style paths
	if not flavour then
		if package.config:sub(1,1) ==  "/" then -- paths begin with /
			flavour="nix"
		else
			flavour="win"
		end
	end

	if flavour == "win" then
	
		wpath.root="C:\\"
		wpath.separator="\\"
		wpath.delimiter=":"
		wpath.winhax=true

	elseif flavour == "nix" then

		wpath.root="/"
		wpath.separator="/"
		wpath.delimiter=";"
		wpath.winhax=true
		
-- always enable winhax by default, so we can hack around with windows style paths
-- we should make sure that we are always windows safe

	end

end
wpath.setup()



--[[#lua.wetgenes.path.split

split a path into numbered components

]]
wpath.split=function(p)
	local ps={}
	local fi=1
	while true do
		local fa,fb=string.find(p,wpath.winhax and "[\\/]" or "[/]",fi)
		if fa then
			local s=string.sub(p,fi,fa-1)
			if s~="" or ps[#ps]~="" then -- ignore multiple separators
				ps[#ps+1]=s
			end
			fi=fb+1
		else
			break
		end
	end
	ps[#ps+1]=string.sub(p,fi)

	return ps
end

--[[#lua.wetgenes.path.join

join a split path, tables are auto expanded

]]
wpath.join=function(...)
	local ps={}
	for i,v in ipairs({...}) do
		local t=type(v)
		if t=="table" then
			for j,s in ipairs(v) do
				if type(s)=="string" then
					ps[#ps+1]=s
				end
			end
		else
			if type(v)=="string" then
				ps[#ps+1]=v
			end
		end
	end
	return table.concat(ps,wpath.separator)
end

--[[#lua.wetgenes.path.parse

split a path into named parts like so

	-------------------------------------
	|               path                |
	-------------------------------------
	|           dir        |    file    |
	|----------------------|------------|
	| root |    folder     | name  ext  |
	|----------------------|------------|
	|  /   |  home/user/   | file  .txt |
	-------------------------------------
	
this can be reversed with simple joins and checks for nil

	dir = (root or "")..(folder or "")
	file = (name or "")..(ext or "")
	path = (dir or "")..(file or "")
	
if root is set then it implies an absolute path and will be something 
like C:\ under windows.

]]
wpath.parse=function(p)
	local ps=wpath.split(p)
	local r={}

	if ps[1] then
		if ps[1]=="" and ps[2] then -- unix root
			r.root=wpath.separator
			table.remove(ps,1)
		elseif #(ps[1])==2 and string.sub(ps[1],2,2)==":" and wpath.winhax then -- windows root
			r.root=ps[1]..wpath.separator
			table.remove(ps,1)
		end
	end

	if ps[1] then
		r.file=ps[#ps]
		table.remove(ps,#ps)

		local da,db=string.find(r.file, ".[^.]*$")
		if da and da>1 then -- ignore if at the start of name
			r.name=string.sub(r.file,1,da-1)
			r.ext=string.sub(r.file,da,db)
		else
			r.name=r.file
		end

	end

	if ps[1] then
		if ps[#ps] ~= "" then
			ps[#ps+1]="" -- force a trailing /
		end
		r.folder=table.concat(ps,wpath.separator)
	end

	if r.root then -- root is part of dir
		r.dir=r.root..(r.folder or "")
	else
		r.dir=r.folder -- may be nil
	end
	
	r.path = (r.dir or "")..(r.file or "")

	return r
end

--[[#lua.wetgenes.path.normalize

remove ".." and "." components from the path string

]]
wpath.normalize=function(p)

	local pp=wpath.parse(p) -- we need to know if path contains a root
	local ps=wpath.split(p)
	
	local idx=2

	while idx <= #ps-1 do
		if ps[idx]=="" then -- remove double //
			table.remove(ps,idx)
		else -- just advance
			idx=idx+1
		end
	end
			
	idx=1
	while idx <= #ps do
		if ps[idx]=="." then -- just remove this one, no need to advance
			table.remove(ps,idx)
		elseif ps[idx]==".." then -- remove this and the previous one if we can
			if idx>(pp.root and 2 or 1) then -- can we remove previous part
				idx=idx-1
				table.remove(ps,idx)
				table.remove(ps,idx)
			else -- we can not remove so must ignore
--				table.remove(ps,idx)
				idx=idx+1
			end
		else -- just advance
			idx=idx+1
		end
	end

	return wpath.join(ps)
end

--[[#lua.wetgenes.path.currentdir

Get the current working directory, this requires lfs and if lfs is not 
available then it will return wpath.root this path will have a trailing 
separator so can be joined directly to a filename.

	wpath.currentdir().."filename.ext"

]]
wpath.currentdir=function()

	local d
	if lfs then d=lfs.currentdir() end
	
	if d then -- make sure we end in a separator
		local ds=wpath.split(d)
		if ds[#ds] ~= "" then
			ds[#ds+1]="" -- force a trailing /
		end
		return wpath.join(ds)
	end

	return wpath.root -- default root
end


--[[#lua.wetgenes.path.resolve

Join all path segments and resolve them to absolute using wpath.join 
and wpath.normalize with a prepended wpath.currentdir as necessary.

]]
wpath.resolve=function(...)

	local p=wpath.join(...)

	if wpath.parse(p).root then -- already absolute
		return wpath.normalize(p) -- just normalize
	end
	
	return wpath.normalize( wpath.currentdir()..p ) -- prepend currentdir
end


--[[#lua.wetgenes.path.relative

Build a relative path from point a to point b this will probably be a 
bunch of ../../../ followed by some of the ending of the second 
argument.

]]
wpath.relative=function(pa,pb)

	local a=wpath.split(wpath.resolve(pa))
	local b=wpath.split(wpath.resolve(pb))
	
	if a[#a] == "" then -- remove trailing slash
		table.remove(a,#a)
	end

	local r={}
	local match=#a+1 -- if the test below falls through then the match is all of a
	for i=1,#a do
		if a[i] ~= b[i] then -- start of difference
			match=i
			break
		end
	end

	if match==1 or ( match==2 and a[1]=="" )  then -- no match
		return pb -- just return full path
	end

	for i=match,#a do r[#r+1]=".." end -- step back
	if #r==0 then r[#r+1]="." end -- start at current
	for i=match,#b do r[#r+1]=b[i] end -- step forward

	return wpath.join(r)
end

--[[#lua.wetgenes.path.parent

Resolve input and go up a single directory level, ideally you should 
pass in a directory, IE a string that ends in / or \ and we will return 
the parent of this directory.

If called repeatedly, then eventually we will return wpath.root

]]
wpath.parent=function(...)
	return wpath.resolve(...,"..","")
end
