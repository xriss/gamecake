-- The MIT License (MIT)
--
-- Copyright (c) 2021 Stefano Trettel
--
-- Software repository: MoonUSB, https://github.com/stetre/moonusb
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

--=============================================================================
-- USB DEVICE EMULATOR -- moonusb.emulator.lua
--=============================================================================
-- Emulates a USB device by implementing a USB/IP server exporting a fake device.

local socket = require("socket")
local usb = require("moonusb") -- for utilities only
local has_timers, timers = pcall(require, "moontimers") -- optional

-- Utilities
local fmt = string.format
local function printf(...) io.write(fmt(...)) end
local rep, pack, unpack = string.rep, string.pack, string.unpack
local doubleface = usb.doubleface
local hex, bcd2str, str2bcd = usb.hex, usb.bcd2str, usb.str2bcd
local zeropad, packbytes, unpackbytes = usb.zeropad, usb.packbytes, usb.unpackbytes 

local client -- the currently connected client

-- Configurable parameters
local IP, PORT, USBIP_VER
local ATTACHED, DETACHED, RECEIVE_SUBMIT, RECEIVE_UNLINK -- user callbacks
local VENDOR_ID, PRODUCT_ID, RELEASE_NUMBER
local PATH, SPEED, BUSNUM, DEVNUM, BUSID, DEVID
local DEVICE_CLASS, DEVICE_SUBCLASS, DEVICE_PROTOCOL
local NUM_CONFIGURATIONS, CONFIGURATION_VALUE, INTERFACES

-- usbip opcodes
local OP_REQ_DEVLIST    = 0x8005
local OP_REP_DEVLIST    = 0x0005
local OP_REQ_IMPORT     = 0x8003
local OP_REP_IMPORT     = 0x0003
local USBIP_CMD_SUBMIT  = 0x00000001
local USBIP_CMD_UNLINK  = 0x00000002
local USBIP_RET_SUBMIT  = 0x00000003
local USBIP_RET_UNLINK  = 0x00000004

local USB_DIR = doubleface({
   ['out'] = 0,
   ['in'] = 1,
})

local USB_CLASS = doubleface({
   ['per interface'] = 0x00,
   ['audio'] = 0x01,
   ['cdc'] = 0x02,
   ['hid'] = 0x03,
   ['physical'] = 0x05,
   ['image'] = 0x06,
   ['printer'] = 0x07,
   ['mass storage'] = 0x08,
   ['hub'] = 0x09,
   ['cdc data'] = 0x0a,
   ['smart card'] = 0x0b,
   ['content security'] = 0x0d,
   ['video'] = 0x0e,
   ['personal healthcare'] = 0x0f,
   ['audio video'] = 0x10,
   ['billboard'] = 0x11,
   ['type c bridge'] = 0x12,
   ['diagnostic'] = 0xdc,
   ['wireless'] = 0xe0,
   ['miscellaneous'] = 0xef,
   ['application specific'] = 0xfe,
   ['vendor specific'] = 0xff,
})

local USB_SPEED = doubleface({
   ['unknown'] = 0,
   ['low'] = 1,
   ['full'] = 2,
   ['high'] = 3,
   ['super'] = 4,
   ['super plus'] = 5,
})

-- USBIP protocol --------------------------------------------------------------

