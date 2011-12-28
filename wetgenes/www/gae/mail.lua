

local core=require("wetgenes.www.gae.mail.core")

local os=os

module(...)


function send(...)
	return core.send(...)
end
