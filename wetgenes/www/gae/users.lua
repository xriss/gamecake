
local json=require("wetgenes.json")

local dat=require("wetgenes.www.any.data")
local cache=require("wetgenes.www.any.cache")

local log=require("wetgenes.www.any.log").log -- grab the func from the package

local fetch=require("wetgenes.www.any.fetch")
local sys=require("wetgenes.www.any.sys")


local core=require("wetgenes.www.gae.users.core")

local os=os
local string=string
local math=math

local tostring=tostring
local type=type
local ipairs=ipairs
local require=require

local wet_string=require("wetgenes.string")
local str_split=wet_string.str_split
local serialize=wet_string.serialize

module(...)

function login_url(a)

	return core.login_url(a)

end


function logout_url(a)

	return core.logout_url(a)

end

function get_google_user()

	return core.get_google_user()

end