local function send_submit_response(submit, status, error_count, data)
-- Send a USBIP_RET_SUBMIT response.
-- submit: the unmodified submit received via the receive_submit() callback,
-- status: 0 for success, non-zero for error @@ codes from <errno.h>?
-- error_count: integer
-- data: binary string containing the URB response, or nil if none
   local t = {}
   -- truncate data if too long to fit (the driver will repeat the
   -- submit request with the appropriate length)
   if data and #data > submit.transfer_buffer_length then
      data = data:sub(1, submit.transfer_buffer_length)
   end
   t[#t+1] = pack(">I4", USBIP_RET_SUBMIT)
   t[#t+1] = pack(">I4", submit.seqnum)
   t[#t+1] = pack(">I4", submit.devid)
   t[#t+1] = pack(">I4", USB_DIR[submit.direction])
   t[#t+1] = pack(">I4", submit.ep)
   t[#t+1] = pack(">i4", status or 0)
   t[#t+1] = pack(">I4", data and #data or 0) -- actual_length
   t[#t+1] = pack(">I4", submit.start_frame)
   t[#t+1] = pack(">I4", submit.number_of_packets)
   t[#t+1] = pack(">I4", error_count or 0)
   t[#t+1] = zeropad(8) -- setup
   t[#t+1] = data
   client:send(table.concat(t))
end

local function send_unlink_response(unlink, status)
-- Send a USBIP_RET_UNLINK response.
-- unlink: the unmodified unlink received via the receive_unlink() callback,
-- status: 0 for success, non-zero for error @@ codes from <errno.h>?
   local t = {}
   t[#t+1] = pack(">I4", USBIP_RET_UNLINK)
   t[#t+1] = pack(">I4", unlink.victim_seqnum)
   t[#t+1] = pack(">I4", unlink.devid)
   t[#t+1] = pack(">I4", USB_DIR[unlink.direction])
   t[#t+1] = pack(">I4", unlink.ep)
   t[#t+1] = pack(">i4", status or 0)
   t[#t+1] = zeropad(24)
   client:send(table.concat(t))
end

local function send_devlist_response()
-- Sends an OP_REP_DEVLIST response to an OP_REQ_DEVLIST.
   local t = {}
   t[#t+1] = pack(">I2I2I4I4", USBIP_VER, OP_REP_DEVLIST, 0, 1) -- 1 device only
   t[#t+1] = PATH
   t[#t+1] = BUSID
   t[#t+1] = pack(">I4", BUSNUM)
   t[#t+1] = pack(">I4", DEVNUM)
   t[#t+1] = pack(">I4", SPEED)
   t[#t+1] = pack(">I2I2", VENDOR_ID, PRODUCT_ID)
   t[#t+1] = pack(">I2", RELEASE_NUMBER)
   t[#t+1] = pack("I1", USB_CLASS[DEVICE_CLASS])
   t[#t+1] = pack("I1", DEVICE_SUBCLASS)
   t[#t+1] = pack("I1", DEVICE_PROTOCOL)
   t[#t+1] = pack("I1", CONFIGURATION_VALUE)
   t[#t+1] = pack("I1", NUM_CONFIGURATIONS)
   t[#t+1] = pack("I1", #INTERFACES)
   for _, itf in ipairs(INTERFACES) do
      t[#t+1] = pack("I1I1I1I1", USB_CLASS[itf.class], itf.subclass or 0, itf.protocol or 0, 0)
   end
   client:send(table.concat(t))
end

local function send_import_response(busid)
-- Sends an OP_REP_IMPORT response to an OP_REQ_IMPORT for busid.
-- Returns true if the request can be accepted, false otherwise.
   local t = {}
   t[#t+1] = pack(">I2I2", USBIP_VER, OP_REP_IMPORT)
   if busid ~= BUSID then
      t[#t+1] = pack(">I4", 1) -- status = not ok
      client:send(table.concat(t))
      return false
   end
   t[#t+1] = pack(">I4", 0) -- status = ok
   t[#t+1] = PATH
   t[#t+1] = BUSID
   t[#t+1] = pack(">I4", BUSNUM)
   t[#t+1] = pack(">I4", DEVNUM)
   t[#t+1] = pack(">I4", SPEED)
   t[#t+1] = pack(">I2I2", VENDOR_ID, PRODUCT_ID)
   t[#t+1] = pack(">I2", RELEASE_NUMBER)
   t[#t+1] = pack("I1", USB_CLASS[DEVICE_CLASS])
   t[#t+1] = pack("I1", DEVICE_SUBCLASS)
   t[#t+1] = pack("I1", DEVICE_PROTOCOL)
   t[#t+1] = pack("I1", CONFIGURATION_VALUE)
   t[#t+1] = pack("I1", NUM_CONFIGURATIONS)
   t[#t+1] = pack("I1", #INTERFACES)
   client:send(table.concat(t))
   return true
end

local function receive_cmd()
-- Receives a command (in attached state), and handles it to the user.
   local hdr = client:receive(48)
   if not hdr then return false end
   local cmd = unpack(">I4", hdr)
   if cmd == USBIP_CMD_SUBMIT then
      local submit = {}
      submit.seqnum = unpack(">I4", hdr, 5)
      submit.devid = unpack(">I4", hdr, 9)
      submit.direction = USB_DIR[unpack(">I4", hdr, 13)]
      submit.ep = unpack(">I4", hdr, 17)
      submit.transfer_flags = unpack(">I4", hdr, 21)
      submit.transfer_buffer_length = unpack(">I4", hdr, 25)
      submit.start_frame = unpack(">I4", hdr, 29)
      submit.number_of_packets = unpack(">I4", hdr, 33)
      submit.interval = unpack(">I4", hdr, 37)
      submit.setup = unpack("c8", hdr, 41)
      local len = submit.transfer_buffer_length
      if submit.direction == 'out' and len > 0 then
         submit.data = client:receive(len)
         if not submit.data then return false end
      end
      RECEIVE_SUBMIT(submit)
      return true
   elseif cmd == USBIP_CMD_UNLINK then
      local unlink = {}
      unlink.seqnum = unpack(">I4", hdr, 5)
      unlink.devid = unpack(">I4", hdr, 9)
      unlink.direction = USB_DIR[unpack(">I4", hdr, 13)]
      unlink.ep = unpack(">I4", hdr, 17)
      unlink.victim_seqnum = unpack(">I4", hdr, 21)
      RECEIVE_UNLINK(unlink)
      return true
   else
      printf("received unknown cmd=0x%.8x\n", cmd)
      return false
   end
end

local function receive_op()
   local hdr = client:receive(8)
   if not hdr then return false end
   local ver, op = unpack(">I2I2", hdr)
   if op == OP_REQ_DEVLIST then
      send_devlist_response()
      return false
   elseif op == OP_REQ_IMPORT then
      local busid = client:receive(32)
      if not busid then return false end
      return send_import_response(busid)
   else
      printf("received unknown op=0x%.4x\n", op)
      return false
   end
end

local function start(cfg)
-- Configure the module and start operations.
   USBIP_VER = str2bcd(cfg.usbip_ver or "01.11")
   IP = cfg.ip or 'localhost'
   PORT = cfg.port or 3240
   BUSNUM = cfg.busnum or 1
   DEVNUM = cfg.devnum or 1
   PATH = pack("c256", cfg.path or "moonusb emulated device")
   VENDOR_ID = cfg.vendor_id or 0x0000
   PRODUCT_ID = cfg.product_id or 0x0000
   RELEASE_NUMBER = str2bcd(cfg.release_number or "00.00")
   SPEED = USB_SPEED[cfg.speed or 'high']
   DEVICE_CLASS = cfg.device_class or 'per interface'
   DEVICE_SUBCLASS = cfg.device_subclass or 0
   DEVICE_PROTOCOL = cfg.device_protocol or 0
   CONFIGURATION_VALUE = cfg.configuration_value or 1
   NUM_CONFIGURATIONS = cfg.num_configurations or 1
   INTERFACES = cfg.interfaces or {}
   ATTACHED = cfg.attached or function() end
   DETACHED = cfg.detached or function() end
   RECEIVE_SUBMIT = cfg.receive_submit
   RECEIVE_UNLINK = cfg.receive_unlink
   DEVID = (BUSNUM << 16 | DEVNUM) -- see usbip_common.h in the Linux kernel
   BUSID = pack("c32", BUSNUM.."-"..DEVNUM)
   -- Create server socket and start listening for client connections
   printf("starting moonusb device emulator on ip=%s:%d\n", IP, PORT)
   local server = assert(socket.bind(IP, PORT))
   assert(server:setoption('reuseaddr', true))
   while true do
      client = assert(server:accept())
      local ip, port = client:getpeername()
      printf("client %s:%d connected\n", ip, port)
      local attached = receive_op()
      if attached then -- keep the connection up for submit and unlink commands
         printf("device attached\n")
         ATTACHED() -- notify the user
         local recvt, r = { client }, nil
         while true do
            if has_timers then timers.trigger() end
            r = socket.select(recvt, nil, 0)
            if r and r[client] then 
               if not receive_cmd() then break end
            end
         end
         printf("device detached\n")
         DETACHED() -- notify the user
      end
      client:close()
      printf("client %s:%d disconnected\n", ip, port)
   end
   server:close()
end

return {
   start = start,
   send_submit_response = send_submit_response,
   send_unlink_response = send_unlink_response,
}

