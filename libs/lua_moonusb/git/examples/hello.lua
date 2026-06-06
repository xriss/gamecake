#!/usr/bin/env lua
-- MoonUSB example: hello.lua
local usb = require("moonusb")

local ctx = usb.init()

local devices = ctx:get_device_list()

for i, dev in ipairs(devices) do
   local descr = dev:get_device_descriptor()
   local devhandle = dev:open()
   print(string.format("USB %s - bus:%d port:%d %.4x:%.4x %s %s (%s)",
      descr.usb_version, dev:get_bus_number(), dev:get_port_number(),
      descr.vendor_id, descr.product_id,
      devhandle:get_string_descriptor(descr.manufacturer_index) or "???",
      devhandle:get_string_descriptor(descr.product_index) or "???",
      descr.class))
   devhandle:close()
   dev:free()
end

