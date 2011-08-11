#! /usr/bin/env lua

require("lgob.loader")
require("lgob.gtk")

Pattern = {}
local __builder

function get_builder()
	if not __builder then
		__builder = gtk.Builder.new()
		__builder:add_from_file("pattern.ui")
	end
	
	return __builder
end

function Pattern.new()
	local self = {}
	setmetatable(self, {__index = Pattern})
	local builder = get_builder()
	
	self.window = builder:get_object("main_window")
	self.b_test = builder:get_object("b_test")
	self.e_pattern = builder:get_object("e_pattern")
	self.b_input = builder:get_object("v_input"):get("buffer")
	self.b_output = builder:get_object("v_output"):get("buffer")
	
	self.window:connect("delete-event", self.quit, self)
	self.b_test:connect("clicked", self.test_pattern, self)
	
	self.e_pattern:set("text", "(.+) Hello (.+)")
	self.b_input:set("text", "This is: Hello World!")
	
	return self
end

function Pattern:test_pattern()
	local reg       = self.e_pattern:get("text")
	local input     = self.b_input:get("text")
    local ret       = {pcall(input.find, input, reg)}
    
    if not ret[1] then
        self.b_output:set("text", ret[2])
        return
    end
    
    table.remove(ret, 1)
	
	if #reg > 0 and ret[1] and ret[2] then	
		ret[2] = string.format("(%d %d) -> '%s'\n", ret[1], ret[2], input:sub(ret[1], ret[2]))
		self.b_output:set("text", table.concat(ret, "\n", 2))
	else
		self.b_output:set("text", "")
	end
end

function Pattern:quit()
	gtk.main_quit()
	return true
end

function Pattern:run()
	self.window:show_all()
	gtk.main();
end

-- Main

local inst = Pattern.new()
inst:run() 
