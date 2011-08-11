--[[
	The mighty utilities.
--]]

local ti, sf = table.insert, string.format
local sep = '/'

---
-- Checks if a file exists.
function checkFile(name)
	local f = io.open(name)
	if f then f:close() return true end
end

---
-- Gets the basename of a file (oi/myfile.ext -> oi/myfile).
function basename(name)
	local p1 = name:find('%.')
	if p1 then return name:sub(1, p1 -1) else return name end
end

---
-- Skips the library name (GtkBin, Gtk -> Bin)
function skipLib(name, defLib)
	local _, _, newn = name:find(defLib .. '(.+)')
	return newn or skipFirst(name)
end

---
-- GTK_OI_AMIGO -> OI_AMIGO, GtkOiAmigo -> OiAmigo
function skipFirst(name)
	local _, _, crop = name:find('_(.+)')
   	if not crop then _, _, crop = name:find('(%u.+)', 2) end
   	return crop or name
end

---
-- Modules to override this
function skipLibEnum(name, defLib)
	return skipFirst(name)
end

---
-- Gets a simplified name for a reg typedef (GtkCoolWidget -> coolwidget)
function regName(name, defLib)
	return skipLib(name, defLib):lower()
end

local function camelHelper(str)
	local a, b = str:sub(1, 1), str:sub(2, 2)
	
	if b == b:upper() then
		return '_' .. str:lower()
	else
		return '_' .. a:lower() .. b
	end
end

---
-- Converts GtkWidget to gtk_widget.
function camelToC(name)
	if #name > 1 then
		local cname = name:gsub('%u.', camelHelper)
		return cname:sub(2)
	else
		return name:lower()
	end
end

---
-- Converts gtk_widget_show_all to show_all
function skipClass(name, class, lib)
	if class then
		local class = skipLib(class, lib)
		name = skipLib(name, camelToC(lib)):sub(2)
		local name, skip = name:gsub(camelToC(class), "", 1)
		return name:sub(skip + 1)
	else
		return skipLib(name, camelToC(lib)):sub(2)
	end
end

---
-- Gets the path of a file (../kknd/file -> ../kknd/)
function getPath(name)
	local p1 = name:reverse():find(sep)
	
	if p1 then
		return name:sub(1, #name - p1 + 1)
	else
		return name
	end
end

local baseFolder = ''

---
-- Sets the base folder using a file path.
function setBasePath(name)
	baseFolder = getPath(name)
end

---
-- Includes a file. Uses the base path of the main definition file as base.
function include(name)
	dofile(baseFolder .. name)
end

---
-- Gets the MT name of a type (GtkObject -> lgobObjectMT) 
function mtName(name, defLib, lib)
	local skip = skipLib(name, defLib)
	return lib .. (skip or name) .. 'MT'
end

---
-- Converts a function name to a internal representation
function wrapName(name)
	return '_wrap_' .. name
end

---
-- Checks if a function is defined. If not, log it.
function functionDefined(name)
	if defFunctions[name] then
		return true
	else
		log(sf('The function %s is used, but not defined.', name))
		return false
	end
end

---
-- Parses a version string into major and minor (numbers).
function parseVersion(v)
	if type(v) == 'string' then
		local _, _, v1, v2 = v:find('(%d+).(%d+)')
		return tonumber(v1), tonumber(v2)
	end
end

---
-- Compares two versions. Returns > 0 if A1.A2 > B1.B2 and so on.
function cmpVersion(A1, A2, B1, B2)
	local retA = A1 - B1
	if retA ~= 0 then return retA end
	return A2 - B2
end

---
-- Checks if an attr is deprecated.
function isDeprecated(deprecated, v1, v2)
	if deprecated then
		local v3, v4 = parseVersion(deprecated)
	
		if not v1 or not v2 or not v3 or not v4 or cmpVersion(v1, v2, v3, v4) >= 0 then
			return true
		end
	end
end

---
-- Checks if an attr is available in the current version.
function isAvailable(since, v1, v2)
	local v3, v4 = parseVersion(since)
	
	if v1 and v2 and v3 and v4 then
		return cmpVersion(v1, v2, v3, v4) >= 0
	else
		return true
	end
end

---
-- Parse options.
function parseOptions(args)
	local options = {}
	
	for i = 1, #args, 2 do
		options[args[i]] = args[i + 1]
	end
	
	return options
end

---
-- Heuristic to check if a name is private
function isPrivateStruct(name)
	local p = name:find('private')
	if p then return true end
	
	local p = name:find('Private')
	if p then return true end
	
	local p = name:find('Class')
	if p then return true end
	
	local p = name:find('class')
	if p then return true end
end

---
-- Get the property of an accessor (gtk_label_get_use_underline -> use-underline)
function getPropertyName(func, class, lib)
	local _, _, prefix, name = skipClass(func, class, lib):find("(.-)_(.+)")
	
	if (prefix == 'set' or prefix == 'get') and name then
		local name = name:gsub("_", "-")
		return prefix, name
	end
end

---
-- Checks if the property of an accessor is available for direct access.
function propertyAvailable(version, v1, v2)
	if not version or not v1 or not v2 then
		return false
	else
		local v3, v4 = parseVersion(version)
		return cmpVersion(v1, v2, v3, v4) >= 0
	end
end

---
-- Atk.Implementor, Gtk -> AtkImplementor
-- Buildable, Gtk -> GtkBuildable
function fullName(name, defName)
	local _, _, p, s = name:find('(.-)%.(.+)')
	local ret
	
	if p and s then
		ret = p .. s
	else
		ret = defName .. name
	end
	
	return "'" .. ret .. "'"
end

---
-- Do a shallow copy of a table.
function shallowCopy(tbl)
	local newtbl = {}
	for i, j in pairs(tbl) do newtbl[i] = j end
	return newtbl
end
