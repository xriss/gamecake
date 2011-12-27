
if ngx then
	return require("wetgenes.www.any").wrap_module("wetgenes.www.ngx.cache",...)
else
	return require("wetgenes.www.any").wrap_module("wetgenes.www.gae.cache",...)
end
