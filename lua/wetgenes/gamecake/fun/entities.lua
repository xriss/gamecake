
--module had been moved and renamed to 

local deepcopy=require("wetgenes"):export("deepcopy")

local modname=(...)

package.loaded[modname]=deepcopy( require("wetgenes.gamecake.zone.scene") )

-- flag compatibility hax
local old_create=package.loaded[modname].create
package.loaded[modname].create=function(scene)
	scene=scene or {}
	scene.fun64=true
	return old_create(scene)
end
