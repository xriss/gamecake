#!/usr/bin/env lua

-- Classic "Hello, world" demonstration program

local polarssl = require"polarssl"

local stdout = io.stdout
local format = string.format


local str = "Hello, world!"
local digest = polarssl.hash_data("MD5", str)

stdout:write("\n  MD5('", str, "') = ", digest, "\n\n")

