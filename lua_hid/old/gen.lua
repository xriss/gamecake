local structs = {}

------------------------------------------------------------------------------

table.insert(structs, {
	name = 'device_info',
	cname = 'struct hid_device_info',
	fields = {
		{name='path', ctype='char*', luatype='string'},
		{name='vendor_id', ctype='unsigned short', luatype='number'},
		{name='product_id', ctype='unsigned short ', luatype='number'},
		{name='serial_number', ctype='wchar_t*', luatype='wstring'},
		{name='release_number', ctype='unsigned short ', luatype='number'},
		{name='manufacturer_string', ctype='wchar_t*', luatype='wstring'},
		{name='product_string', ctype='wchar_t*', luatype='wstring'},
		{name='usage_page', ctype='unsigned short ', luatype='number'},
		{name='usage', ctype='unsigned short ', luatype='number'},
		{name='interface_number', ctype='int', luatype='number'},
		{name='next', luatype='userdata'},
	},
})

table.insert(structs, {
	name = 'device',
	cname = 'hid_device',
	gc = true,
})

------------------------------------------------------------------------------

local structs_h = assert(io.open('structs.h', 'wb'))
local structs_c = assert(io.open('structs.c', 'wb'))

structs_h:write([[
/* this is a generated file, see gen.lua */
#include <lua.h>
#include <hidapi.h>

]])

structs_c:write([[
/* this is a generated file, see gen.lua */
#include "structs.h"

#include <lauxlib.h>
#include "compat.h"

static int luahid_generic_index(lua_State* L)
{
	lua_getmetatable(L, 1);
	/* get a getter and call it */
	lua_getfield(L, -1, "getters");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (!lua_isnil(L, -1))
	{
		lua_pushvalue(L, 1);
		lua_call(L, 1, 1);
		return 1;
	}
	lua_pop(L, 2);
	/* get a method */
	lua_getfield(L, -1, "methods");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	return 1;
}

static int luahid_generic_tostring(lua_State* L)
{
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "typename");
	lua_pushfstring(L, "%s: %p", lua_tostring(L, -1), lua_touserdata(L, 1));
	return 1;
}

]])

