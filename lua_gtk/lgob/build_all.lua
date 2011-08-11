#! /usr/bin/env lua

--[[
	Usage: ./build_all.lua absolute_out_path [AMD64]
--]]

local LUA = 'lua'

local modules = {
	'codegen',
	'common',
	'loader',
	'gobject',
	'gdk',
	'gtk',
	'cairo',
	'atk','clutter',
	'cluttergtk',
	'goocanvas',
	'gst',
	'gtkextra',
	'gtkglext',
--  'gtkieembed',
	'gtksourceview',
	'gtkspell',
	'pango',
	'pangocairo',
	'poppler',
	'vte',
	'webkit',
}

local usage = string.format([[Usage: %s absolute_dest]], arg[0])

local out   = assert(arg[1], usage)
local flags = arg[2] or ''
local ex    = function(...) os.execute(string.format(...)) end

for _, mod in ipairs(modules) do
	ex('cd %s; %s ../build.lua %s %s', mod, LUA, out, flags)
end
