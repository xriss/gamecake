local hid = require 'hid'

local MAX_STR = 255

-- Enumerate and print the HID devices on the system

local function printf(format, ...)
	io.write(string.format(format, ...))
end
	
local devs = hid.enumerate(0x0, 0x0)
local cur_dev = devs
while cur_dev do
	printf("Device Found\n  type: %04x %04x\n  path: %s\n  serial_number: %s",
		cur_dev.vendor_id, cur_dev.product_id, cur_dev.path, cur_dev.serial_number)
	printf("\n")
	printf("  Manufacturer: %s\n", cur_dev.manufacturer_string)
	printf("  Product:      %s\n", cur_dev.product_string)
	printf("\n")
	cur_dev = cur_dev.next
end
devs = nil

-- Open the device using the VID, PID,
-- and optionally the Serial number.
local handle = assert(hid.open(0xd00b, 0x0002, nil))

-- Read the Manufacturer String
local wstr = assert(handle:get_manufacturer_string(MAX_STR))
printf("Manufacturer String: %s\n", wstr)

-- Read the Product String
local wstr = assert(handle:get_product_string(MAX_STR))
printf("Product String: %s\n", wstr)

-- Read the Serial Number String
local wstr = assert(handle:get_serial_number_string(MAX_STR))
printf("Serial Number String: %s", wstr)
printf("\n")

--[[
-- Read a data
while true do
	local buf = assert(handle:read(16))
--	printf("Data\n   ");
	for i=1,#buf do
		printf("%02x ", string.byte(buf, i))
	end
	printf("\n");
end
--]]

-- Send a Feature Report to the device
--assert(handle:send_feature_report(0x2, string.char(0xa0, 0x0a)..string.rep("\0", 14)))

--[[
-- Read a Feature Report from the device
local buf = assert(handle:get_feature_report(0x6, 16))

-- Print out the returned buffer.
printf("Feature Report\n   ");
for i=1,#buf do
	printf("%02x ", string.byte(buf, i))
end
printf("\n");
--]]

-- Set the hid_read() function to be non-blocking.
assert(handle:set_nonblocking(true))

--[=[
-- Send an Output report to toggle the LED (cmd 0x80)
buf[0] = 1; -- First byte is report number
buf[1] = 0x80;
res = hid_write(handle, buf, 65);

-- Send an Output report to request the state (cmd 0x81)
buf[1] = 0x81;
hid_write(handle, buf, 65);

-- Read requested state
res = hid_read(handle, buf, 65);
if (res < 0) then
	printf("Unable to read()\n");
end

-- Print out the returned buffer.
for (i = 0; i < res; i++)
	printf("buf[%d]: %d\n", i, buf[i]);
end

--]=]

