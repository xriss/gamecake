


local usb = require("moonusb")


--usb.trace_objects(true) -- trace creation/deletion of objects


local ctx = usb.init()
local vendor_id, product_id = 0x0483,  0x2023
local device, devhandle = ctx:open_device(vendor_id, product_id)


local buffer = "SELFTEST\n"
local mem1 = usb.malloc(nil, #buffer)
mem1:write(0,nil,buffer)
local bytes_sent = devhandle:bulk_transfer(0x01, mem1:ptr(), #buffer, 0)
mem1:free()


devhandle:close()




