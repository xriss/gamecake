if ngx then
	return require("wetgenes.www.sqlite.data")
else
	return require("wetgenes.www.gae.data")
end
