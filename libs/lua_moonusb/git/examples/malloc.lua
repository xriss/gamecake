#!/usr/bin/env lua
-- MoonUSB example: malloc.lua

local usb = require("moonusb")

usb.trace_objects(true) -- trace creation/deletion of objects
local ctx = usb.init()

-- We need a device to try out dma memory allocation, so replace these values
-- with those from one of your devices:
local vendor_id, product_id = 0x046d, 0xc534

local device, devhandle = ctx:open_device(vendor_id, product_id)
local mem1 = usb.malloc(nil, 256)       -- this allocates normal host memory
local mem2 = usb.malloc(devhandle, 256) -- this attempts to allocate dma memory
print("1 - closing the device")
devhandle:close() -- mem2 is released with the devhandle closure
print("2 - exiting")
-- mem1 is automatically released at exit, unless one calls mem1:free() on it

