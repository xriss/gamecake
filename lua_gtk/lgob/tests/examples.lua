#! /usr/bin/env lua

--[[
	Runs all the examples.
	Some of then will not work because they expects the user to provide
	parameters.
--]]

local lfs = require('lfs')

lfs.chdir('../examples')

for file in lfs.dir('.') do
	if file:match('%.lua') then
		os.execute('./' .. file)
	end
end

print('End.')
