
-- create a global function that can be called to fix lua paths so we can find things
-- unfortunatly you still have to know where this file is first and run a 
-- dofile("thisfile") need to come up with a better plan, possibly preload a module?
--
-- yup this module is assumed to be preloaded, afterwhich all other modules can be found
-- and loaded.

local package=package
local require=require
local string=string
local table=table
local ipairs=ipairs
local print=print
local os=os
local io=io
local pcall=pcall

module("apps")

--
-- get/set current dir
--
local get_cd=function()

	local lfs ; pcall( function() lfs=require("lfs") end )

	if lfs then	return string.gsub(lfs.currentdir(),'\\','/') end
	
	return "." -- lfs is not available

end

local set_cd=function(str)
	local lfs=require("lfs")

	lfs.chdir(str)

end

local file_exists=function(str)
	local fp=io.open(str,"r")
	if fp then fp:close() return true end
	return false
end

	
function setpaths(dll,dirs)

	if dll then
		local cpath={}
		for i,v in ipairs(dirs) do
			cpath[#cpath+1]=v .. "?." .. dll
			cpath[#cpath+1]=v .. "?/init." .. dll
		end
		cpath[#cpath+1]=package.cpath
		package.cpath=table.concat(cpath,";")
	end
	
	local path={}
	for i,v in ipairs(dirs) do
		path[#path+1]=v .. "?.lua"
		path[#path+1]=v .. "?/init.lua"
		path[#path+1]=v .. "lua/?.lua"
		path[#path+1]=v .. "lua/?/init.lua"
	end
	path[#path+1]=package.path
	package.path=table.concat(path,";")

end

--
-- find where our exe lives
--
function find_bin()

	local dir=get_cd()
	local exe="."
	local exe_path
	pcall(function() exe_path=require("wetgenes.win.core").get_exe_path() end)
	if exe_path then -- remove filename
		local dir,name,ext=string.match(exe_path,"(.-)([^\\/]-%.?([^%.\\/]*))$")
		exe=dir:sub(1,-2) -- remove trailing / or \
	end

-- print(dir,exe)

	local tdirs={ -- look in these dirs
		dir,
		dir.."/..",
		dir.."/../..",
		exe,
		exe.."/..",
		exe.."/../..",
	}
	local bin_dir=dir.."/"
	for i=1,#tdirs do local v=tdirs[i]
		if file_exists(v.."/lua/apps.lua") then bin_dir=v.."/" break end -- found a bin dir?
	end

	return bin_dir
end

--
-- find our bin dir and set search for all lua files under there, makes debuging a bit easier
-- than using the builtin strings. Also lets us pick up any dlls in there.
--
function default_paths(appdir)

-- we are looking for a dir/lua/name.lua and dir will be our base dir so look in various places

	if not pcall( function() return require("lfs") end ) then return end -- not possible without lfs

	local osflavour="win"
	local os_shell=os.getenv("SHELL")
	if os_shell and string.sub(os_shell,1,5)== "/bin/" then -- if your shell is not here then we assume windows...
		osflavour="nix"
	end

	local dll="dll"
	if osflavour=="nix" then dll="so" end

	local luadir=find_bin()
	
	if appdir and appdir~="" then -- use this as app dir

		appdir=appdir:gsub("\\","/").."../"
		setpaths(dll,{luadir,appdir,"./"})

	else
	
		appdir=get_cd().."/" -- use cd as app dir	
		setpaths(dll,{luadir,"./"})
		
	end
	
	local wzips=require("wetgenes.zips") -- and search for data+lua here
	wzips.paths[#wzips.paths+1]=appdir

	return luadir,appdir
	
end





-- the stuff below is old and should not be used anymore
-- just call default_paths() in your init.lua to make sure we have paths setup OK

--
-- this needs to get more searchy so it can find where the lua app is without any explicit values
--
function find(name)

	local lfs=require("lfs")


-- we are looking for a dir/lua/name.lua and dir will be our base dir so look in various places

	local osflavour="win"
	local os_shell=os.getenv("SHELL")
	if os_shell and string.sub(os_shell,1,5)== "/bin/" then -- if your shell is not here then we assume windows...
		osflavour="nix"
	end

	local dll="dll"
	if osflavour=="nix" then dll="so" end

	local bin_dir=find_bin()
	
print("BIN PATH",bin_dir,dll)


	local dir=get_cd()
	local tdirs={ -- look in these dirs
	
		dir,
		dir.."/"..name,
		dir.."/apps/"..name,
		dir.."/lua/"..name,
		dir.."/lua/apps/"..name,
		
		dir.."/..",
		dir.."/../"..name,
		dir.."/../apps/"..name,
		dir.."/../lua/"..name,
		dir.."/../lua/apps/"..name,
		
		dir.."/../..",
		dir.."/../../"..name,
		dir.."/../../apps/"..name,
		dir.."/../../lua/"..name,
		dir.."/../../lua/apps/"..name,
		
	}
	local app_dir=dir.."/"
	local app_name=name
	for i=1,#tdirs do local v=tdirs[i]
		if file_exists(v.."/lua/"..name..".lua") then app_dir=v.."/" break end -- found a base dir?
		if file_exists(v.."/lua/app.lua") then app_dir=v.."/" app_name="app" break end -- found a base dir?
	end

print("APP PATH",app_dir,dll)

	setpaths(dll,{app_dir,bin_dir})

	return app_dir,app_name
end


-- only call this once
-- probably on the commandline
function start(_name,...)

	path_orig=package.path
	cpath_orig=package.cpath

	exe="exe"
	name=_name
	dll="so"

	dir,appname=find(name)

	path=package.path
	cpath=package.cpath
	
	args=(...)
	
	print("appname ",appname)
	print("apps.exe",exe)
	print("apps.name",name)
	print("apps.dll",dll)
	print("apps.dir",dir)
	print("apps.args",args)

	return require(appname).start(...)
end
