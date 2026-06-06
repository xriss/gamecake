#!/usr/bin/env lua
-- MoonUSB example: hotplug.lua
local usb = require("moonusb")

--usb.trace_objects(true)
local ctx = usb.init()

local devices = ctx:get_device_list()

local function cb_plugged(ctx, device, event)
   local descr = device:get_device_descriptor()
   print("device", device, event)
   print(string.format("USB %s - bus:%d port:%d %.4x:%.4x (%s)",
      descr.usb_version, device:get_bus_number(), device:get_port_number(),
      descr.vendor_id, descr.product_id, descr.class))
   device:free()
end

local function cb_unplugged(ctx, device, event)
   print("device", device, event)
   device:free() -- free any device you are not interested in keeping around!
end

local hp = ctx:hotplug_register("attached", cb_plugged, true)
local hp = ctx:hotplug_register("detached", cb_unplugged)

while true do
   ctx:handle_events(0)
end

