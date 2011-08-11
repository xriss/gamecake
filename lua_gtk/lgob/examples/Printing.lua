#! /usr/bin/env lua

-- Printing example.

require("lgob.gtk")
require("lgob.cairo")
require("lgob.pango")
require("lgob.pangocairo")

-- Create the widgets
local window = gtk.Window.new()
local button = gtk.Button.new_with_mnemonic("Print test")

function beginPrint(ud, context)
	local operation, data = unpack(ud)
	local width, height = context:get_size()

	-- Read the file
	file = io.open(data.fileName)
	data.lineCount = 0
	data.text = {}

	-- Read the file and get the line count
	while true do
		local line = file:read("*line")
		if not line then break end

		data.lineCount = data.lineCount + 1
		data.text[data.lineCount] = line
	end

	-- Calculate the number of pages
	data.fontHeight = data.fontSize / 2
	data.linesPerPage = math.floor(height / data.fontHeight)
	data.pageCount = math.ceil(data.lineCount / data.linesPerPage)
	operation:set("n-pages", data.pageCount)
end

function drawPage(ud, context, page)
	local operation, data = unpack(ud)
	local cr = context:get_cairo_context()
	local width, height = context:get_size()

	local layout = context:create_pango_layout()
	local desc = pango.FontDescription.from_string("mono  " .. data.fontSize)
	layout:set_font_description(desc)

	-- Draw!
	cr:move_to(0, 0)
	cr:set_source_rgb(0, 0, 1)
	local line = ((data.linesPerPage + 1) * page) + 1
	
	for i = 0, data.linesPerPage do
		if line > data.lineCount then break end
		layout:set_text(data.text[line], -1)
		pangocairo.show_layout(cr, layout)
		cr:rel_move_to(0, data.fontHeight)
		line = line + 1
	end
end

function printTest()
	local op = gtk.PrintOperation.new()
	op:set("unit", gtk.UNIT_MM)
	
	-- Set print settings
	local ps = gtk.PrintSettings.new()
	
	-- Page Setup
	local st = gtk.PageSetup.new()
	local st2 = gtk.print_run_page_setup_dialog(window, st, ps)
	
	op:set("default-page-setup", st2, "print-settings", ps)

	-- Some info for the printing
	data = {
		fileName = "Printing.lua",
		fontSize = 10
	}
	
	-- Add example widget
	op:set("custom-tab-label", "My custom tab")
	op:connect("create-custom-widget",
		function()
			local label = gtk.Label.new("Hello world!")
			label:show()
			return label
		end
	)

	op:connect("draw-page", drawPage, {op, data})
	op:connect("begin-print", beginPrint, {op, data})
	op:run(gtk.PRINT_OPERATION_ACTION_PRINT_DIALOG, window)
end

window:add(button)
window:set("title", "Print test", "window-position", gtk.WIN_POS_CENTER)
window:connect("delete-event", gtk.main_quit)
button:connect("clicked", printTest)

window:show_all()
gtk.main()
