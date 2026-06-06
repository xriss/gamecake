#!/usr/bin/env lua
-- MoonUSB example: gamepads.lua
--
-- This example uses the hotplug and asynchronous IO APIs to handle concurrently one or
-- more gamepads of a known type (vendor id + product id).
-- It detects when a new device of the given type is plugged in, and then continuously
-- retrieves reports from it and prints them, until the device is unplugged. 
-- Note that here we use hardcoded information, such as the address of the endpoint
-- that sends the reports, or the report length. We know this information in advance
-- because we limit ourself to a known product, otherwise we would have to learn it
-- from the device descriptors. This would involve:
-- 1) Getting the device and configuration descriptors (these have already been retrieved
--    and cached by libusb during enumeration, so no transfer is needed).
-- 2) Learning from them the needed information, including the length of the HID report
--    descriptor.
-- 3) Submitting a standard 'get descriptor' control transfer to retrieve the HID report
--    descriptor.
-- 4) Parsing the HID report descriptor to learn the length and meaning of the reports
--    delivered (and/or expected) by the device.
--
-- I wrote this example for a type of gamepad of which I happen to have a couple around
-- (I'm not even a gamer, but fortunately there are some in my family :-). Hopefully, it
-- should work also for similar gamepads from different vendors, by appropriately setting
-- the upper-case variables that follow:
local VENDOR_ID = 0x0079   -- DragonRise Inc.
local PRODUCT_ID = 0x0006  -- PC TWIN SHOCK Gamepad
local ITF_NO = 0           -- interface number
local EP_INT_IN = 0x81     -- address of the 'interrupt in' endpoint that sends the reports
local REPORT_LEN = 8       -- report length for this device

local usb = require("moonusb")
usb.lock_on_close(true) -- see issues #1 and #2

-- usb.trace_objects(true)
local ctx = usb.init()

-- Table of known gamepad devices, indexed by the device object.
-- The corresponding value is an integer id that we use here to discriminate between the
-- gamepads currently attached. Whenever a device is plugged in, we assign it a new id.
local known_devices = {}
local id = 1 -- next id to assign

local function P(dev, ...)
-- Utility to print a formatted string, prepending the device's id.
   print("[" .. (known_devices[dev] or '?').."] ".. string.format(...))
end

local function hex(bytes)
-- Convert a binary string to a readable string of hexadecimal bytes (eg. "00 21 f3 54")
   local fmt = string.rep("B", #bytes)
   local t = { string.unpack(fmt, bytes) }
   t[#t] = nil -- this entry is the index of the next unread byte, we don't want it!
   for i, x in ipairs(t) do t[i] = string.format("%.2x", x) end
   return table.concat(t, " ")
end

local function start_retrieving_reports(dev, devhandle)
-- Allocates memory for a report, and submits an interrupt transfer.
-- By returning true at the end of the callback, the interrupt transfer will
-- be resubmitted again and again.
   local mem = usb.malloc(devhandle, REPORT_LEN)
   devhandle:submit_interrupt_transfer(EP_INT_IN, mem:ptr(), REPORT_LEN, 2000,
      function(transfer, status) -- the callback
         if status ~= 'completed' then
            P(dev, "transfer status: "..status)
            mem:free()
            return false -- delete this transfer
         end
         -- Get and print the received report. A real application would instead
         -- interpret it (as per the HID report descriptor) and process the
         -- information it contains (axes movements, button presses, etc).
         local bytes = mem:read(0, REPORT_LEN)
         P(dev, "report %s", hex(bytes))
         return true -- submit again
      end)
end

-- Hotplug callbacks

local function attached(ctx, dev, event)
   local descr = dev:get_device_descriptor()
   if descr.vendor_id == VENDOR_ID and descr.product_id == PRODUCT_ID then
      known_devices[dev] = id
      id = id + 1
      P(dev, "Attached")
      local devhandle = dev:open()
      local itf = devhandle:claim_interface(ITF_NO)
      start_retrieving_reports(dev, devhandle)
   else -- we are not interested in this device, so we free the object
      dev:free()
   end
end

local function detached(ctx, dev, event)
   local descr = dev:get_device_descriptor()
   if known_devices[dev] then
      P(dev, "Detached")
      -- Delete the device object. This also gracefully deletes any related object
      -- that is still around (interface, devhandle, hostmem, transfers, and so on...)
      dev:free()
      known_devices[dev] = nil
   else -- we are not interested in this device, so we free the object
      dev:free()
   end
end

ctx:hotplug_register("attached", attached, true)
ctx:hotplug_register("detached", detached)

-- The event loop
while true do
   ctx:handle_events(0) 
   -- ...
   -- draw frames etc
   -- ...
end

