#! /usr/bin/env lua

--[[
	Checksum example
--]]

require("lgob.gobject")

local data = "Testing checksums!"
print( glib.compute_checksum_for_string(glib.CHECKSUM_MD5, data, #data) )
print( glib.compute_checksum_for_string(glib.CHECKSUM_SHA1, data, #data) )
print( glib.compute_checksum_for_string(glib.CHECKSUM_SHA256, data, #data) )
