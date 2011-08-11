--[[
	This file is part of lgob.

	lgob is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	lgob is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with lgob.  If not, see <http://www.gnu.org/licenses/>.
    
    Copyright (C) 2009 - 2010 Lucas Hermann Negri
--]]

local loadfunc = require('lgob._loader')

if loadfunc then
	local function file_readable(filename)
		local file = io.open(filename)
		if file then file:close() return true end
	end
	
	local function global_loader(modulename)
        if modulename:match('lgob') then
            for path in string.gmatch(package.cpath .. ';', '([^;]*);') do
                local filename = string.gsub(path, '%?', (string.gsub(modulename, '%.', '/')))
                
                if file_readable(filename) then
                    return loadfunc(filename, 'luaopen_' .. modulename:gsub('%.', '_'))
                end
            end
        end
	end

	table.insert(package.loaders, 3, global_loader)
end
