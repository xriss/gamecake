if ngx then
	return require("wetgenes.www.ngx.users")
else
	return require("wetgenes.www.gae.users")
end
