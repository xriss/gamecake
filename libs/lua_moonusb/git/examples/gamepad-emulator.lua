#!/usr/bin/env lua
-- MoonUSB example: gamepad-emulator.lua
--
-- Gamepad Device Emulator
-- ----------------------------------------------------------------------------
-- This example uses the moonusb.emulator module to emulate a gamepad device.
-- The emulated gamepad is of the same type as expected by the gamepads.lua
-- example, that can therefore be used as client to test this fake gamepad. 
--
-- Usage
-- ----------
--
--    First of all, ensure that the needed kernel modules are loaded:
--    $ sudo modprobe usbip-core
--    $ sudo modprobe vhci-hcd
--
--    Launch the emulator. It will open a tcp port and listen for client connects.
--    $[examples]./gamepad-emulator.lua
--
--    From another shell, list the devices exported by the script, using the
--    usbip tool:
--    $ usbip list -r 127.0.0.1
--
--    This command should list a single device, and indicate "4-5" as its busid
--    (the busid is composed as busnum-devnum).
--
--    Import (or attach) the device, again with the usbip tool:
--    $ sudo usbip attach -r 127.0.0.1 -b 4-5
--
--    This should start the configuration of the device, with the vhci driver
--    issuing commands (get descriptor, etc) and the fake device responding.
--    At the end of this phase, the fake device should appear in the list of
--    USB devices available on the system. To see this list, use the lsusb tool:
--    $ lsusb
--
--    Now, from yet another shell, launch the client application:
--    $[examples]./gamepads.lua
--
--    If everything goes as expected, the client should end up receiving the fake
--    HID reports from the fake driver (in this case, a report is 8 bytes long,
--    with values from 0x01 to 0x08).
--
--    To detach the fake device, first see its 'port' number (likely 00), then
--    then issue the detach command:
--    $ sudo usbip port                # list the imported devices
--    $ sudo usbip detach -p 00        # detach Port 00
--
--    (Or simply send a SIGINT (ctl-C) to the gamepad-emulator.lua script, which
--    should be equivalent to unplugging the device).
--
local usb = require("moonusb")
local emulator = require("moonusb.emulator")
-- local timers = require("moontimers")

-- Utilities
local fmt = string.format
local function printf(...) io.write(fmt(...)) end
local rep, pack, unpack = string.rep, string.pack, string.unpack
local doubleface = usb.doubleface
local hex, bcd2str, str2bcd = usb.hex, usb.bcd2str, usb.str2bcd
local zeropad, packbytes, unpackbytes = usb.zeropad, usb.packbytes, usb.unpackbytes 
local send_submit_response = emulator.send_submit_response
local send_unlink_response = emulator.send_unlink_response

local cfg = {
-- usbip_ver = "01.11",
   busnum = 4,
   devnum = 5,
   vendor_id = 0x0079,  -- DragonRise Inc.
   product_id = 0x0006, -- PC TWIN SHOCK Gamepad
   release_number = '00.00',
   device_class = 'per interface',
   device_subclass = 0,
   device_protocol = 0,
   configuration_value = 1,
   num_configurations = 1,
   interfaces = {{class='hid', subclass=0, protocol=0}}
}

-------------------------------------------------------------------------------
-- Descriptors
-------------------------------------------------------------------------------
local devicedescriptor = packbytes{
   0x12, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x79, 0x00,
   0x06, 0x00, 0x07, 0x01, 0x01, 0x02, 0x00, 0x01
}

local interfacedescriptor = packbytes{0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00}
local hiddescriptor = packbytes{0x09, 0x21, 0x10, 0x01, 0x21, 0x01, 0x22, 0x65, 0x00}
local epin1descriptor = packbytes{0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0a}
local epout1descriptor = packbytes{0x07, 0x05, 0x01, 0x03, 0x08, 0x00, 0x0a}

local configdescriptor = table.concat{
   packbytes{ 0x09, 0x02, 0x29, 0x00, 0x01, 0x01, 0x00, 0x80, 0xfa },
   interfacedescriptor,
   hiddescriptor, 
   epin1descriptor,
   epout1descriptor
}

