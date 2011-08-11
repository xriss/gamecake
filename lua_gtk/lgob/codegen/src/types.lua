--[[
	Handles the conversions between C and Lua function calls.
--]]

Types = {}

-- For unhandled types
local unhandled = {
		['arg'] = function(pos, tables) tables['unhandled'] = true end,
		['ret'] = function(tables) tables['unhandled'] = true end,
		['type'] = 'void',
}

setmetatable(Types, {__index = function(tbl, name) return unhandled end})
local ti, tc, sf = table.insert, table.concat, string.format

Types['char* none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tostring(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushstring(L, ret);')
	end,
	['type'] = 'const char*',
}

Types['char* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('char* arg%i;', pos))
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['return'], sf('lua_pushstring(L, arg%i);', pos))
		ti(tables['clean'], sf('_LIB_FREE(arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushstring(L, ret);')
		ti(tables['clean'], '_LIB_FREE(ret);')
	end,
	['type'] = 'char*',
}

Types['unsigned char* none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('(const unsigned char*)lua_tostring(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushstring(L, (const char*)ret);')
	end,
	['type'] = 'const unsigned char*',
}

Types['unsigned char* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('unsigned char* arg%i;', pos))
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['return'], sf('lua_pushstring(L, (char*)arg%i);', pos))
		ti(tables['clean'], sf('_LIB_FREE(arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushstring(L,(char*) ret);')
		ti(tables['clean'], '_LIB_FREE(ret);')
	end,
	['type'] = 'unsigned char*',
}

Types['char none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('const char* arg%i = lua_tostring(L, %i);', pos, pos))
		ti(tables['args'], sf('arg%i ? arg%i[0] : %s', pos, pos, [['\0']]))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushlstring(L, &ret, 1);')
	end,
	['type'] = 'char',
}

Types['int none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'int',
}

Types['int full'] = Types['int none']

Types['boolean none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_toboolean(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushboolean(L, ret);')
	end,
	['type'] = 'int',
}

Types['boolean full'] = Types['boolean none']

Types['unsigned int none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'unsigned int',
}

Types['unsigned int full'] = Types['unsigned int none']

Types['int* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('int arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushinteger(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'int',
}

Types['int* none'] = Types['int* full'];

Types['unsigned int* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('unsigned int arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushinteger(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'unsigned int',
}

Types['unsigned int* none'] = Types['unsigned int* full']

Types['unsigned short none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'unsigned short',
}

Types['long none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'long',
}

Types['int32 none'] = Types['long none']

Types['int32 full'] = Types['long none']

Types['long full'] = Types['long none']

Types['unsigned long none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'unsigned long',
}

Types['unsigned long full'] = Types['unsigned long none']

Types['unsigned long* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('unsigned long arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushinteger(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'unsigned long',
}

Types['unsigned long* none'] = Types['unsigned long* full']

Types['gsize none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tointeger(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushinteger(L, ret);')
	end,
	['type'] = 'gsize',
}

Types['gsize full'] = Types['gsize none']

Types['gsize* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('gsize arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushinteger(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'gsize',
}

Types['gsize* none'] = Types['gsize* full']

-- * FIXME *
Types['unsigned long long none'] = Types['unsigned long none']
Types['unsigned long long full'] = Types['unsigned long none']

Types['ptraslong none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_touserdata(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushlightuserdata(L, ret);')
	end,
	['type'] = 'void*',
}

Types['ptraslong full'] = Types['ptraslong none']

Types['float none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tonumber(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushnumber(L, ret);')
	end,
	['type'] = 'float',
}

Types['double none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_tonumber(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushnumber(L, ret);')
	end,
	['type'] = 'double',
}

Types['float* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('float arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushnumber(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'float',
}

Types['double* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('double arg%i;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['return'], sf('lua_pushnumber(L, arg%i);', pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'double',
}

Types['GObject* none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['input'], sf([[Object* oarg%i = lua_touserdata(L, %i);
	void* arg%i = oarg%i ? oarg%i->pointer : NULL; 
]], pos, pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'object_new(L, ret, FALSE);')
	end,
	['type'] = 'void*',
}

Types['GObject* full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['input'], sf([[Object* oarg%i = lua_touserdata(L, %i);
	void* arg%i = oarg%i ? oarg%i->pointer : NULL; 
]], pos, pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'object_new(L, ret, FALSE);')
	end,
	['type'] = 'void*',
}

Types['GObject* full constructor'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['input'], sf([[Object* oarg%i = lua_touserdata(L, %i);
	void* arg%i = oarg%i ? oarg%i->pointer : NULL; 
]], pos, pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'object_new(L, ret, TRUE);')
	end,
	['type'] = 'void*',
}

Types['GObject* none constructor'] = Types['GObject* full constructor']

Types['GError** full'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['input'], sf('GError* arg%i = NULL;', pos))
		ti(tables['args'], sf('&arg%i', pos))
		ti(tables['poscall'], sf([[
		
	if(arg%i)
	{
		lua_pushboolean(L, FALSE);
		lua_pushinteger(L, arg%i->code);
		lua_pushstring(L, arg%i->message);
		g_error_free(arg%i);
		return 3;
	}
]], pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'GError*',
}

Types['struct* none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['input'], sf([[Object* oarg%i = lua_touserdata(L, %i);
	void* arg%i = oarg%i ? oarg%i->pointer : NULL; 
]], pos, pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables)
		tables['unhandled'] = true
	end,
	['type'] = 'void*',
}

Types['boolean none'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('lua_toboolean(L, %i)', pos))
	end,
	['ret'] =
	function(tables)
		ti(tables['return'], 'lua_pushboolean(L, ret);')
	end,
	['type'] = 'int',
}

Types['special*'] = {
	['arg'] =
	function(pos, tables)
		ti(tables['args'], sf('arg%i', pos))
		ti(tables['input'], sf([[Object* oarg%i = lua_touserdata(L, %i);
	void* arg%i = oarg%i ? oarg%i->pointer : NULL; 
]], pos, pos, pos, pos, pos))
	end,
	['ret'] =
	function(tables, mt)
		ti(tables['return'], sf('special_type_new(L, "%s", ret);', mt))
	end,
	['type'] = 'void*',
}

-- If called as the 'main', just list the types handled
if not inGenerator then
	local names = {}
	for name, j in pairs(Types) do ti(names, name) end
	table.sort(names)
	
	print('Types handled:')
	print(tc(names, ', '))
end
