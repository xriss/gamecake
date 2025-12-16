	files { "cache.c" , "cache_funcs.c" }


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
--	print(m,v)
end

for i,v in ipairs( os.matchfiles("../lua/**") or {} ) do dorawfilename(i,v) end


for i,v in ipairs( LUA_CACHE_FILES or {} ) do
	for i,v in ipairs( os.matchfiles(v) or {} ) do dofilename(i,v) end
end


local readfile=function(name)
	local fp=assert(io.open(name,"r"))
	local d=fp:read("*all")
	fp:close()
	return d
end

function version_from_time(t,vplus)

	vplus=vplus or 0 -- slight tweak if we need it

	t=t or os.time()

	local d=os.date("*t",t)

-- how far through the year are we
	local total=os.time{year=d.year+1,day=1,month=1} - os.time{year=d.year,day=1,month=1}
	local part=t - os.time{year=d.year,day=1,month=1}

-- build major and minor version numbers
	local maj=math.floor(d.year-2000)
	local min=math.floor((part/total)*1000)+vplus

	if min>=1000 then min=min-1000 maj=maj+1 end -- paranoia fix

	return string.format("%02d.%03d",maj,min)
end

	local version=GAMECAKE_VERSION or version_from_time()
	local buildtime=os.date(" %Y-%m-%d %H:%M:%S")


	local fp=io.open("cache.c","w")
	local t={}
	local function put(s)
		t[#t+1]=s
	end
	
	local libnames={}
	
	for i,v in ipairs(lua_lib_loads) do
		libnames[#libnames+1]=v[1]
	end
	
	libnames=table.concat(libnames," ")

	local srcnames={}

	for i,v in pairs(mod_files) do
		srcnames[#srcnames+1]=i
	end
	
	srcnames=table.concat(srcnames," ")
	
	put([[

	const char *wetgenes_wetmods_version()
	{
		return "Gamecake V]]..version..buildtime..[[ https://github.com/xriss/gamecake containing ]]..libnames..[[";
	}

]])

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
