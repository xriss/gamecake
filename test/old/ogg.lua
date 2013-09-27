#!../../../bin/dbg/lua

local ogg=require("wetgenes.ogg")

local fp=io.open("test.ogg","r")
local fw=io.open("test.raw","w")

local og=ogg.create():open()

local function push()
	local b=(fp:read(4096))
	if b then
		og:push(b)
--		print("pushed ",#b)
	end
end

push()

while true do

	local r=og:pull()
	if not r then
		if og.err=="push" then push() og.err=nil end
		if og.err then error( og.err ) end
	else
--		print("decoded",#r)
		fw:write(r)
		if og.err=="end" then break end
		if og.err then error( og.err ) end
	end

end




