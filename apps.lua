
wetlua={}
wetlua.exe="exe"
wetlua.app=arg[1]
wetlua.dll="so"
wetlua.dir="../../"..wetlua.app.."/"

print(wetlua.exe)
print(wetlua.app)
print(wetlua.dll)
print(wetlua.dir)

wetlua.path_orig=package.path
wetlua.cpath_orig=package.cpath

wetlua.cpath=	     wetlua.dir .. "?." .. wetlua.dll ..
				";" .. wetlua.dir .. "?/init." .. wetlua.dll .. ";" ..
				package.cpath

wetlua.path=	     wetlua.dir .. "lua/?.lua" ..
				";" .. wetlua.dir .. "lua/?/init.lua" ..
				";../lua/?.lua" ..
				";../lua/?/init.lua" ..
				";"..package.path

package.cpath=wetlua.cpath -- and set paths so we can find things
package.path=wetlua.path


dofile(wetlua.dir.."lua/main.lua") -- start the app
