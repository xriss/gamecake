#!/usr/bin/lua

local lash = require('lash')

-- do hashes on either the 
-- first command line argument
-- or this file itself
local val = arg[1] or arg[0]

-- MD5 functions
print('MD5')
print(lash.MD5.file2hex(val))
print(lash.MD5.string2hex(val))
print('')

-- CRC32 functions
print('CRC32')
print(lash.CRC32.string2hex(val))
print(lash.CRC32.string2num(val))
print(lash.CRC32.file2hex(val))
print(lash.CRC32.file2num(val))
print('')

-- SHA1 functions
print('SHA1')
print(lash.SHA1.string2hex(val))
print(lash.SHA1.file2hex(val))
print('')
