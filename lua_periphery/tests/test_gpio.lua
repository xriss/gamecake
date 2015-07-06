--
-- lua-periphery by vsergeev
-- https://github.com/vsergeev/lua-periphery
-- License: MIT
--

require('test')
local periphery = require('periphery')
local GPIO = periphery.GPIO

--------------------------------------------------------------------------------

local pin_input = nil
local pin_output = nil

--------------------------------------------------------------------------------

function test_arguments()
    local gpio = nil

    ptest()

    -- Invalid open types
    passert_periphery_error("invalid open types", function () gpio = GPIO("abc", "blah") end, "GPIO_ERROR_ARG")
    passert_periphery_error("invalid open types", function () gpio = GPIO(1, 1) end, "GPIO_ERROR_ARG")
    -- Invalid direction
    passert_periphery_error("invalid direction", function () gpio = GPIO(1, "blah") end, "GPIO_ERROR_ARG")
end

function test_open_config_close()
    local gpio = nil
    local value = nil

    ptest()

    -- Make sure module version exists
    passert("module version", GPIO.version ~= nil)

    -- Open non-existent GPIO (export should fail with EINVAL)
    passert_periphery_error("non-existent GPIO", function () gpio = GPIO(9999, "in") end, "GPIO_ERROR_EXPORT", 22)

    -- Open legitimate GPIO
    passert_periphery_success("real GPIO", function () gpio = GPIO(pin_output, "in") end)
    passert("property pin", gpio.pin == pin_output)
    passert("fd > 0", gpio.fd > 0)
    io.write(string.format("gpio: %s\n", gpio:__tostring()))

    -- Set direction out, check direction out, check value low
    passert_periphery_success("set direction out", function () gpio.direction = "out" end)
    passert("direction is out", gpio.direction == "out")
    passert("value is low", gpio:read() == false)
    -- Set direction low, check direction out, check value low
    passert_periphery_success("set direction low", function () gpio.direction = "low" end)
    passert("direction is out", gpio.direction == "out")
    passert("Value is low", gpio:read() == false)
    -- Set direction high, check direction out, check value high
    passert_periphery_success("set direction high", function () gpio.direction = "high" end)
    passert("direction is in", gpio.direction == "out")
    passert("value is high", gpio:read() == true)
    -- Set direction in, check direction in
    passert_periphery_success("set direction", function () gpio.direction = "in" end)
    passert("direction is in", gpio.direction == "in")

    -- Set invalid direction
    passert_periphery_error("set invalid direction", function () gpio.direction = "blah" end, "GPIO_ERROR_ARG")
    -- Set invalid edge
    passert_periphery_error("set invalid edge", function () gpio.edge = "blah" end, "GPIO_ERROR_ARG")

    -- Check interrupt edge support
    passert("check interrupt edge support", gpio.supports_interrupts == true)

    -- Set edge none, check edge none
    passert_periphery_success("set edge none", function () gpio.edge = "none" end)
    passert("edge is none", gpio.edge == "none")
    -- Set edge rising, check edge rising
    passert_periphery_success("set edge rising", function () gpio.edge = "rising" end)
    passert("edge is rising", gpio.edge == "rising")
    -- Set edge falling, check edge falling
    passert_periphery_success("set edge falling", function () gpio.edge = "falling" end)
    passert("edge is falling", gpio.edge == "falling")
    -- Set edge both, check edge both
    passert_periphery_success("set edge both", function () gpio.edge = "both" end)
    passert("edge is both", gpio.edge == "both")
    -- Set edge none, check edge none
    passert_periphery_success("set edge none", function () gpio.edge = "none" end)
    passert("edge is none", gpio.edge == "none")

    -- Close gpio
    passert_periphery_success("close gpio", function () gpio:close() end)

    -- Open with table arguments
    passert_periphery_success("real GPIO", function () gpio = GPIO{pin = pin_output, direction="in"} end)
    passert("property pin", gpio.pin == pin_output)
    passert("direction", gpio.direction == "in")
    passert_periphery_success("close gpio", function () gpio:close() end)
