

-- android startup so we do android things before anything else happens


local apps=require("apps")
-- try hard to find any files wemay need
apps.default_paths()

-- need to know about the system we are running on
local SDL=require("SDL")
--local platform=SDL.getPlatform()

SDL.log("START ANDROID")

-- replace print so we send print output to SDL.log	
print=function(...)
	local t={...}
	for i=1,#t do t[i]=tostring(t[i]) end
	SDL.log( table.concat(t,"\t") )
end

os.exit=function()print("os.exit() IN ANDROID IS DISABLED") return error("os.exit") end
