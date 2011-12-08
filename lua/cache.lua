	files { "cache.c" , "cache_funcs.c" }


-- this is a premake file
-- it sticks all of the current lua sources into a .c file for internal packing

lua_files = os.matchfiles("../bin/lua/**.lua")

mod_files={}
for i,v in ipairs(lua_files) do
	local m=v:sub(12,-1):sub(1,-5):gsub("/",".") -- remove head and tail and replace / with .
	if m:sub(-5,-1)==".init" then m=m:sub(1,-6) end -- special init.lua case
	mod_files[m]=v
--	print(m,v)
end

local readfile=function(name)
	local fp=assert(io.open(name,"r"))
	local d=fp:read("*all")
	fp:close()
	return d
end


	local fp=io.open("cache.c","w")
	local t={}
	local function put(s)
		t[#t+1]=s
	end

	put([[

const char* wetgenes_cache_lua_mods[]={

]])

	for i,v in pairs(mod_files) do
	
	local d=readfile(v)
	d=string.format("%q",d):gsub("\n","n")
	d=d:gsub("\\n","\\n\"\n\"")
	put(string.format([[
%q,%s,

]],i,d))

	end

	put([[

0,0};

]])

	fp:write(table.concat(t))
	fp:close()
