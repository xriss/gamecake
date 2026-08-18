

local test={ modname=(...) } package.loaded[test.modname]=test

local n={
	"wetgenes.pack.core","wetgenes.zip","zlib",
	"wetgenes.freetype.core","wetgenes.ogg.core","wetgenes.al.core",
	"wetgenes.alc.core","wetgenes.tardis.core","wetgenes.gles.core","wetgenes.grd.core",
	"wetgenes.grdmap.core","wetgenes.sod.core","socket.core",
	"mime.core","wetgenes.gamecake.core","wetgenes.win.core","lfs",
	"lsqlite3","SDL","cmsgpack",
	"rex_pcre",
	"wetgenes.opus.core",
	"wetgenes.wire.core",
	}

--[[
"kissfft.core",
"posix_c","lash",
"periphery",
"wetgenes.v4l2.core",
"sys","sys.sock","pgsql",
,"linenoise","brimworks_zip"


]]

for i,v in ipairs(n) do

	local req=v
	local name="test_"..req:gsub("%.","_")

	test[name]=function()
		local t=require(req)
	end

end


return test
