#!/usr/bin/env lua
-- MoonUSB example: open_short.lua

local usb = require("moonusb")

usb.trace_objects(true)
local ctx = usb.init()

-- replace this with values from one of your devices:
local vendor_id, product_id = 0x1d6b, 0x0002

local device, devhandle = ctx:open_device(vendor_id, product_id)

