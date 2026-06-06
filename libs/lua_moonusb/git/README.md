## MoonUSB: Lua bindings for libusb

MoonUSB is a Lua binding library for [libusb](https://libusb.info/),
allowing applications to access and use USB devices.

MoonUSB also provides a submodule for emulating USB devices via [USB/IP](http://usbip.sourceforge.net/).

It runs on GNU/Linux <!-- and on Windows (MSYS2/MinGW) --> and requires 
[Lua](http://www.lua.org/) (>=5.3) and [libusb](https://github.com/libusb/libusb/releases) (>= 1.0.24).

_Author:_ _[Stefano Trettel](https://www.linkedin.com/in/stetre)_

[![Lua logo](./doc/powered-by-lua.gif)](http://www.lua.org/)

#### License

MIT/X11 license (same as Lua). See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://stetre.github.io/moonusb/doc/index.html).

#### Getting and installing

Setup the build environment as described [here](https://github.com/stetre/moonlibs), then:

```sh
$ git clone https://github.com/stetre/moonusb/
$ cd moonusb
moonusb$ make
moonusb$ sudo make install
```

#### Example

The example below lists the devices currently attached to the system.

Other examples can be found in the **examples/** directory.

```lua
-- MoonUSB example: hello.lua
local usb = require("moonusb")

local ctx = usb.init()

local devices = ctx:get_device_list()

for i, dev in ipairs(devices) do
   local descr = dev:get_device_descriptor()
   local devhandle = dev:open()
   print(string.format("USB %s - bus:%d port:%d %.4x:%.4x %s %s (%s)",
      descr.usb_version, dev:get_bus_number(), dev:get_port_number(),
      descr.vendor_id, descr.product_id,
      devhandle:get_string_descriptor(descr.manufacturer_index) or "???",
      devhandle:get_string_descriptor(descr.product_index) or "???",
      descr.class))
   devhandle:close()
   dev:free()
end

```

The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ lua hello.lua
```

#### See also

* [MoonLibs - Graphics and Audio Lua Libraries](https://github.com/stetre/moonlibs).
