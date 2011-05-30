--+-----------------------------------------------------------------------------------------------------------------+--
--
-- (C) Kriss Daniels 2005 http://www.XIXs.com
--
-- This file made available under the terms of The MIT License : http://www.opensource.org/licenses/mit-license.php
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
--+-----------------------------------------------------------------------------------------------------------------+--


--
-- A thrown together build toool, well some useful lua functions for making a build.
--
-- The intent is to optimise a fullbuild rather than a partial build,
--
-- it is the times you have to rebuild everything that causes you to go make a cup of tea, after all.
--
-- This is exceptionally true of windows where process creation has a huge overhead
--
-- Hopefully things are kept shrinkwrapped enough here to enable an easy unixy port when I need it
--

local lfs=require("lfs")
local table=table
local string=string
local os=os
local io=io
local print=print

local ipairs=ipairs

module("wetgenes.bake")

osflavour="win"
	
local os_shell=os.getenv("SHELL")
if os_shell and string.sub(os_shell,1,5)== "/bin/" then
	osflavour="nix"
end

-- fullpaths to usefull commands

cmd={}


-- place to store options

opt={}




--
-- get/set current dir
--
get_cd=function()

	return string.gsub(lfs.currentdir(),'\\','/')

end
set_cd=function(str)

	lfs.chdir(str)

end


--
-- combine strings and resolve . or .. and cancel out multiple // and switch \ to /
-- so we should end up with a valid clean path
--
path_clean=function(...)

local str

	str=table.concat({...})
	str=string.gsub(str,'\\','/')

	return(str)

end

--
-- as path_clean but add .exe (so we can easily not do this later if under unix)
--
path_clean_exe=function(...)

if osflavour=="nix" then
	return(path_clean(...))
else
	return(path_clean(...)..'.exe')
end

end


--
-- return the substring after the last .
--
path_ext=function(str)

	return(str)

end

--
-- perform some substitutions and then execute the command from the given cwd
--
execute=function(cwd,cmd,arg)

	if cwd then
	
		lfs.chdir(cwd)
	
	end
	
	if arg then
	
		os.execute(cmd..' '..arg)
		
	else
	
		os.execute(cmd)

	end

end

--
-- Copy a file from one location to another
--
copyfile=function(f,t)

local fpr=io.open(f,"rb")
local d=fpr:read("*a")
local fpw=io.open(t,"wb")

	fpw:write(d)
	fpr:close()
	fpw:close()
end


--
-- given a filename make sure that its containing directory exists
--
create_dir_for_file=function(n)
	local t={}
	for w in string.gmatch(n, "[^/]+") do t[#t+1]=w end
	local s=""
	t[#t]=nil -- remove the filename
	for i,v in ipairs(t) do
		s=s..v
		lfs.mkdir(s)
		s=s.."/"
	end
end

--
-- get the filenames (relative to the basedir) of all files matching the filter
--
findfiles=function(opts)
if not opts then return end
if not opts.basedir then return end
if not opts.dir then return end
if not opts.filter then return end
opts.ret=opts.ret or {}

	local subdirs={}
	local d=opts.basedir.."/"..opts.dir
	for v in lfs.dir(d) do
		local a=lfs.attributes(d.."/"..v)
--print("test",v,a.mode)
		if a.mode=="file" then
			if string.find(v,opts.filter) then
--print("found",v)
				opts.ret[#opts.ret+1]=opts.dir.."/"..v
			end
		end
		if a.mode=="directory" then
			if v:sub(1,1)~="." then
				subdirs[#subdirs+1]=v
			end
		end
	end

-- recurse
	local dir=opts.dir
	for i,v in ipairs(subdirs) do
		opts.dir=dir.."/"..v
		findfiles(opts)
	end

	return opts
end






