
local apps=require("apps")

-- try hard to find any files wemay need
apps.default_paths()


-- we expect to find the initial file here

local name="lua/init.lua"
local fp=assert(io.open(name))
local str=fp:read("*a")
fp:close()

if str:sub(1,2)=="#!" then
	str="--"..str -- ignore hashbang on first line
end
local func=assert(loadstring(str,name))

-- finally call with args
func(...)

os.exit(0) -- so that we do not end up at a console?
