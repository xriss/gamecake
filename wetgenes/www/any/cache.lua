
if ngx then
	return require("wetgenes.www.ngx.cache")
else
	return require("wetgenes.www.gae.cache")
end
