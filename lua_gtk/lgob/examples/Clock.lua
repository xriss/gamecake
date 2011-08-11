#! /usr/bin/env lua

-- A Clock Widget

require("lgob.gtk")
require("lgob.gdk")
require("lgob.cairo")

gtk.Clock = {}
setmetatable(gtk.Clock, {__index = gtk.DrawingArea})

---
-- Constructor. The colors must be tables in the form {red, green, blue, alpha},
-- where the values are in the range [0, 1].
--
-- @param bgColor 	Background color
-- @param hColor 	Color of the hours pointer
-- @param mColor 	Color of the minutes pointer
-- @param bgColor 	Color of the seconds pointer
function gtk.Clock.new(bgColor, hColor, mColor, sColor)
	local obj = gtk.DrawingArea.new()
	obj:cast(gtk.Clock)

	-- Set the initial color
	obj:setColor(bgColor, hColor, mColor, sColor)

	glib.timeout_add(glib.PRIORITY_HIGH_IDLE, 1000, gtk.Clock.paint, obj)
	obj:connect("expose-event", gtk.Clock.expose, obj)

	return obj;
end

---
-- Set the clock colors.
--
-- @param bgColor 	Background color
-- @param hColor 	Color of the hours pointer
-- @param mColor 	Color of the minutes pointer
-- @param bgColor 	Color of the seconds pointer
function gtk.Clock:setColor(bgColor, hColor, mColor, sColor)
	local mt = getmetatable(self)
	mt.bgColor = bgColor or {1, 1, 1, 0}
	mt.hColor  = hColor  or {0, 0, 0, 0.8}
	mt.mColor  = mColor  or {0, 0, 0, 0.6}
	mt.sColor  = sColor  or {0, 0, 0, 0.4}
end

---
-- Get the object colors.
--
-- @return bgColor, hColor, mColor, bgColor
function gtk.Clock:getColor()
	local mt = getmetatable(self)
	return mt.bgColor, mt.hColor, mt.mColor, mt.sColor
end

---
-- Expose even callback
function gtk.Clock:expose()
	local cr = gdk.cairo_create(self:get_window())
	local size = (math.min(self:get_size()) / 2)
	local lineWidth = (size / 80)
	local mt = getmetatable(self)

	cr:translate(size, size)
	cr:rectangle(-size, -size, size * 2, size * 2)
	cr:clip()
	size = size * 0.95

	cr:set_line_cap(cairo.LINE_CAP_ROUND)

	-- Clock circle
	cr:set_line_width(lineWidth * 4)
	cr:arc(0, 0, size, 0, 2 * math.pi)
	cr:set_source_rgba(unpack(mt.bgColor))
	cr:fill_preserve()
	cr:set_source_rgb(0, 0, 0)
	cr:stroke()

	-- Ticks
	for i = 0, 11 do
		local pos = -size / 1.15
		cr:set_line_width(lineWidth)

		if i % 3 == 0 then
			pos = -size / 1.3
			cr:set_line_width(lineWidth * 3)
		end

		cr:move_to(0, pos)
		cr:line_to(0, -size)
		cr:stroke()
		cr:rotate(2 * math.pi / 12)
    end

    -- Pointers
    local date = os.date("*t")

    -- Hour pointer
    cr:save()
    local angle = (date.hour + date.min / 60) * ((2 * math.pi) / 12)
    cr:set_source_rgba(unpack(mt.hColor))
    cr:rotate(angle)
    cr:set_line_width(lineWidth * 5)
    cr:move_to(0, 0)
	cr:line_to(0, -size / 2)
	cr:stroke()
	cr:restore()

	-- Minutes pointer
	cr:save()
    local angle = (date.min + date.sec / 60) * ((2 * math.pi) / 60)
    cr:set_source_rgba(unpack(mt.mColor))
    cr:rotate(angle)
    cr:set_line_width(lineWidth * 3.5)
    cr:move_to(0, 0)
	cr:line_to(0, -size / 1.5)
	cr:stroke()
	cr:restore()

	-- Seconds pointer
	cr:save()
    local angle = date.sec * ((2 * math.pi) / 60)
    cr:set_source_rgba(unpack(mt.sColor))
    cr:rotate(angle)
    cr:set_line_width(lineWidth * 2)
    cr:move_to(0, 0)
	cr:line_to(0, -size / 1.2)
	cr:stroke()
	cr:restore()

	cr:destroy()
end

---
-- Timeout callback
function gtk.Clock:paint()
	self:queue_draw()
	return true
end

-- Test program

local window = gtk.Window.new()
local box    = gtk.HBox.new(true, 20)
local clock1 = gtk.Clock.new()
local clock2 = gtk.Clock.new(
	{0.7, 0.8, 0.5, 0.9}, {0.3, 0.6, 0.1, 0.9}, {0.1, 0.3, 0.6, 0.9}, {0.7, 0.7, 0.7, 0.8}
)

window:set("title", "Clock Widget", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)
window:set("width-request", 400, "height-request", 200)

box:add(clock1, clock2)
window:add(box)
window:show_all()
gtk.main()
