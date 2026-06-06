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

-- Assorted utilties for data handling and logs

do
local usb = moonusb -- require("moonusb")
local fmt = string.format
local rep, pack, unpack = string.rep, string.pack, string.unpack
local table_unpack, table_concat = table.unpack, table.concat

usb.hex = function(bytes)
-- Convert a binary string to a readable string of hexadecimal bytes (eg. "00 21 f3 54")
   local t = { unpack(rep("B", #bytes), bytes) }
   t[#t] = nil -- this entry is the index of the next unread byte, we don't want it!
   for i, x in ipairs(t) do t[i] = fmt("%.2x", x) end
   return table_concat(t, " ")
end

usb.bcd2str = function(x) -- e.g. 0x1234 --> "12.34"
   return fmt("%d%d.%d%d", (x>>12)&0x0f, (x>>8)&0x0f, (x>>4)&0xf, x&0xf)
end

usb.str2bcd = function(s) -- e.g. "12.34" --> 0x1234
   local function f(n) return tonumber(s:sub(n, n)) end
   return (f(1)<<12)|(f(2)<<8)|(f(4)<<4)|f(5)
end

local ZERO = pack("I1", 0)
usb.zeropad = function(n)
-- returns a binary string of length n filled with zeros
   return rep(ZERO, n)
end

-- pack/unpack an array of bytes
usb.packbytes = function(t) return pack(rep("I1", #t), table_unpack(t)) end
usb.unpackbytes = function(s)
   local t = {unpack(rep("I1", #s), s)}
   t[#t]=nil -- the last entry is an index in the string, not a byte value
   return t
end

usb.doubleface = function(t)
-- Creates a double-keyed version of the table t,
-- i.e. one where if t[k]=v, then also t[v]=k
   local tt = {}
   for k, v in pairs(t) do
      if tt[v] then
         error(fmt("duplicated value %s %s", tostring(k), tostring(v)))
      end
      tt[k]=v
      tt[v]=k
   end
   return tt
end

end