local stringdescriptor = {
   [0] = packbytes{ -- language
         0x04, 0x03, 0x09, 0x04  },
   [1] = packbytes{ -- manufacturer
         0x24, 0x03, 0x44, 0x00, 0x72, 0x00, 0x61, 0x00, 0x67, 0x00,
         0x6f, 0x00, 0x6e, 0x00, 0x52, 0x00, 0x69, 0x00, 0x73, 0x00,
         0x65, 0x00, 0x20, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x63, 0x00,
         0x2e, 0x00, 0x20, 0x00, 0x20, 0x00 },
   [2] = packbytes{ -- product
         0x34, 0x03, 0x47, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x65, 0x00,
         0x72, 0x00, 0x69, 0x00, 0x63, 0x00, 0x20, 0x00, 0x20, 0x00,
         0x20, 0x00, 0x55, 0x00, 0x53, 0x00, 0x42, 0x00, 0x20, 0x00,
         0x20, 0x00, 0x4a, 0x00, 0x6f, 0x00, 0x79, 0x00, 0x73, 0x00,
         0x74, 0x00, 0x69, 0x00, 0x63, 0x00, 0x6b, 0x00, 0x20, 0x00,
         0x20, 0x00 },
}

local hidreportdescriptor = packbytes{
   0x05, 0x01, 0x09, 0x04, 0xa1, 0x01, 0xa1, 0x02, 0x75, 0x08,
   0x95, 0x05, 0x15, 0x00, 0x26, 0xff, 0x00, 0x35, 0x00, 0x46,
   0xff, 0x00, 0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x32,
   0x09, 0x35, 0x81, 0x02, 0x75, 0x04, 0x95, 0x01, 0x25, 0x07,
   0x46, 0x3b, 0x01, 0x65, 0x14, 0x09, 0x39, 0x81, 0x42, 0x65,
   0x00, 0x75, 0x01, 0x95, 0x0c, 0x25, 0x01, 0x45, 0x01, 0x05,
   0x09, 0x19, 0x01, 0x29, 0x0c, 0x81, 0x02, 0x06, 0x00, 0xff,
   0x75, 0x01, 0x95, 0x08, 0x25, 0x01, 0x45, 0x01, 0x09, 0x01,
   0x81, 0x02, 0xc0, 0xa1, 0x02, 0x75, 0x08, 0x95, 0x07, 0x46,
   0xff, 0x00, 0x26, 0xff, 0x00, 0x09, 0x02, 0x91, 0x02, 0xc0,
   0xc0,
}

-------------------------------------------------------------------------------
-- USB protocol
-------------------------------------------------------------------------------
-- References
-- [USB2] USB 2.0 Specification
-- [HID]  Device Class Specification for Human Interface Devices (HID) ver 1.11

-- Request codes (0xTTRR where TT=bmRequestType, RR=bmRequest)
local REQ = doubleface{
   -- Standard requests ([USB2]/9.4)
   ['CLEAR_FEATURE_DEV']   = 0x8001,
   ['CLEAR_FEATURE_ITF']   = 0x8101,
   ['CLEAR_FEATURE_EP']    = 0x8201,
   ['GET_CONFIGURATION']   = 0x8008,
   ['GET_DESCRIPTOR']      = 0x8006,
   ['GET_DESCRIPTOR_ITF']  = 0x8106, -- see [HID]/7.1.1
   ['GET_INTERFACE']       = 0x810a,
   ['GET_STATUS_DEV']      = 0x8000,
   ['GET_STATUS_ITF']      = 0x8100,
   ['GET_STATUS_EP']       = 0x8200,
   ['SET_ADDRESS']         = 0x0005,
   ['SET_CONFIGURATION']   = 0x0009,
   ['SET_DESCRIPTOR']      = 0x0007,
   ['SET_DESCRIPTOR_ITF']  = 0x0107, -- see [HID]/7.1.2
   ['SET_FEATURE_DEV']     = 0x0003,
   ['SET_FEATURE_ITF']     = 0x0103,
   ['SET_FEATURE_EP']      = 0x0203,
   ['SET_INTERFACE']       = 0x010b,
   ['SYNCH_FRAME']         = 0x020c,
   -- HID-class requests ([HID]/7)
   ['GET_REPORT']          = 0xa101,
   ['SET_REPORT']          = 0x2109,
   ['GET_IDLE']            = 0xa102,
   ['SET_IDLE']            = 0x210a,
   ['GET_PROTOCOL']        = 0xa103,
   ['SET_PROTOCOL']        = 0x210b,
}

