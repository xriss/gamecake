
-- the first successful require is returned as this module

for i,v in ipairs{

	"lfs_ffi2",
	"lfs2",

} do
	local _M,M=pcall( function() return require(v) end ) ; M=_M and M

print(v,M)

	if M then return M end

end

error("lfs not found")

