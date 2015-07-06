### NAME

GPIO module for Linux userspace sysfs GPIOs.

### SYNOPSIS

``` lua
local periphery = require('periphery')
local GPIO = periphery.GPIO

-- Module Version
GPIO.version                <string>

-- Constructor
gpio = GPIO(pin <number>, direction <string>)
gpio = GPIO{pin=<number>, direction=<string>}

-- Methods
gpio:read() --> <boolean>
gpio:write(value <boolean>)
gpio:poll(timeout_ms <number>) --> <boolean>
gpio:close()

-- Properties
gpio.fd                     immutable <number>
gpio.pin                    immutable <number>
gpio.supports_interrupts    immutable <boolean>
gpio.direction              mutable <string>
gpio.edge                   mutable <string>
```

### CONSTANTS

* GPIO Direction
    * "in": In
    * "out": Out, initialized to low
    * "low": Out, initialized to low
    * "high": Out, initialized to high

* GPIO Edge
    * "none": No interrupt edge
    * "rising": Rising edge (0 -> 1 transition)
    * "falling": Falling edge (1 -> 0 transition)
    * "both": Both edges (X -> !X transition)

### DESCRIPTION

``` lua
Property GPIO.version   immutable <string>
```
Version of GPIO module as a string (e.g. "1.0.0").

--------------------------------------------------------------------------------

``` lua
GPIO(pin <number>, direction <string>) --> <GPIO object>
GPIO{pin=<number>, direction=<string>} --> <GPIO object>
```

Instantiate a GPIO object and open the sysfs GPIO corresponding to the specified pin, with the specified direction. Direction can be "in", "out", "low", or "high" (see [constants](#constants) above).

Example:
``` lua
gpio = GPIO(23, "out")
gpio = GPIO{pin=23, direction="out"}
```

Returns a new GPIO object on success. Raises a [GPIO error](#errors) on failure.

--------------------------------------------------------------------------------

``` lua
gpio:read() --> <boolean>
```
Read the state of the GPIO.

Returns `true` for high state, `false` for low state. Raises a [GPIO error](#errors) on failure.

--------------------------------------------------------------------------------

``` lua
gpio:write(value <boolean>)
```
Set the state of the GPIO to `value`, where `true` is a high state and `false` is a low state.

Raises a [GPIO error](#errors) on failure.

--------------------------------------------------------------------------------

``` lua
gpio:poll(timeout_ms <number>) --> <boolean>
```
Poll a GPIO for the edge event configured with the `.edge` property.

`timeout_ms` can be a positive number for a timeout in milliseconds, 0 for a non-blocking poll, or a negative number for a blocking poll.

Returns `true` if an edge event occurred, `false` on timeout. Raises a [GPIO error](#errors) on failure.

--------------------------------------------------------------------------------

``` lua
gpio:close()
```
Close the sysfs GPIO.

Raises a [GPIO error](#errors) on failure.

--------------------------------------------------------------------------------

``` lua
Property gpio.fd    immutable <number>
```
Get the file descriptor for the underlying sysfs GPIO "value" file of the GPIO object.

Raises a [GPIO error](#errors) on assignment.

--------------------------------------------------------------------------------

``` lua
Property gpio.pin   immutable <number>
```
Get the GPIO object's pin.

Raises a [GPIO error](#errors) on assignment.

--------------------------------------------------------------------------------

``` lua
Property gpio.supports_interrupts   immutable <boolean>
```
Get whether or not this GPIO supports edge interrupts configurable with the `gpio.edge` property.

Raises a [GPIO error](#errors) on assignment.

--------------------------------------------------------------------------------

``` lua
Property gpio.direction     mutable <string>
```
Get or set the GPIO's direction. Can be "in", or "out" (see [constants](#constants) above).

Raises a [GPIO error](#errors) on assignment with invalid direction.

--------------------------------------------------------------------------------

``` lua
Property gpio.edge          mutable <string>
```
Get or set the GPIO's interrupt edge. Can be "none", "rising", "falling", or "both" (see [constants](#constants) above).

Raises a [GPIO error](#errors) on assignment with an invalid edge or if edge interrupts are not supported.

### ERRORS

The periphery GPIO methods and properties may raise a Lua error on failure that can be propagated to the user or caught with Lua's `pcall()`. The error object raised is a table with `code`, `c_errno`, `message` properties, which contain the error code string, underlying C error number, and a descriptive message string of the error, respectively. The error object also provides the necessary metamethod for it to be formatted if it is propagated to the user by the interpreter.

``` lua
--- Example of error propagated to user
> periphery = require('periphery')
> gpio = periphery.GPIO(23, "in")
Exporting GPIO: opening 'export': Permission denied [errno 13]
> 

--- Example of error caught with pcall()
> status, err = pcall(function () gpio = periphery.GPIO(23, "in") end)
> =status
false
> dump(err)
{
  c_errno = 13,
  message = "Exporting GPIO: opening 'export': Permission denied [errno 13]",
  code = "GPIO_ERROR_EXPORT"
}
> 
```

| Error Code                    | Description                   |
|-------------------------------|-------------------------------|
| "GPIO_ERROR_ARG"              | Invalid arguments             |
| "GPIO_ERROR_EXPORT"           | Exporting GPIO                |
| "GPIO_ERROR_OPEN"             | Opening GPIO value            |
| "GPIO_ERROR_IO"               | Reading/writing GPIO value    |
| "GPIO_ERROR_CLOSE"            | Closing GPIO value            |
| "GPIO_ERROR_SET_DIRECTION"    | Setting GPIO direction        |
| "GPIO_ERROR_GET_DIRECTION"    | Getting GPIO direction        |
| "GPIO_ERROR_SET_EDGE"         | Setting GPIO interrupt edge   |
| "GPIO_ERROR_GET_EDGE"         | Getting GPIO interrupt edge   |

### EXAMPLE

``` lua
local GPIO = require('periphery').GPIO

-- Open GPIO 10 with input direction
local gpio_in = GPIO(10, "in")
-- Open GPIO 12 with output direction
local gpio_out = GPIO(12, "out")

local value = gpio_in:read()
gpio_out:write(not value)

print("gpio_in properties")
print(string.format("\tpin: %d", gpio_in.pin))
print(string.format("\tfd: %d", gpio_in.fd))
print(string.format("\tdirection: %s", gpio_in.direction))
print(string.format("\tsupports interrupts: %s", gpio_in.direction))

gpio_in:close()
gpio_out:close()
```

