
if ngx then
	return require("wetgenes.www.any").wrap_module("wetgenes.www.ngx.users",...)
else
	return require("wetgenes.www.any").wrap_module("wetgenes.www.gae.users",...)
end
