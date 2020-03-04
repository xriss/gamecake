
-- the first successful require is returned as this module

for i,v in ipairs{

	"lfs_ffi",
	"lfs",

} do

	local M=select(2,pcall( function() return require(v) end ))

	if M then return M end

end

