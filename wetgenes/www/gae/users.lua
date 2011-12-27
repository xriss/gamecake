
local json=require("wetgenes.json")

local dat=require("wetgenes.aelua.data")
local cache=require("wetgenes.aelua.cache")

local log=require("wetgenes.aelua.log").log -- grab the func from the package

local fetch=require("wetgenes.aelua.fetch")
local sys=require("wetgenes.aelua.sys")


local core=require("wetgenes.aelua.users.core")

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

module("wetgenes.aelua.users")

function login_url(a)

	return core.login_url(a)

end


function logout_url(a)

	return core.logout_url(a)

end

function get_google_user()

	return core.get_google_user()

end
