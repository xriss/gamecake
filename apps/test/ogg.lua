#!../../../bin/dbg/lua

local core=require("wetgenes.ogg.core")

print(core)

local fp=io.open("test.ogg","r")
local fw=io.open("test.raw","w")

local og=core.create()

local function push()
	local b=(fp:read(4096))
	if b then
		core.push(og,b)
		print("pushed ",#b)
	end
end

core.open(og)
push()

while true do

	local r=core.pull(og)
	if not r then
		local err=core.info(og).err
		if err=="push" then push() err=nil end
		if err then error( err ) end
	else
		print("decoded",#r)
		fw:write(r)
		local err=core.info(og).err
		if err then error( err ) end
	end

end




