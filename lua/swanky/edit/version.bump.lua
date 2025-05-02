#!/usr/bin/env gamecake 

-- get path to version.lua
local file=arg[0]:gsub("%.bump%.",".")

-- read file and
local fp=assert(io.open(file,"rb"))
local dat=fp:read("*a")
fp:close()
local ret=assert(loadstring(dat))()


local version=ret.version
local va,vb,vc=version:match("(.+)%.(.+)%.(.+)")
local nb=os.date("%y%m%d")

if nb ~= vb then -- no need to bump revision
	vb=nb
	vc=1
else -- only bump revision
	vc=tonumber(vc)+1
end

version=va.."."..vb.."."..vc

local fp=assert(io.open(file,"wb"))
fp:write([[
return {
version="]]..version..[[",
}
]])

fp:close()

print("FILE  :",file)
print("READ  :",ret.version)
print("WRITE :",version)

