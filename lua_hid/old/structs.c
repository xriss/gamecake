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

struct hid_device_info* luahid_to_device_info(lua_State* L, int index)
{
	struct hid_device_info* udata;
	udata = *(struct hid_device_info**)lua_touserdata(L, index);
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

struct hid_device_info* luahid_check_device_info(lua_State* L, int index)
{
	struct hid_device_info* udata;
	udata = *(struct hid_device_info**)luaL_checkudata(L, index, "struct hid_device_info");
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

void luahid_push_device_info(lua_State* L, const struct hid_device_info* value, int owner)
{
	const struct hid_device_info** udata;
	if (owner < 0) owner = lua_gettop(L) + 1 + owner;
	udata = (const struct hid_device_info**)lua_newuserdata(L, sizeof(struct hid_device_info*));
	*udata = value;
	luaL_getmetatable(L, "struct hid_device_info");
	lua_setmetatable(L, -2);
	if (owner != 0)
	{
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, owner);
		lua_rawseti(L, -2, 1);
		setuservalue(L, -2);
	}
}

static int luahid_get_device_info_path(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	if (udata->path)
	{
		lua_pushstring(L, (const char*)udata->path);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int luahid_get_device_info_vendor_id(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->vendor_id);
	return 1;
}

static int luahid_get_device_info_product_id(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->product_id);
	return 1;
}

static int luahid_get_device_info_serial_number(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	if (udata->serial_number)
	{
		lua_pushwstring(L, (const wchar_t*)udata->serial_number);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int luahid_get_device_info_release_number(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->release_number);
	return 1;
}

static int luahid_get_device_info_manufacturer_string(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	if (udata->manufacturer_string)
	{
		lua_pushwstring(L, (const wchar_t*)udata->manufacturer_string);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int luahid_get_device_info_product_string(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	if (udata->product_string)
	{
		lua_pushwstring(L, (const wchar_t*)udata->product_string);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int luahid_get_device_info_usage_page(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->usage_page);
	return 1;
}

static int luahid_get_device_info_usage(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->usage);
	return 1;
}

static int luahid_get_device_info_interface_number(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_to_device_info(L, 1);
	lua_pushnumber(L, udata->interface_number);
	return 1;
}

int luahid_get_device_info_next(lua_State* L);

static struct luaL_Reg device_info__getters[] = {
	{"path", luahid_get_device_info_path},
	{"vendor_id", luahid_get_device_info_vendor_id},
	{"product_id", luahid_get_device_info_product_id},
	{"serial_number", luahid_get_device_info_serial_number},
	{"release_number", luahid_get_device_info_release_number},
	{"manufacturer_string", luahid_get_device_info_manufacturer_string},
	{"product_string", luahid_get_device_info_product_string},
	{"usage_page", luahid_get_device_info_usage_page},
	{"usage", luahid_get_device_info_usage},
	{"interface_number", luahid_get_device_info_interface_number},
	{"next", luahid_get_device_info_next},
	{0, 0},
};
static struct luaL_Reg device_info__metamethods[] = {
	{"__index", luahid_generic_index},
	{"__tostring", luahid_generic_tostring},
	{0, 0},
};
extern struct luaL_Reg device_info__methods[];

hid_device* luahid_to_device(lua_State* L, int index)
{
	hid_device* udata;
	udata = *(hid_device**)lua_touserdata(L, index);
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

hid_device* luahid_check_device(lua_State* L, int index)
{
	hid_device* udata;
	udata = *(hid_device**)luaL_checkudata(L, index, "hid_device");
	if (!udata)
	{
		luaL_argerror(L, index, "context has already been deinitialized");
		return NULL;
	}
	return udata;
}

void luahid_push_device(lua_State* L, const hid_device* value, int owner)
{
	const hid_device** udata;
	if (owner < 0) owner = lua_gettop(L) + 1 + owner;
	udata = (const hid_device**)lua_newuserdata(L, sizeof(hid_device*));
	*udata = value;
	luaL_getmetatable(L, "hid_device");
	lua_setmetatable(L, -2);
	if (owner != 0)
	{
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, owner);
		lua_rawseti(L, -2, 1);
		setuservalue(L, -2);
	}
}

static struct luaL_Reg device__getters[] = {
	{0, 0},
};
int luahid_device_gc(lua_State* L);
static struct luaL_Reg device__metamethods[] = {
	{"__index", luahid_generic_index},
	{"__tostring", luahid_generic_tostring},
	{"__gc", luahid_device_gc},
	{0, 0},
};
extern struct luaL_Reg device__methods[];

void luahid_init_structs(lua_State* L)
{
	/* device_info */
	luaL_newmetatable(L, "struct hid_device_info");
	lua_pushvalue(L, 2);
	setfuncs(L, device_info__metamethods, 1);
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, device_info__methods, 1);
	lua_setfield(L, -2, "methods");
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, device_info__getters, 1);
	lua_setfield(L, -2, "getters");
	lua_pushliteral(L, "struct hid_device_info");
	lua_setfield(L, -2, "typename");
	lua_pop(L, 1);
	/* device */
	luaL_newmetatable(L, "hid_device");
	lua_pushvalue(L, 2);
	setfuncs(L, device__metamethods, 1);
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, device__methods, 1);
	lua_setfield(L, -2, "methods");
	lua_newtable(L);
	lua_pushvalue(L, 2);
	setfuncs(L, device__getters, 1);
	lua_setfield(L, -2, "getters");
	lua_pushliteral(L, "hid_device");
	lua_setfield(L, -2, "typename");
	lua_pop(L, 1);
}
