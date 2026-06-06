#!/usr/bin/env lua
-- MoonUSB example: describe.lua
-- Describes a device, given its vendor id and product id.

local usb = require("moonusb")

local function fmt(...) return string.format(...) end

local vendor_id = tonumber(arg[1])
local product_id = tonumber(arg[2])
if not vendor_id or not product_id or
   vendor_id & 0x0000ffff ~= vendor_id or product_id & 0x0000ffff ~= product_id then
   print("Usage:   "..arg[0].." vendor_id product_id")
   print("Example: "..arg[0].." 0x0012 0x0034 \n")
   os.exit(true)
end

local function hex(bytes)
-- Convert a binary string to a readable string of hexadecimal bytes
-- (eg. "00 21 f3 54")
   local fmt = string.rep("B", #bytes)
   local t = { string.unpack(fmt, bytes) }
   t[#t] = nil -- this entry is the index of the next unread byte, we don't want it!
   for i, x in ipairs(t) do t[i] = string.format("%.2x", x) end
   return table.concat(t, " ")
end

local function bcd2string(x)
   return string.format("%d%d.%d%d", (x>>12)&0x0f, (x>>8)&0x0f, (x>>4)&0xf, x&0xf)
end

local function decode_hid_descriptor(bytes)
-- Decode a HID descriptor as per section 6.2.1 of the specification
-- "Device Class Definition for Human Interface Devices (HID)" release 1.11
   local len, dt, rel, cc, n, ofs = string.unpack("I1I1I2I1I1", bytes)
   -- Check that it is indeed a HID descriptor of the correct length
   assert(dt == 0x21 and len == (6 + 3*n))
   local desc = {}
   desc.release = bcd2string(rel)
   desc.country_code = cc
   desc.num_descriptors = n
   desc.descriptor = {}
   for i=1, n do
      local t, l, ofs = string.unpack("I1I2", bytes, ofs)
      if t == 0x22 then t = "report"
      elseif t == 0x23 then t = "physical"
      else error("unsexpected hid descriptor type "..t)
      end
      desc.descriptor[i] = { type=t, length=l }
   end
   return desc
end

-- Create a context and try to open the device:
local ctx = usb.init()
local device, devhandle = ctx:open_device(vendor_id, product_id)

-- Get and print the basic properties of the device (these are cached):
local bus = device:get_bus_number()
local port = device:get_port_number()
local ports = device:get_port_numbers()
local address = device:get_address()
local speed = device:get_speed()

print("Device properties")
print("  bus number: "..bus)
print("  port number: "..port)
print("  port path: ".. (#ports>0 and table.concat(ports, ', ') or "n.a."))
print("  address: "..address)
print("  speed: "..speed)

-- Get the device descriptor (also cached) and print its fields 
local desc = device:get_device_descriptor()
print("Device descriptor")
print("  usb_version: "..desc.usb_version)
print("  class: "..desc.class)
print("  subclass: "..desc.subclass)
print("  protocol: "..desc.protocol)
print("  vendor_id: "..fmt("0x%.4x", desc.vendor_id))
print("  product_id: "..fmt("0x%.4x", desc.product_id))
print("  release_number: "..desc.release_number)
print("  num_configurations: "..desc.num_configurations)

-- The descriptor contains indices of string descriptors.
-- For any non-zero index, we should be able to retrieve the corresponding string.
local manufacturer, product, serial_number = "???", "???", "???"
if desc.manufacturer_index ~= 0 then 
   manufacturer = devhandle:get_string_descriptor(desc.manufacturer_index)
end
print("  manufacturer: "..manufacturer)
if desc.product_index ~= 0 then 
   product = devhandle:get_string_descriptor(desc.product_index)
end
print("  product: "..product)
if desc.serial_number_index ~= 0 then 
   serial_number = devhandle:get_string_descriptor(desc.serial_number_index)
end
print("  serial number: "..serial_number)

-- Get and print the configuration(s)
for _, conf in ipairs(desc.configuration) do
   print("Configuration")
   print("  value: "..conf.value)
   print("  index: "..conf.index)
   print("  self_powered: "..tostring(conf.self_powered)) -- boolean are not coerced...
   print("  remote_wakeup: "..tostring(conf.remote_wakeup))
   print("  max_power: "..conf.max_power)
   print("  num_interfaces: "..conf.num_interfaces)
   print("  extra bytes: ".. (conf.extra and hex(conf.extra) or "-"))
   -- Print the interface(s) for this configuration
   for _, alt in ipairs(conf.interface) do
      -- Each entry in conf.interface is a list of 1+ interfacdescriptor's,
      -- which are the 'alternate settings' for the interface.
      for _, itf in ipairs(alt) do
         print("  Interface")
         print("    number: "..itf.number)
         print("    alt_setting: "..itf.alt_setting)
         print("    class: "..itf.class)
         print("    subclass: "..itf.subclass)
         print("    protocol: "..itf.protocol)
         print("    index: "..itf.index)
         print("    num_endpoints: "..itf.num_endpoints)
         if itf.class == 'hid' then
            -- The extra bytes should contain a HID descriptor, so let's decode it
            local hid = decode_hid_descriptor(itf.extra)
            print("    HID descriptor (extra bytes)")
            print("      release: "..hid.release)
            print("      country_code: "..hid.country_code)
            print("      num_descriptors: "..hid.num_descriptors)
            for _, d in ipairs(hid.descriptor) do
               print("        type: "..d.type)
               print("        length: "..d.length)
            end
         else
            print("    extra bytes: ".. (itf.extra and hex(itf.extra) or "-"))
         end
         -- Print the endpoints for this interface
         for _, ep in ipairs(itf.endpoint) do
            print("    Endpoint")
            print("      address: "..fmt("0x%.2x", ep.address))
            print("      number: "..ep.number)
            print("      direction: "..ep.direction)
            print("      transfer_type: "..ep.transfer_type)
            print("      iso_sync_type: "..(ep.iso_sync_type or "n.a."))
            print("      iso_usage_type: "..(ep.iso_usage_type or "n.a."))
            print("      max_packet_size: "..ep.max_packet_size)
            print("      interval: "..ep.interval)
            print("      refresh: "..ep.refresh)
            print("      synch_address: "..fmt("0x%.2x", ep.synch_address))
            print("      extra bytes: ".. (ep.extra and hex(ep.extra) or "-"))
            local comp = ep.ss_endpoint_companion_descriptor
            if comp then 
               print("      Endpoint companion")
               print("        max_burst: ", comp.max_burst)
               print("        attributes: ", comp.attributes)
               print("        bytes_per_interval: ", comp.bytes_per_interval)
            end
         end
      end
   end
end

