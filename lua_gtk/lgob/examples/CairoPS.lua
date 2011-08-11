#! /usr/bin/env lua

require("lgob.cairo")

local width, height = 300, 300
local surface = cairo.PsSurface.create("output.ps", width, height)
local cr = cairo.Context.create(surface)

cr:set_source_rgb(0, 0, 0)
cr:set_line_width(10)
cr:rectangle(20, 20, width - 40, height - 40)
cr:stroke()
cr:rectangle(20, 20, width - 40, height - 40)
cr:set_source_rgb(0, 0, 1)
cr:fill()

cr:show_page()

-- Free them
cr:destroy()
surface:destroy()
