#!/usr/local/bin/gamecake

local bit=require("bit")

local pack=require("wetgenes.pack")
local wstr=require("wetgenes.string")

for i,v in pairs(wstr) do
	print(i,v)
end

		print(wstr.serialize({"uhm"}))

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

		print(wstr.serialize({"uhm"}))

-- Open the device using the VID, PID,
-- and optionally the Serial number.
local handle = assert(hid.open(0x2833, 0x0001, nil))	-- the oculus rift

		print(wstr.serialize({"uhm"}))

-- Read the Manufacturer String
local wstr = assert(handle:get_manufacturer_string(MAX_STR))
printf("Manufacturer String: %s\n", wstr)

		print(wstr.serialize({"uhm"}))

-- Read the Product String
local wstr = assert(handle:get_product_string(MAX_STR))
printf("Product String: %s\n", wstr)

		print(wstr.serialize({"uhm"}))

--[[
-- Read the Serial Number String
local wstr = assert(handle:get_serial_number_string(MAX_STR))
printf("Serial Number String: %s", wstr)
printf("\n")
]]

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
--assert(handle:set_nonblocking(true))

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


-- asking for a feature report sees to be needed, no idea wjhat it tells me but whatever

assert(handle:set_nonblocking(true))

		print(wstr.serialize({"uhm"}))

assert(handle:send_feature_report(0x2, string.char(0xa0, 0x0a)..string.rep("\0", 14)))
-- Read a Feature Report from the device
local buf = assert(handle:get_feature_report(0x6, 16))
-- Print out the returned buffer.
--[[
printf("Feature Report\n   ");
for i=1,#buf do
	printf("%02x ", string.byte(buf, i))    TrackerMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 62)
            return TrackerMessage_SizeError;

        SampleCount		= buffer[1];
        Timestamp		= DecodeUInt16(buffer + 2);
        LastCommandID	= DecodeUInt16(buffer + 4);
        Temperature		= DecodeSInt16(buffer + 6);
        
        //if (SampleCount > 2)        
        //    OVR_DEBUG_LOG_TEXT(("TackerSensor::Decode SampleCount=%d\n", SampleCount));        

        // Only unpack as many samples as there actually are
        UByte iterationCount = (SampleCount > 2) ? 3 : SampleCount;

        for (UByte i = 0; i < iterationCount; i++)
        {
            UnpackSensor(buffer + 8 + 16 * i,  &Samples[i].AccelX, &Samples[i].AccelY, &Samples[i].AccelZ);
            UnpackSensor(buffer + 16 + 16 * i, &Samples[i].GyroX,  &Samples[i].GyroY,  &Samples[i].GyroZ);
        }

        MagX = DecodeSInt16(buffer + 56);
        MagY = DecodeSInt16(buffer + 58);
        MagZ = DecodeSInt16(buffer + 60);

        return TrackerMessage_Sensors;

end
printf("\n");
]]


-- Read a data
while true do
	local buf = assert(handle:read(62))
--	printf("Data\n   ");

	if #buf>0 then

		for i=1,#buf do
			printf("%02x", string.byte(buf, i))
		end
		if #buf>0 then
			printf("\n");
		end
		
		local head=pack.load(buf,{
			"u8","count",
			"u16","time",
			"u16","last",
			"u16","temp",
		},0)

		local tail=pack.load(buf,{
			"u16","magx",
			"u16","magy",
			"u16","magz",
		},56)
		
		local function debit(base)
--print(base,8,#buf)	
			local b={}
			for i=1,8 do b[i]=pack.read(buf,"u8",base+i-1) end
			local t={}
--[[			
    t[1] = (buffer[0] << 13) | (buffer[1] << 5) | ((buffer[2] & 0xF8) >> 3);
    t[2] = ((buffer[2] & 0x07) << 18) | (buffer[3] << 10) | (buffer[4] << 2) | ((buffer[5] & 0xC0) >> 6);
    t[3] = ((buffer[5] & 0x3F) << 15) | (buffer[6] << 7) | (buffer[7] >> 1);
]]
			return t
		end
		
		local data={}
		for i=1,1 do
			data[ (i-1)*2 + 0]= debit( 8 +     16*(i-1) )
			data[ (i-1)*2 + 1]= debit( 8 + 8 + 16*(i-1) )
		end
		
		
		print( head.count,head.time,head.last,head.temp , tail.magx,tail.magy,tail.magz )
	
	end
	
	break
end

		print(wstr.serialize({"uhm"}))

--[[
        // Only unpack as many samples as there actually are
        UByte iterationCount = (SampleCount > 2) ? 3 : SampleCount;

        for (UByte i = 0; i < iterationCount; i++)
        {
            UnpackSensor(buffer + 8 + 16 * i,  &Samples[i].AccelX, &Samples[i].AccelY, &Samples[i].AccelZ);
            UnpackSensor(buffer + 16 + 16 * i, &Samples[i].GyroX,  &Samples[i].GyroY,  &Samples[i].GyroZ);
        }

        MagX = DecodeSInt16(buffer + 56);
        MagY = DecodeSInt16(buffer + 58);
        MagZ = DecodeSInt16(buffer + 60);
]]


