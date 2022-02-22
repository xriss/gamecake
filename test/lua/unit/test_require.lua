

local test={ modname=(...) } package.loaded[test.modname]=test

local n={
	"kissfft.core","wetgenes.pack.core","zip","zlib",
	"wetgenes.freetype.core","wetgenes.ogg.core","al.core",
	"alc.core","wetgenes.tardis.core","gles.core","wetgenes.grd.core",
	"wetgenes.grdmap.core","wetgenes.sod.core","socket.core",
	"mime.core","wetgenes.gamecake.core","wetgenes.win.core","lfs",
	"lsqlite3","posix_c","lash","SDL","cmsgpack","periphery",
	"wetgenes.v4l2.core","rex_pcre","linenoise","brimworks_zip",
	"sys","sys.sock","pgsql","wetgenes.opus.core"}


for i,v in ipairs(n) do

	local req=v
	local name="test_"..req:gsub("%.","_")

	test[name]=function()
		local t=require(req)
	end

end


return test
