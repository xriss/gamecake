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
local lfs ; pcall( function() lfs=require("lfs_any") end )

--[[#lua.wetgenes.path.unslash

If the path ends in "/" ( or multiple "//" etc ) then remove it,

return the path which is now guaranteed not to end in the slash separator.

]]
wpath.unslash=function(s)
	while s:sub(-#wpath.separator)==wpath.separator do -- trim trailing separator
		s=s:sub(1,-1-#wpath.separator)
	end
	return s
end

--[[#lua.wetgenes.path.split

split a path into numbered components

]]
wpath.split=function(...)
	local p=wpath.join(...)
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

	|--------------------------------------------|
	|                     path                   |
	|-----------------------|--------------------|
	|         dir           |        file        |
	|----------|------------|----------|---------|
	| root [1] | folder [2] | name [3] | ext [4] |
	|----------|------------|----------|---------|
	| /        | home/user/ | file     | .txt    |
	|----------|------------|----------|---------|

this can be reversed with simple joins and checks for nil, note that 
[1][2][3][4] are forced strings so will be "" rather than nil unlike 
their named counterparts. This means you may use wpath.join to reverse 
this parsing.

	dir = (root or "")..(folder or "")
	file = (name or "")..(ext or "")
	path = (dir or "")..(file or "")
	path = (root or "")..(folder or "")..(name or "")..(ext or "")
	path = [1]..[2]..[3]..[4]
	path = wpath.join(it)
	
if root is set then it implies an absolute path and will be something 
like C:\ under windows.

]]
wpath.parse=function(...)
	local p=wpath.join(...)
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
	
	r[1]=r.root   or ""
	r[2]=r.folder or ""
	r[3]=r.name   or ""
	r[4]=r.ext    or ""

	return r
end

--[[#lua.wetgenes.path.normalize

remove ".." and "." components from the path string if we can

this will also convert multiple "/" into a single "/" but will not remove a trailing "/"

]]
wpath.normalize=function(...)

	local p=wpath.join(...)
	local pp=wpath.parse(p) -- we need to know if path contains a root
	local ps=wpath.split(p)
	
	local idx=2

	while idx <= #ps-1 do -- do not remove trailing /
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
				if ps[idx-1]==".." then -- double .. cant remove
					idx=idx+1
				else
					idx=idx-1
					table.remove(ps,idx)
					table.remove(ps,idx)
				end
			else -- we can not remove so must ignore
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

expand paths starting with ~ to the home path which is usually found in 
environment variable HOME ( or USERPROFILE on windows )

prepended wpath.currentdir if this is not a full path with a root

Join all path segments and resolve them to absolute using wpath.join 
and wpath.normalize.

We should end up with an absolute path without any . or .. parts.

]]
wpath.resolve=function(...)

	local p=wpath.join(...)
	if wpath.home and ( (p:sub(1,2)=="~/") or (p=="~") ) then -- auto home expansion
		p=wpath.join( wpath.home , p:sub(2) )
	end

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

--[[#lua.wetgenes.path.dir

	local dir=wpath.dir(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the dir component of the result.

this will probably end in a / so you may want to call unslash as well 
to remove any trailing slashes and create a more valid path for this 
directory. Note this will transform a root dir of "/" into one of "".

]]
wpath.dir=function(...)
	return wpath.parse( wpath.resolve(...) ).dir
end

--[[#lua.wetgenes.path.file

	local file=wpath.file(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the file component of the result.

]]
wpath.file=function(...)
	return wpath.parse( wpath.resolve(...) ).file
end

--[[#lua.wetgenes.path.root

	local root=wpath.root(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the root component of the result.

]]
wpath.root=function(...)
	return wpath.parse( wpath.resolve(...) ).root
end

--[[#lua.wetgenes.path.folder

	local folder=wpath.folder(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the folder component of the result.

]]
wpath.folder=function(...)
	return wpath.parse( wpath.resolve(...) ).folder
end

--[[#lua.wetgenes.path.name

	local name=wpath.name(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the name component of the result.

]]
wpath.name=function(...)
	return wpath.parse( wpath.resolve(...) ).name
end

--[[#lua.wetgenes.path.ext

	local ext=wpath.ext(path)

This is a small utility function to perform a wpath.resolve then 
wpath.parse and return the ext component of the result.

]]
wpath.ext=function(...)
	return wpath.parse( wpath.resolve(...) ).ext
end

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
		wpath.home=os.getenv("USERPROFILE")

	elseif flavour == "nix" then

		wpath.root="/"
		wpath.separator="/"
		wpath.delimiter=";"
		wpath.winhax=true
		wpath.home=os.getenv("HOME")

	end
	
	if wpath.home then
		wpath.home=wpath.resolve(wpath.home)
	end

end


--[[#lua.wetgenes.path.dir_exists

This function requires a working lfs.

given a dirname return true if it exists and is a dir

]]
wpath.dir_exists=function(p)
	assert(lfs)
	if p:sub(-1)=="/" then p=p:sub(1,-2) end -- auto remove trailing slash
	local a=lfs.attributes(p)
	return a and a.mode=="directory"
end

--[[#lua.wetgenes.path.file_exists

This function requires a working lfs.

given a filename return true if it exists and is a file

]]
wpath.file_exists=function(p)
	assert(lfs)
	local a=lfs.attributes(p)
	return a and a.mode=="file"
end

--[[#lua.wetgenes.path.create_dirs

This function requires a working lfs.

given a filename make sure that its containing directory exists, we 
will work our way up the path creating missing directories.

Note you must include a filename or end with a /

]]
wpath.create_dirs=function(n)
	assert(lfs)
	local t={}
	for w in string.gmatch(n, "[^/]+") do t[#t+1]=w end
	local s=""
	if n:sub(1,1)=="/" then s="/" end -- start with slash
	t[#t]=nil -- remove the filename
	for i,v in ipairs(t) do
		s=s..v
		if not wpath.dir_exists(s) then
			lfs.mkdir(s)
		end
		s=s.."/"
	end
end



-- finally setup paths
wpath.setup()
