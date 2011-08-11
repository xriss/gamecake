#! /usr/bin/env lua

--[[
	Performance tests.
--]]

local n = arg[1] and tonumber(arg[1]) or 99999

-- initialization
local before = os.clock()
local gtk = require("lgob.gtk")
print("lib init: ", os.clock() - before)

-- Function call
before = os.clock()
local f1 = gtk['true']
local f2 = gtk['false']

for i = 1, n do
	f1()
	f2()
end

print("function call: ", os.clock() - before)

-- get / set (direct)
local get = gtk.Object.get
local set = gtk.Object.set

before = os.clock()

local label = gtk.Label.new("Hi! I'm a label")

for i = 1, n do
	local l, a = get(label, "label", "angle")
	set(label, "label", l, "angle", a)
end

print("get / set (direct): ", os.clock() - before)

-- get / set (indirect)
before = os.clock()

local label = gtk.Label.new("Hi! I'm a label")

for i = 1, n do
	local l = label:get_label()
	local a = label:get_angle()
	label:set_label(l)
	label:set_angle(a)
end

print("get / set (indirect): ", os.clock() - before)

-- ListStore
before = os.clock()

local list = gtk.ListStore.new("glong", "gchar", "gchararray", "gint", "gint", "gchar")
local iter = gtk.TreeIter.new()

for i = 1, n do
	list:append(iter)
	local t = {i * 5000, "a", "sdsdsad", 10, 10, "c"}
	list:seto(iter, unpack(t))
end

print("ListStore: ", os.clock() - before)

-- Object creation
before = os.clock()

for i = 1, n do
	gtk.Button.new()
end

print("obj creation: ", os.clock() - before)
print(string.rep('-', 20))
print("Total time: ", os.clock())

print("\nEnd.")

if arg[1] == "mem" then
	-- Pause to manually check the memory use
	io.read("*n")
end
