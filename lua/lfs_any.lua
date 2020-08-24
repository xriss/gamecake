
-- the first successful require is returned as this module

for i,v in ipairs{

	"lfs_ffi",
	"lfs",

} do
	local _,M=pcall( function() return require(v) end ) ; M=_ and M

	if M then return M end

end

error("lfs not found")

