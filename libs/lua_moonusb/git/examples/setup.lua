#!/usr/bin/env lua
-- MoonUSB example: setup.lua
-- Encoding/decoding control setup structures

local usb = require("moonusb")
local ctx = usb.init()

-- Allocate memory where to write/read the setup struct.
-- This struct is 8 bytes long, so we need at least that space,
-- plus the space for data to be transferred.
local mem = usb.malloc(nil, 256)

local function test(setup)
   usb.encode_control_setup(mem:ptr(), setup)
   local setup1 = usb.decode_control_setup(mem:ptr())
   print("--------------")
   for k, v in pairs(setup1) do
      print(k, v)
      -- check that the value is the same as the original
      assert(v == setup[k])
   end
   local bytes = mem:read(0, 8, 'uchar')
   for i, b in ipairs(bytes) do bytes[i] = string.format("%.2x", b) end
   print("hex", table.concat(bytes, ' '))
end

local standard_setup = {
   request_recipient = 'interface',
   request_type = 'standard',
   direction = 'in',
   request = 'get descriptor', -- we must use string literals here, since it's a standard request
   value = 123,
   index = 4,
   length = 128,
   }

local vendor_setup = {
   request_recipient = 'other',
   request_type = 'vendor',
   direction = 'out',
   request = 0xf1, -- since this is not a standard request, this field is numeric (a byte)
   value = 123,
   index = 4,
   length = 128,
   }

test(standard_setup)
test(vendor_setup)

mem:free()

print("--------------")
-- Encoding/decoding to/from binary string
local b = string.pack('I1I1I1I1I1I1I1I1', 0x81, 0x06, 0x00, 0x22, 0x00, 0x00, 0x34, 0x00)
local s = usb.decode_control_setup(b)
for k,v in pairs(s) do print(k, v) end
local b1 = usb.encode_control_setup(nil, s)
assert(b1 == b)
