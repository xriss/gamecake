if ngx then
	return require("wetgenes.www.any").wrap_module("wetgenes.www.sqlite.data",...)
else
	return require("wetgenes.www.any").wrap_module("wetgenes.www.gae.data",...)
end