end

function test_loopback()
    local gpio_in = nil
    local gpio_out = nil
    local value = nil

    ptest()

    -- Open in and out pins
    passert_periphery_success("open gpio in", function () gpio_in = GPIO(pin_input, "in") end)
    passert_periphery_success("open gpio out", function () gpio_out = GPIO(pin_output, "out") end)

    -- Drive out low, check in low
    print("Drive out low, check in low")
    passert_periphery_success("write gpio out low", function () gpio_out:write(false) end)
    passert("value is false", gpio_in:read() == false)

    -- Drive out high, check in high
    print("Drive out high, check in high")
    passert_periphery_success("write gpio out high", function () gpio_out:write(true) end)
    passert("value is true", gpio_in:read() == true)

    -- Check poll falling 1 -> 0 interrupt
    print("Check poll faliing 1 -> 0 interrupt")
    passert_periphery_success("set gpio in edge falling", function () gpio_in.edge = "falling" end)
    passert_periphery_success("write gpio out low", function () gpio_out:write(false) end)
    passert("gpio in polled 1", gpio_in:poll(1000) == true)
    passert("value is low", gpio_in:read() == false)
    -- Check poll rising 0 -> 1 interrupt
    print("Check poll faliing 0 -> 1 interrupt")
    passert_periphery_success("set gpio in edge rising", function () gpio_in.edge = "rising" end)
    passert_periphery_success("write gpio out high", function () gpio_out:write(true) end)
    passert("gpio in polled 1", gpio_in:poll(1000) == true)
    passert("value is high", gpio_in:read() == true)

    -- Check poll timeout
    passert("poll timed out", gpio_in:poll(1000) == false)

    passert_periphery_success("close gpio in", function () gpio_in:close() end)
    passert_periphery_success("close gpio out", function () gpio_out:close() end)
end

function test_interactive()
    local gpio = nil

    ptest()

    passert_periphery_success("gpio out open", function () gpio = GPIO(pin_output, "out") end)

    print("Starting interactive test. Get out your multimeter, buddy!")
    print("Press enter to continue...")
    io.read()

    -- Drive GPIO out low
    passert_periphery_success("gpio out low", function () gpio:write(false) end)
    print("GPIO out is low? y/n")
    passert("interactive success", io.read() == "y")

    -- Drive GPIO out high
    passert_periphery_success("gpio out high", function () gpio:write(true) end)
    print("GPIO out is high? y/n")
    passert("interactive success", io.read() == "y")

    -- Drive GPIO out low
    passert_periphery_success("gpio out low", function () gpio:write(false) end)
    print("GPIO out is low? y/n")
    passert("interactive success", io.read() == "y")

    passert_periphery_success("close gpio", function () gpio:close() end)
end

if #arg < 2 then
    io.stderr:write(string.format("Usage: lua %s <pin #1> <pin #2>\n\n", arg[0]))
    io.stderr:write("[1/4] Argument test: No requirements.\n")
    io.stderr:write("[2/4] Open/close test: GPIO pin #2 should be real.\n")
    io.stderr:write("[3/4] Loopback test: GPIOs #1 and #2 should be connected with a wire.\n")
    io.stderr:write("[4/4] Interactive test: GPIO #2 should be observed with a multimeter.\n\n")
    io.stderr:write("Hint: for BeagleBone Black, connect a wire between\n")
    io.stderr:write("GPIO 38 (P8.3, gpio1[6]) and GPIO 39 (P8.4, gpio1[7]),\n")
    io.stderr:write("then run this test with:\n")
    io.stderr:write(string.format("    lua %s 38 39\n\n", arg[0]))
    os.exit(1)
end

pin_input = tonumber(arg[1])
pin_output = tonumber(arg[2])

test_arguments()
pokay("Arguments test passed.")
test_open_config_close()
pokay("Open/close test passed.")
test_loopback()
pokay("Loopback test passed.")
test_interactive()
pokay("Interactive test passed.")

pokay("All tests passed!")

