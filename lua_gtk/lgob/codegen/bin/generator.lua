#! /usr/bin/env lua

--[[
	Reads definitions from a Lua script and writes the code to handle it.

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

inGenerator = true

require('lgob.definition')
require('lgob.types')
require('lgob.templates')
require('lgob.utils')

-- Globals: defName, defOverrides, defFunctions, defEnums, defClasses, defTypes, v1, v2
local ti, tc, sf, ex = table.insert, table.concat, string.format, os.execute
local nl = '\n\t'

local input, output, logout

-- Parse options
local options = parseOptions(arg)
if options['-v'] then v1, v2 = parseVersion(options['-v']) end
if options['-i'] then input = options['-i'] else error("You must specify the input file") end  
if options['-o'] then output = io.open(options['-o'], 'w') else output = io.stdout end
if options['-l'] then logout = io.open(options['-l'], 'w') else logout = io.stderr end
if options['-d'] then allow_deprecated = options['-d'] == "allow" end -- global

function print(str) output:write(str,'\n') end
function log(str) logout:write(str,'\n') end

-- Load the file
if not input then error("You must specify an input file.") end
setBasePath(input) 
dofile(input)
log(sf("** Input **: %s (%s) ...", input, defName))

-- Informative
local nFuncTotal, nFuncHandled = 0, 0
local funcByClass = {}
local iface = {
	['functions'] = {}, ['reg'] = {}, ['regadd'] = {}, ['enum'] = {}, 
	['ifacecpy'] = {}, ['regcpy'] = {}
}

-- Visit each class, checking for some special conditions
local classVisited = {}
if defClasses['GtkWindow'] then defClasses['GtkWindow'].noMemHandle = true end

function classRecurse(cname, class)
	if classVisited[cname] then return end
	classVisited[cname] = true
	local ptbl = defClasses[class.parent]
	
	if class.parent and ptbl then
		classRecurse(class.parent, ptbl)
		
		-- GtkWindow and its children can't have the memory automatically handled
		if ptbl.noMemHandle then class.noMemHandle = true end
	end
end

for cname, class in pairs(defClasses) do
	classRecurse(cname, class)
end

-- Generate the code for each function

local sFuncs = {} -- sorted for better debug
for cname, func in pairs(defFunctions) do func.name = cname ti(sFuncs, func) end
table.sort(sFuncs, function(a, b) return a.name < b.name end)

for pos, func in ipairs(sFuncs) do
	if not propertyAvailable(func.accessorSince, v1, v2) then
		local funcBody = ''
		local tables = {
			['input'] = {}, ['args'] = {}, ['poscall'] = {},
			['return'] = {}, ['clean'] = {},
		}
		
		-- luaL_reg row
		local val = sf('{"%s", %s}', skipClass(func.name, func.class, defLib), wrapName(func.name))
		nFuncTotal = nFuncTotal + 1
		
		-- If there's an override, then just use it. Otherwise, generate the function.
		if defOverrides[func.name] then
			funcBody = sf(Templates['functionItem'], wrapName(func.name), defOverrides[func.name])
		else			
			local ccall = ''
			
			-- Return value
			if func.ret then
				local ret = defTypes[func.ret] and 'special' or func.ret
				ret = (func.constructor and (not func.class or not defClasses[func.class].noMemHandle)) and ret .. ' constructor' or ret
				ccall = sf('%s ret = ', Types[ret]['type'])
				Types[ret]['ret'](tables, mtName(func.ret, defLib, defName))
			end
			
			-- Arguments
			if func.args then
				for apos, arg in ipairs(func.args) do
					Types[arg]['arg'](apos, tables)
				end
			end
			
			-- Main call
			ccall = sf('%s%s(%s);', ccall, func.name, tc(tables['args'], ', '))
			
			-- Build the function body
			local body = {}
			if #tables['input'] > 0 then ti(body, tc(tables['input'], nl)) end
			if #ccall > 0 then ti(body, ccall) end
			if #tables['poscall'] > 0 then ti(body, tc(tables['poscall'], nl)) end
			if #tables['return'] > 0 then ti(body, tc(tables['return'], nl)) end
			if #tables['clean'] > 0 then ti(body, tc(tables['clean'], nl)) end
			ti(body, sf('\n\treturn %i;', #tables['return']))
			
			funcBody = sf(Templates['functionItem'], wrapName(func.name), tc(body, nl))
		end
			
		if tables['unhandled'] then
			tables['unhandled'] = false
			log(sf('Function %s not handled.', func.name))
		else
			-- Mark the function as 'readed'
			if not func.class then func.class = '*global*' end
			if not funcByClass[func.class] then funcByClass[func.class] = {} end
			ti(funcByClass[func.class], val)
		
			nFuncHandled = nFuncHandled + 1
			ti(iface['functions'], funcBody) 
		end
	end
end

local genClass = {}

---
-- Recursive function to generate the classes. Follows the inheritance order.
local function generateClass(cname, class)
	if genClass[cname] then return end
	genClass[cname] = true
	if not class then return end
	
	if class.parent then
		generateClass(class.parent, defClasses[class.parent])
	end
	
	local classFuncs = funcByClass[cname] or {}
	class.implements = class.implements or {}
	
	-- Copy the implemented interface functions
	for pos, ifacename in ipairs(class.implements) do
		local parentImplements = class.parent and defClasses[class.parent] and defClasses[class.parent].implements[ifacename]
			
		if defClasses[ifacename] and not parentImplements then
			local cpy = sf(Templates['ifacecpy'], skipLib(ifacename, defLib), skipLib(cname, defLib), regName(ifacename, defLib))
			ti(iface['ifacecpy'], cpy)
		end
		
		-- Implement just one time in the inheritance tree
		class.implements[ifacename] = true
	end
	
	-- Do not generate code for classes that doesn't have parent
	-- and don't have any method
	if #classFuncs == 0 and not class.parent and not class.implements and not class.abstract then return end
	
	ti(classFuncs, [[{NULL, NULL}]])
	local self, parent = skipLib(cname, defLib), class.parent and skipLib(class.parent, defLib)
	
	if self then
		self = sf('"%s"', self)
	else
		error("Self can't be nil!")
	end
	
	if parent then
		parent = sf('"%s"', parent)
	else
		parent = 'NULL'
	end
	
	ti(iface['reg'], sf(Templates['reg'], regName(cname, defLib), tc(classFuncs, ',\n\t')))
	ti(iface['regadd'], sf(Templates['regadd'],  self, parent, regName(cname, defLib)))
end

-- Generate the code for each class (follows the inheritance order)
for cname, class in pairs(defClasses) do
	generateClass(cname, class)
end

-- Generate the code for each enum

local sEnums = {} -- sorted to easier debug
for cname, values in pairs(defEnums) do ti(sEnums, {['name'] = cname, ['values'] = values}) end
table.sort(sEnums, function(a, b) return a.name < b.name end)

for pos, enum in ipairs(sEnums) do
	ti(iface['enum'], '\n' .. sf(Templates['lineComment'], 'enum ' .. enum.name))
	
	for name, value in pairs(enum.values) do
		local crop = skipLibEnum(name, defLib)
		ti(iface['enum'], sf(Templates['enum'], crop, value))
	end
end

-- Generate the code for the global functions
if funcByClass['*global*'] then
	local globalFuncs = funcByClass['*global*']
	ti(globalFuncs, [[{NULL, NULL}]])
	ti(iface['reg'], sf(Templates['reg'], '_global', tc(globalFuncs, ',\n\t')))
end

-- Generate the code for custom types
for cname, ctype in pairs(defTypes) do
	local nFuncs = 3
	local funcs = {}
	
	if ctype.gc and functionDefined(ctype.gc) then
		ti(funcs, sf('lua_pushcfunction(L, %s);', wrapName(ctype.gc)))
		nFuncs = nFuncs + 1 
		
		if ctype.tostring and functionDefined(ctype.tostring) then
			ti(funcs, sf('lua_pushcfunction(L, %s);', wrapName(ctype.tostring)))
			nFuncs = nFuncs + 1
		
			if ctype.eq and functionDefined(ctype.eq) then
				ti(funcs, sf('lua_pushcfunction(L, %s);', wrapName(ctype.eq)))
				nFuncs = nFuncs + 1
			end
		end
	end
	
	ti(iface['regadd'], sf(Templates['specialType'], mtName(cname, defLib, defName), 
		defName, skipLib(cname, defLib), tc(funcs, '\n\t\t'), nFuncs))
end

local up = defName:upper()

-- Generate the output
print(sf(Templates['headerStart'], os.date(), up, up, defName))
print(tc(iface['functions'], '\n'))
print(tc(iface['reg'], '\n'))
print(sf(Templates['mainStart'], defName, defName))
print(tc(iface['regadd'], '\n'))
print(tc(iface['enum'], '\n'))
print('')
print(tc(iface['ifacecpy'], '\n'))
print('')
print(tc(iface['regcpy'], '\n'))
print(sf(Templates['mainEnd'], defName))
print(Templates['headerEnd'])

-- Print some information
log(sf("** Handled %i of %i functions **", nFuncHandled, nFuncTotal))
