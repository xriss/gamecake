--[[
	Handles definition files.
--]]

require('lgob.utils')

defOverrides = {}	-- Defined 'forced' functions
defFunctions = {}	-- Defined functions (like gtk_widget_show)
defEnums	 = {}	-- Defined enumerations	(like GtkWindowPosition)
defClasses	 = {}	-- Defined classes	(like GtkButton)
defTypes	 = {}	-- Defined special types (like GtkTreePath)
defStructs	 = {}	-- Defines plain C structs (like GdkColor)

function defClass(name, class)
	defClasses[name] = class
end

function defType(name, ctype)
	defTypes[name] = ctype
	defClasses[name] = ctype
end

function defEnum(name, enum)
	defEnums[name] = enum
end

function defFunction(name, func)
	if isAvailable(func.since, v1, v2) then
		if allow_deprecated or ( not isDeprecated(func.deprecated, v1, v2) ) then
			defFunctions[name] = func
		end
	end
end

function defStruct(name, struct)
	defStructs[name] = struct
end

function fixVersion(name, since, deprecated, accessorSince)
	local func = defFunctions[name]
	
	if func then
		if since then func.since = since end
		if deprecated then func.deprecated = deprecated end
		if accessorSince then func.accessorSince = accessorSince end
		
		if not ( isAvailable(func.since, v1, v2) and not isDeprecated(func.deprecated, v1, v2) ) then
			defFunctions[name] = nil
		end
	end
end

function defOverride(name, ovr)
	defOverrides[name] = ovr
end

function undef(tbl)
	for pos, name in ipairs(tbl) do
		defOverrides[name] = nil
		defFunctions[name] = nil
		defEnums[name] = nil
		
		if defClasses[name] then
			-- undef all the class methods
			for fname, func in pairs(defFunctions) do
				if func.class == name then 
					defFunctions[fname] = nil
				end
			end
			
			defClasses[name] = nil
		end
		
		defTypes[name] = nil
	end
end

function undefPattern(pattern)
	for name, obj in pairs(defFunctions) do
		if name:find(pattern) then
			defFunctions[name] = nil
		end
	end
	
	for name, obj in pairs(defClasses) do
		if name:find(pattern) then
			defClasses[name] = nil
		end
	end
	
	for name, obj in pairs(defEnums) do
		if name:find(pattern) then
			defEnums[name] = nil
		end
	end
end
