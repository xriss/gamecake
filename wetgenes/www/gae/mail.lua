

local core=require("wetgenes.aelua.mail.core")

local os=os

module(...)


function send(...)
	return core.send(...)
end
