
dofile("../version.lua")

-- this is a premake file
-- it sticks all of the current lua sources into a .c file for internal packing

mod_files={}

--[[
function dofilename(i,v)
	local m=v:sub(v:find("/lua/")+5,-1) -- strip upto this starting part of the path (mostly works)
	m=m:sub(1,-5):gsub("/",".") -- remove tail and replace / with .
	if m:sub(-5,-1)==".init" then m=m:sub(1,-6) end -- special init.lua case
	mod_files[m]=v
--	print(m,v)
end
]]


function dorawfilename(i,v)
	local m=v:sub(v:find("/lua/")+5,-1) -- strip upto this starting part of the path (mostly works)
	m="lua/"..m
	mod_files[m]=v
end

if os.matchfiles then -- premake

for i,v in ipairs( os.matchfiles("../../../lua/**") or {} ) do dorawfilename(i,v) end

else -- standalone rock run

	local lfs=require("lfs")

	local dodir ; dodir=function(d)
		if lfs.attributes(d) then -- only if dir exists
			for v in lfs.dir(d) do
				local n=d.."/"..v
				local a=lfs.attributes(n)
				if a and a.mode=="file" then
					dorawfilename(0,n)
				end
				if a and a.mode=="directory" then
					if v:sub(1,1)~="." then
						dodir(n)
					end
				end
			end
		end
	end
	dodir("../../../lua")

end

local readfile=function(name)
	local fp=assert(io.open(name,"r"))
	local d=fp:read("*all")
	fp:close()
	return d
end

	local version=GAMECAKE_VERSION
	local buildtime=os.date(" %Y-%m-%d %H:%M:%S")


	local fp=io.open("cache.c","w")
	local t={}
	local function put(s)
		t[#t+1]=s
	end
	
	local libnames={}
	
	for i,v in ipairs(lua_lib_loads or {}) do
		libnames[#libnames+1]=v[1]
	end
	
	libnames=table.concat(libnames," ")

	local srcnames={}

	for i,v in pairs(mod_files) do
		srcnames[#srcnames+1]=i
	end
	
	srcnames=table.concat(srcnames," ")
	
	if lua_lib_loads then --premake will have set this list
	put([[

	const char *wetgenes_wetmods_version()
	{
		return "Gamecake V]]..version..buildtime..[[ https://github.com/xriss/gamecake containing ]]..libnames..[[";
	}

]])
	else
	put([[

	const char *wetgenes_wetmods_version()
	{
		return "Gamecake V]]..version..buildtime..[[ https://github.com/xriss/gamecake rock";
	}

]])
	end
	
	put([[

const char* wetgenes_cache_lua_modnames[]={

]])

	for n,v in pairs(mod_files) do
	
	local m=n

	if m:sub(-4)==".lua" then -- only lua files
		local m=m:sub(m:find("lua/")+4,-1) -- strip upto this starting part of the path (mostly works)
		m=m:sub(1,-5):gsub("/",".") -- remove tail and replace / with .
		if m:sub(-5,-1)==".init" then m=m:sub(1,-6) end -- special init.lua case

--print(m,n)


		put(string.format([[
%q,%q,

]],m,n))
		end

	end

	put([[

0,0};

]])


	put([[

const char* wetgenes_cache_lua_files[]={

]])

	for n,v in pairs(mod_files) do
	
	local d=readfile(v)

--	d=string.format("%q",d):gsub("\n","n")
--	d=d:gsub("\\n","\\n\"\n\"")

	d=d:gsub("\\","\\\\")
	d=d:gsub("\"","\\\"")
	d=d:gsub("\n","\\n\\\n")

	put(string.format([[
%q,"%s",

]],n,d))

	end

	put([[

0,0};

]])

	fp:write(table.concat(t))
	fp:close()
