#!/usr/bin/env lua

--[[
	Reads function versions (since and deprecated) from a ".devhelp2" XML, and
	updates .def files.
	
	Usage: ./update lib.devhelp2 < lib.def > libfunctions.def

	@author Hans Elbers
]]

require("lxp")
require("lgob.utils")

local debugi = true
local baseVersion = '0.1'

local function pr(fmt, ...)
	if debugi then io.stderr:write(string.format('-- ' .. fmt .. '\n', ...)) end
end

-- read 'def' file
local defFunctions = {}
local defClasses = {}
local defEnums = {}

function defClass(name, param)
	defClasses[name] = param
	param.since = "9999.9999"
end

function defType(name, param)		defTypes[name] 		= param	end
function defEnum(name, param)		end
function defFunction(name, param) 	defFunctions[name] 	= param	end

local input = io.read('*all')
local readDef = loadstring(input)
readDef();

-- read devhelp2 xml file and adjust defFunctions
function doFunc(parser, name, attr)
	if not (name == 'keyword' and attr.type == 'function') then return end

	local func, since, deprecated = attr.name, attr.since, attr.deprecated
	funcn = string.gsub(func, '%s.*', '')	-- 'foo ()' -> 'foo'
	
	if funcn then func = defFunctions[funcn] end

	if func then
		-- Now we're interested (it's about a function we know)
		if since then 
			local _, _, v = since:find("([0-9.]+)")
			since = v
		end
		
		since = since or baseVersion
		
		-- the *_get_type version info is missing, so we get the oldest
		-- function of the class and use as the *_get_type 'since' tag
		if func.class and not funcn:match("_get_type") then
			local class = defClasses[func.class]
			
			local v1, v2 = parseVersion(since)
			local v3, v4 = parseVersion(class.since)
			
			if cmpVersion(v1, v2, v3, v4) < 0 then
				class.since = since
			end
		end
		
		if not (since == func.since) then
  			pr('I: %s: since %s -> %s', funcn, func.since or 'nil', since)
			func.since = since
		end

		if deprecated then
			local _, _, v = deprecated:find("(%d+.%d+)")
			if not v then v = baseVersion end
			
			if not (v == func.deprecated) then
				pr('I: %s: deprecated %s -> %s', func.name or funcn, func.deprecated or 'nil', v or 'nil')
				func.deprecated = v
			end
		end
	else
		pr('W: %s is not defined in input', funcn)
	end
end

local devhelp2 = assert(io.open(arg[1]), 'r')
local xml = lxp.new{StartElement = doFunc}
xml:parse(devhelp2:read('*all')) 
xml:close()
devhelp2:close()

-- use the collected data to give a version to *_get_version() functions

for name, func in pairs(defFunctions) do
	if func.class and name:match("_get_type") then
		local since = defClasses[func.class].since
		pr('I: %s: since %s -> %s', name, func.since or 'nil', since)
		func.since = since
	end
end

-- re-write the defLib and defName

io.write("defLib = '", defLib,"'\n")
io.write("defName = '", defName,"'\n\n")

-- re-write the classes

function defClass(name, param)
	local def = "defClass('" .. name .. "',\n\t{\n\t\t"
	local tbl = {}
	if param.parent then tbl[#tbl+1] = "parent = '" .. param.parent .. "'" end
	if param.abstract then tbl[#tbl+1] = "abstract = true" end
	if param.implements then tbl[#tbl+1] = "implements = {'" .. table.concat(param.implements, "', '") .. "'}" end
	io.write(def, table.concat(tbl, ",\n\t\t"), "\n\t}\n)\n\n")
end 	

-- write the updated function definitions

local funcAttr = {
   'class', 'constructor', 'since', 'accessorSince', 'args', 'ret', 'deprecated'
} -- list to order atributes in output

function defFunction(name, class) 
	local func = defFunctions[name]
	local ats = {}
	
	for _, attr in ipairs(funcAttr) do
		if func[attr] then
  			local at
  			if attr == 'args' then
				at = "{'" .. table.concat(func[attr], "', '") .. "'}"
  			elseif attr == 'constructor' then
				at = tostring(func[attr])
  			else
				at = "'" .. func[attr] ..  "'"
  			end
  			
			ats[#ats+1] =  '\t\t' .. attr .. " = " .. at
		end
	end

	io.write("defFunction('" ,name , "',\n\t{\n", table.concat(ats, ',\n'), '\n\t}\n)\n\n')
end

-- re-write the enums

function defEnum(name, param)
	local def = "defEnum('" .. name .. "',\n\t{\n\t\t"
	local tbl = {}
	for name, value in pairs(param) do tbl[#tbl+1] = "['" .. name .. "'] = " .. value end
	io.write(def, table.concat(tbl, ",\n\t\t"), "\n\t}\n)\n\n")
end 

readDef()	-- reprocess input with these new functions...

