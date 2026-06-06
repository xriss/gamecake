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
-- 

-- *********************************************************************
-- DO NOT require() THIS MODULE (it is loaded automatically by MoonUSB)
-- *********************************************************************
do
local usb = moonusb -- require("moonusb")

--[[@@ TODO?
usb.decode_bos_device_capability_descriptor = function(dev_cap_type, data)
-- Decodes a BOS device capability descriptor
-- See USB 3.2 Specification, table 9.13:
-- dev_cap_type = bDevCapabilityType
-- data = Capability-Dependent data
	local descr = {}

	return descr
end
--]]

end
