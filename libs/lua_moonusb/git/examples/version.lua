#!/usr/bin/env lua
-- MoonUSB example: version.lua

local usb = require("moonusb")

print(_VERSION)
print(usb._VERSION)
print(usb._LIBUSB_VERSION)
print(usb.get_version())

usb.setlocale("en")

print("Supported capabilities:")
for _, cap in ipairs(usb.enum("capability")) do
   local has = usb.has_capability(cap)
   print(cap, has and "yes" or "no")
end

local ctx = usb.init()
usb.set_option(ctx, "log level", "warning")
ctx:exit()

