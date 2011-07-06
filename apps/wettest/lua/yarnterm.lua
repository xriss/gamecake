package.path=package.path..";./?/init.lua"

dbg=function(s)
	local fp=io.open("yarn.dbg","a")
	if fp then
		fp:write(s.."\n")
		io.close(fp)
	end
end
dbg(os.date())

local yarn=require("yarn")
local yarn_strings=require("yarn.strings")

local function keycode(code)

--print(code)

	if code==65  then return "up" end
	if code==66  then return "down" end
	if code==68  then return "left" end
	if code==67  then return "right" end
	if code==32  then return "space" end
	if code==127 then return "backspace" end
	if code==27  then return "esc" end
	if code==10  then return "enter" end

	return ""
end

local aesc=string.char(27) .. '['

local fp=io.open("savedata.lua","r")
local sd
if fp then
	sd=yarn_strings.unserialize(fp:read())
	fp:close()
end

yarn.setup(sd)
yarn.update()
print( aesc.."2J"..aesc.."0;0H"..yarn.draw(2) )

local exit=false
while not exit do

	local key_str=io.stdin:read(1)
	
	local key=keycode( key_str:byte() )
	
	if key_str=="q" then exit=true end

	yarn.keypress(key_str,key,"down")
	yarn.keypress(key_str,key,"up")
	yarn.update()
	print( aesc.."0;0H"..yarn.draw(2) )

end

local sd=yarn.save()
local fp=io.open("savedata.lua","w")
fp:write(yarn_strings.serialize(sd))
fp:close()

