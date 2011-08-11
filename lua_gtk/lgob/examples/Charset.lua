#! /usr/bin/env lua

require("lgob.gobject")

local strup, strdown = glib.utf8_strup, glib.utf8_strdown
local validate, normalize = glib.utf8_validate, glib.utf8_normalize
local ALL = glib.NORMALIZE_ALL

local msg = "Hello sad³²³sßæðßæ®³®³¢↓e Á World!"
print("Message: ", msg)
print("Valid UTF8?", validate(msg)) 
print("Up:", strup(msg)) 
print("Down:", strdown(msg))
print("Equal?", normalize(strup(strdown(msg)), ALL) == normalize(strup(msg), ALL))

local filename = "/usr/bin/lua"
print("Filename:", glib.filename_to_utf8(filename))

local orig = normalize("This is my message! éçè", ALL)
local res = glib.convert(orig, #orig, "UTF8", "ISO-8859-1")
print("Conversion?", normalize(glib.convert(res, #res, "ISO-8859-1", "UTF8"), ALL) == orig)
