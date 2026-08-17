
local modnames={
	"kissfft.core","wetgenes.pack.core","zip","zlib",
	"wetgenes.freetype.core","wetgenes.ogg.core","wetgenes.al.core",
	"wetgenes.alc.core","wetgenes.tardis.core","wetgenes.gles.core","wetgenes.grd.core",
	"wetgenes.grdmap.core","wetgenes.sod.core","socket.core",
	"mime.core","wetgenes.gamecake.core","wetgenes.win.core","lfs",
	"lsqlite3","posix_c","lash","SDL","cmsgpack","periphery",
	"wetgenes.v4l2.core","rex_pcre"--[[,"linenoise","brimworks_zip"]],
	"sys","sys.sock","pgsql","wetgenes.opus.core"}


for i,modname in ipairs(modnames) do

	describe( "test require "..modname , function()
		local t=assert.is_true( require(modname) and true )
		
	end)

end