for _,struct in ipairs(structs) do
	local name = struct.name
	local cname = struct.cname
	structs_h:write([[
]]..cname..[[* luahid_to_]]..name..[[(lua_State* L, int index);
]])
	structs_c:write([[
]]..cname..[[* luahid_to_]]..name..[[(lua_State* L, int index)
{
	]]..cname..[[* udata;
	udata = *(]]..cname..[[**)lua_touserdata(L, index);
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

]])
	
	structs_h:write([[
]]..cname..[[* luahid_check_]]..name..[[(lua_State* L, int index);
]])
	structs_c:write([[
]]..cname..[[* luahid_check_]]..name..[[(lua_State* L, int index)
{
	]]..cname..[[* udata;
	udata = *(]]..cname..[[**)luaL_checkudata(L, index, "]]..cname..[[");
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

]])
	
	structs_h:write([[
void luahid_push_]]..name..[[(lua_State* L, const ]]..cname..[[* value, int owner);
]])
	structs_c:write([[
void luahid_push_]]..name..[[(lua_State* L, const ]]..cname..[[* value, int owner)
{
	const ]]..cname..[[** udata;
	if (owner < 0) owner = lua_gettop(L) + 1 + owner;
	udata = (const ]]..cname..[[**)lua_newuserdata(L, sizeof(]]..cname..[[*));
	*udata = value;
	luaL_getmetatable(L, "]]..cname..[[");
	lua_setmetatable(L, -2);
	if (owner != 0)
	{
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, owner);
		lua_rawseti(L, -2, 1);
		setuservalue(L, -2);
	}
}

]])

	-- getters
	if struct.fields then
		for _,field in ipairs(struct.fields) do
			if field.luatype=='number' then
				structs_c:write([[
static int luahid_get_]]..name..[[_]]..field.name..[[(lua_State* L)
{
	]]..cname..[[* udata;
	udata = luahid_to_]]..name..[[(L, 1);
	lua_pushnumber(L, udata->]]..field.name..[[);
	return 1;
}

]])
			elseif field.luatype=='string' then
				structs_c:write([[
static int luahid_get_]]..name..[[_]]..field.name..[[(lua_State* L)
{
	]]..cname..[[* udata;
	udata = luahid_to_]]..name..[[(L, 1);
	if (udata->]]..field.name..[[)
	{
]])
				if type(field.size)=='string' then
					structs_c:write([[
		lua_pushlstring(L, (const char*)udata->]]..field.name..[[, udata->]]..field.size..[[);
]])
				elseif type(field.size)=='number' then
					structs_c:write([[
		lua_pushlstring(L, (const char*)udata->]]..field.name..[[, ]]..field.size..[[);
]])
				elseif type(field.size)=='nil' then
					structs_c:write([[
		lua_pushstring(L, (const char*)udata->]]..field.name..[[);
]])
				else
					error("unsupported field size")
				end
				structs_c:write([[
	}
	else
		lua_pushnil(L);
	return 1;
}

]])
			elseif field.luatype=='wstring' then
				structs_c:write([[
static int luahid_get_]]..name..[[_]]..field.name..[[(lua_State* L)
{
	]]..cname..[[* udata;
	udata = luahid_to_]]..name..[[(L, 1);
	if (udata->]]..field.name..[[)
	{
]])
				if type(field.size)=='string' then
					structs_c:write([[
		lua_pushlwstring(L, (const wchar_t*)udata->]]..field.name..[[, udata->]]..field.size..[[);
]])
				elseif type(field.size)=='number' then
					structs_c:write([[
		lua_pushlwstring(L, (const wchar_t*)udata->]]..field.name..[[, ]]..field.size..[[);
]])
				elseif type(field.size)=='nil' then
					structs_c:write([[
		lua_pushwstring(L, (const wchar_t*)udata->]]..field.name..[[);
]])
				else
					error("unsupported field size")
				end
				structs_c:write([[
	}
	else
		lua_pushnil(L);
	return 1;
}

]])
			else
				structs_c:write([[
int luahid_get_]]..name..[[_]]..field.name..[[(lua_State* L);

]])
			end
		end
	end
	structs_c:write([[
static struct luaL_Reg ]]..name..[[__getters[] = {
]])
	if struct.fields then
		for _,field in ipairs(struct.fields) do
			structs_c:write([[
	{"]]..field.name..[[", luahid_get_]]..name..[[_]]..field.name..[[},
]])
		end
	end
	structs_c:write([[
	{0, 0},
};
]])
	
	-- metamethods
	if struct.gc then
		structs_c:write([[
int luahid_]]..name..[[_gc(lua_State* L);
]])
	end
	structs_c:write([[
static struct luaL_Reg ]]..name..[[__metamethods[] = {
	{"__index", luahid_generic_index},
	{"__tostring", luahid_generic_tostring},
]])
	if struct.gc then
		structs_c:write([[
	{"__gc", luahid_]]..name..[[_gc},
]])
	end
	structs_c:write([[
	{0, 0},
};
]])
	
	-- methods
	structs_c:write([[
extern struct luaL_Reg ]]..name..[[__methods[];

]])
end

structs_h:write([[
void luahid_init_structs(lua_State* L);
]])
structs_c:write([[
void luahid_init_structs(lua_State* L)
{
]])

for _,struct in ipairs(structs) do
	local name = struct.name
	local cname = struct.cname
structs_c:write([[
	/* ]]..name..[[ */
	luaL_newmetatable(L, "]]..cname..[[");
	lua_pushvalue(L, 2);
	setfuncs(L, ]]..name..[[__metamethods, 1);
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, ]]..name..[[__methods, 1);
	lua_setfield(L, -2, "methods");
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, ]]..name..[[__getters, 1);
	lua_setfield(L, -2, "getters");
	lua_pushliteral(L, "]]..cname..[[");
	lua_setfield(L, -2, "typename");
	lua_pop(L, 1);
]])
end

structs_c:write([[
}
]])

