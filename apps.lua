
-- create a global function that can be called to fix lua paths so we can find things
-- unfortunatly you still have to know where this file is first and run a 
-- dofile("thisfile") need to come up with a better plan, possibly preload a module?

local package=package
local require=require
local string=string
local table=table
local ipairs=ipairs
local print=print
local os=os
local io=io

module("apps")

function setpaths(dll,dirs)

	local cpath={}
	for i,v in ipairs(dirs) do
		cpath[#cpath+1]=v .. "?." .. dll
		cpath[#cpath+1]=v .. "?/init." .. dll
	end
	cpath[#cpath+1]=package.cpath
	
	local path={}
	for i,v in ipairs(dirs) do
		path[#path+1]=v .. "lua/?.lua"
		path[#path+1]=v .. "lua/?/init.lua"
	end
	path[#path+1]=package.path


	package.cpath=table.concat(cpath,";")
	package.path=table.concat(path,";")

end

--
-- this needs to get more searchy so it can find where the lua app is without any explicit values
--
function find(name)

	local lfs=require("lfs")

--
-- get/set current dir
--
	local get_cd=function()

		return string.gsub(lfs.currentdir(),'\\','/')

	end
	local set_cd=function(str)

		lfs.chdir(str)

	end
	local file_exists=function(str)
		local fp=io.open(str,"r")
		if fp then fp:close() return true end
		return false
	end

-- we are looking for a dir/lua/name.lua and dir will be our base dir so look in various places

	local osflavour="win"
	local os_shell=os.getenv("SHELL")
	if os_shell and string.sub(os_shell,1,5)== "/bin/" then -- if your shell is not here then we assume windows...
		osflavour="nix"
	end

	local dll="dll"
	if osflavour=="nix" then dll="so" end

	local dir=get_cd()

	local tdirs={ -- look in these dirs
		dir,
		dir.."/bin",
		dir.."/..",
		dir.."/../bin",
		dir.."/../lua",
		dir.."/../lua/bin",
	}
	local bin_dir=dir.."/"
	for i=1,#tdirs do local v=tdirs[i]
		if file_exists(v.."/lua/apps.lua") then bin_dir=v.."/" break end -- found a bin dir?
	end

print("BIN PATH",bin_dir,dll)


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

	print("apps.exe",exe)
	print("apps.name",name)
	print("apps.dll",dll)
	print("apps.dir",dir)
	print(...)

	return require(appname).start(...)
end


-- make sure we return the module so we can use it in a dofile("apps.lua")
return package.loaded.apps
