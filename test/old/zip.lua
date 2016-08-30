
local wstr=require("wetgenes.string")
local wzips=require("wetgenes.zips")



wzips.add_zip_file("lua.zip")

local test=require("test")

print(wstr.dump(test))


local t=wzips.readfile("data/test1.txt")
print(wstr.dump(t))

local t=wzips.readfile("data/test2.txt")
print(wstr.dump(t))
