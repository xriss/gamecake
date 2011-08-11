#! /usr/bin/env lua

--[[
	.gir to lgob .def translator.
		
	@author Lucas Hermann Negri
--]]
SEP = package.config:sub(1,1)

local exe_path
if arg[0]:sub(1,1) == '.' then
   exe_path = os.getenv'PWD'
   exe_path = exe_path .. SEP .. arg[0]
else
   exe_path = arg[0]
end
_,_, exe_path = exe_path:find('^(.+)' .. SEP .. '.+$')

package.path = string.format('%s;%s/../share/lua/5.1/?.lua', package.path, exe_path)
if SEP == '/' then
   package.cpath = string.format('%s;%s/../lib/lua/5.1/?.so', package.cpath, exe_path)
else
   package.cpath = string.format('%s;%s/../lib/lua/5.1/?.dll', package.cpath, exe_path)
end

require('lxp')
require('lgob.utils')
require('lgob.parser')

local ti, tc, sf = table.insert, table.concat, string.format
local input, output, logout

-- * Parse options ** --
local options = parseOptions(arg)
if options['-i'] then input = options['-i']                else error("You must specify the input file") end  
if options['-o'] then output = io.open(options['-o'], 'w') else output = io.stdout end
if options['-n'] then defName = options['-n']              end
if options['-l'] then logout = io.open(options['-l'], 'w') else logout = io.stderr end

function print(str) output:write(str,'\n') end
function log(str) logout:write(str,'\n') end

-- * Parse the XML **
local defLib, classes, structs, enums, functions, symbols, scSymbols = parse(input, v1, v2)
defName = defName or defLib

-- ** Symbol resolution ** --

-- Solve multiple levels of typedefs (gboolean -> gint -> int)
local sChanged = true

while sChanged do
	sChanged = false

	for src, target in pairs(symbols) do
		if symbols[target] then
			symbols[src] = symbols[target]
			sChanged = true
		end
	end
end

for pos, class in ipairs(classes) do
	class.parent = symbols[class.parent] or class.parent
end

for pos, func in ipairs(functions) do
	local ret, args = func.ret, func.args
	ret.type        = symbols[ret.type] or ret.type
	ret.type        = scSymbols[ret.type] or ret.type
	
	for pos, arg in ipairs(func.args or {}) do
		args[pos].type = symbols[arg.type] or args[pos].type
		args[pos].type = scSymbols[arg.type] or args[pos].type
	end
end

-- ** Handle exception throwing **

for pos, func in ipairs(functions) do
	if func.throws then
		ti(func.args, {
			['ownership'] = 'full',
			['type'] = 'GError**',
			['direction'] = 'out'
		})
	end
end

-- ** Associate accessor methods with properties **

for pos, class in ipairs(classes) do
	for pos2, method in ipairs(class.methods) do
		local ptype, pname = getPropertyName(method.name, class.name, defLib)
		
		if pname then
			local property = class.properties[pname]
			
			if property then
				local isAccessor = false
				
				if ptype == 'get' then
					isAccessor = #method.args == 1 and method.ret.type ~= 'void'
				elseif ptype == 'set' then
					isAccessor = #method.args == 2 and method.ret.type == 'void'
				end
				
				if isAccessor then
					method.accessorSince = property.since
				end
			end
		end
		
	end
end

-- ** Generate the .def **

print(sf('defLib = \'%s\'', defLib))
print(sf('defName = \'%s\'\n', defName))

for pos, class in ipairs(classes) do
	local body = {}
	if class.parent          then ti(body, sf([[parent = '%s']], class.parent)) end
	if class.abstract        then ti(body, [[abstract = true]]) end
	if #class.implements > 0 then ti(body, sf([[implements = {%s}]], tc(class.implements, ', '))) end
	
	print(sf(
[[
defClass('%s', 
	{
		%s
	}
)
]], class.name, #body > 0 and tc(body, ',\n\t\t') or ''))
end

local privates = {}

for pos, struct in ipairs(structs) do
	-- Ignore class and private structures
	if not isPrivateStruct(struct.name) then
		local body = {}
		
		print(sf(
[[
defClass('%s', 
	{
	}
)
]], struct.name))
	else
		privates[struct.name] = true
	end
end

for pos, func in ipairs(functions) do
	if not (func.class and privates[func.class]) then
		local body = {}
		if func.class         then ti(body, sf([[class = '%s']], func.class)) end
		if func.constructor   then ti(body, [[constructor = true]]) end
		if func.since         then ti(body, sf([[since = '%s']], func.since)) end
		if func.deprecated    then ti(body, sf([[deprecated = '%s']], func.deprecated)) end
		if func.accessorSince then ti(body, sf([[accessorSince = '%s']], func.accessorSince)) end
		
		local arglist = {}
		for pos, arg in ipairs(func.args) do
            ti(arglist, sf([['%s %s']], arg.type, arg.ownership))
		end
		if #arglist > 0 then ti(body, sf([[args = {%s}]], tc(arglist, ', '))) end

		local ret = func.ret
        if ret.type and ret.type ~= 'void' then  ti(body, sf([[ret = '%s %s']], ret.type, ret.ownership)) end
		
		print(sf(
[[
defFunction('%s',
	{
		%s
	}
)
]], func.name, #body > 0 and tc(body, ',\n\t\t') or ''))
	end
end

for pos, enum in ipairs(enums) do
	local body = {}
	
	for pos, member in ipairs(enum.members) do
		ti(body, sf('[\'%s\'] = %i', member.name, member.value))
	end
	
	print(sf(
[[
defEnum('%s',
	{
		%s
	}
)
]], enum.name, #body > 0 and tc(body, ',\n\t\t') ))
end

output:close()