local DTYPE = doubleface{ -- descriptor types
   -- Standard descriptors ([USB2]/table 9.5)
   ['device'] = 1,
   ['configuration'] = 2,
   ['string'] = 3,
   ['interface'] = 4,
   ['endpoint'] = 5,
   ['device qualifier'] = 6,
   ['other speed configuration'] = 7,
   ['interface power'] = 8,
   -- HID class descriptors ([HID]/7.1
   ['hid'] = 0x21,
   ['report'] = 0x22,
   ['physical'] = 0x23,
}

local handle_submit = {} -- table of functions indexed by request name

handle_submit['GET_DESCRIPTOR'] = function(submit, req_code, req_name) -- [USB2]/9.4.3
   local dindex, dtype = unpack('I1I1', submit.setup, 3)
   local what = DTYPE[dtype]
   local data 
   if what == 'device' then data = devicedescriptor
   elseif what == 'configuration' then data = configdescriptor
   elseif what == 'string' then data = stringdescriptor[dindex]
   elseif what == 'hid' then data = hiddescriptor
   elseif what == 'report' then data = hidreportdescriptor
   end
   if data then
      send_submit_response(submit, 0, 0, data)
   else
      printf("unknown or unsupported descriptor type = %d\n", dtype)
   end
end

handle_submit['GET_DESCRIPTOR_ITF'] = handle_submit['GET_DESCRIPTOR'] -- [HID]/7.1.1

handle_submit['SET_CONFIGURATION'] = function(submit, req_code, req_name) -- [USB2]/9.4.7
   local value = unpack('I1', submit.setup, 3)
   if value == cfg.configuration_value then
      send_submit_response(submit, 0, 0, nil)
   else
      printf("invalid configuration value %d\n", value)
   end
end

handle_submit['SET_IDLE'] = function(submit, req_code, req_name) -- [HID]/7.2.4
   send_submit_response(submit, 0, 0, nil)
end

local function receive_control(submit)
   local req_code = unpack('>I2', submit.setup)
   local req_name = REQ[req_code]
   if not req_name then
      printf("received unknown request (setup: %s)\n", hex(submit.setup))
      return
   end
   local f = handle_submit[req_name]
   if not f then
      printf("received unsupported %s (setup: %s)\n", req_name, hex(submit.setup))
      return
   end
   printf("received %s (setup: %s)\n", req_name or "???", hex(submit.setup))
   return f(submit)
end

local fakereport = packbytes{ 1, 2, 3, 4, 5, 6, 7, 8 }
local function receive_endpoint1(submit)
   if submit.direction == 'in' then
      send_submit_response(submit, 0, 0, fakereport)
   elseif submit.direction == 'out' then
      printf("received report\n") -- @@ ??
      send_submit_response(submit, 0, 0, nil)
   end
end

local function receive_submit(submit)
   if submit.ep == 0 then return receive_control(submit) end
   if submit.ep == 0x01 then return receive_endpoint1(submit) end
   printf("received submit on unknown endpoint 0x%.4x\n", submit.ep)
end

local function receive_unlink(unlink)
   printf("received unlink for seqnum %d\n", unlink.victim_seqnum)
   send_unlink_response(unlink, 0) -- status?
end

local function attached() 
   print("starting configuration")
end

cfg.attached = attached
cfg.receive_submit = receive_submit
cfg.receive_unlink = receive_unlink
emulator.start(cfg)

