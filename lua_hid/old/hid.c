#include <lua.h>
#include <lauxlib.h>
#include <hidapi.h>
#include <string.h>
#include "compat.h"
#include "structs.h"
#include "wstring.h"

/****************************************************************************/

static int luahid__lasterror(lua_State* L, hid_device* device)
{
	lua_pushnil(L);
	lua_pushwstring(L, hid_error(device));
	return 2;
}

/****************************************************************************/

#define BINDING(f) static int lua__libhid_##f(lua_State* L)
#define BIND(f) {#f, lua__libhid_##f},

int luahid_get_device_info_next(lua_State* L)
{
	struct hid_device_info* udata;
	udata = luahid_check_device_info(L, 1);
	if (udata->next)
	{
		/* share owner */
		getuservalue(L, 1); // current, ..., uservalue
		lua_rawgeti(L, -1, 1); // current, ..., uservalue, owner
		luahid_push_device_info(L, udata->next, -1); // current, ..., uservalue, owner, next
	}
	else
		lua_pushnil(L);
	return 1;
}

static int enumeration_gc(lua_State* L)
{
	struct hid_device_info** list;
	list = (struct hid_device_info**)lua_touserdata(L, 1);
	if (list)
	{
		hid_free_enumeration(*list);
		list = NULL;
	}
	return 0;
}

BINDING(enumerate)
{
	unsigned short vendor_id;
	unsigned short product_id;
	struct hid_device_info** result;
	
	vendor_id = (unsigned short)luaL_optnumber(L, 1, 0); /* :FIXME: handle overflow */
	product_id = (unsigned short)luaL_optnumber(L, 2, 0); /* :FIXME: handle overflow */
	
	/* prepare special userdata as owner for list elements */
	result = (struct hid_device_info**)lua_newuserdata(L, sizeof(struct hid_device_info*));
	
	*result = hid_enumerate(vendor_id, product_id);
	if (!*result)
		return 0;
// this is not an error, just means no device found
//		return luaL_error(L, "error in hid_enumerate");
	
	/* complete owner */
	lua_newtable(L);
	lua_pushcfunction(L, enumeration_gc);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	
	/* push first element */
	luahid_push_device_info(L, *result, -1);
	return 1;
}

BINDING(open)
{
	unsigned short vendor_id;
	unsigned short product_id;
	const wchar_t* serial_number;
	hid_device* result;
	
	vendor_id = (unsigned short)luaL_optnumber(L, 1, 0); /* :FIXME: handle overflow */
	product_id = (unsigned short)luaL_optnumber(L, 2, 0); /* :FIXME: handle overflow */
	serial_number = luaL_optwstring(L, 3, NULL);
	
	result = hid_open(vendor_id, product_id, serial_number);
	if (!result)
	{
		lua_pushnil(L);
		return 1;
	}
	
	lua_pushvalue(L, lua_upvalueindex(1));
	luahid_push_device(L, result, -1);
	return 1;
}

BINDING(open_path)
{
	const char* path;
	hid_device* result;
	
	path = luaL_checkstring(L, 1);
	
	result = hid_open_path(path);
	if (!result)
	{
		lua_pushnil(L);
		return 1;
	}
	
	lua_pushvalue(L, lua_upvalueindex(1));
	luahid_push_device(L, result, -1);
	return 1;
}

BINDING(write)
{
	hid_device* device;
	const unsigned char* data;
	size_t length;
	int result;
	
	device = luahid_check_device(L, 1);
	data = luaL_checklstring(L, 2, &length);
	
	result = hid_write(device, data, length);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result < 0)
		return luaL_error(L, "invalid result from hid_write");
	
	lua_pushnumber(L, result);
	return 1;
}

BINDING(read_timeout)
{
	hid_device* device;
	unsigned char* data;
	size_t length;
	int milliseconds;
	int result;
	
	device = luahid_check_device(L, 1);
	length = (size_t)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	milliseconds = (int)luaL_checknumber(L, 3); /* :FIXME: handle overflow */
	
	data = lua_newuserdata(L, length);
	
	result = hid_read_timeout(device, data, length, milliseconds);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result < 0)
		return luaL_error(L, "invalid result from hid_read_timeout");
	
	lua_pushlstring(L, data, result);
	return 1;
}

BINDING(read)
{
	hid_device* device;
	unsigned char* data;
	size_t length;
	int result;
	
	device = luahid_check_device(L, 1);
	length = (size_t)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	
	data = lua_newuserdata(L, length);
	
	result = hid_read(device, data, length);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result < 0)
		return luaL_error(L, "invalid result from hid_read");
	
	lua_pushlstring(L, data, result);
	return 1;
}

BINDING(set_nonblocking)
{
	hid_device* device;
	int nonblock;
	int result;
	
	device = luahid_check_device(L, 1);
	nonblock = lua_toboolean(L, 2) ? 1 : 0;
	
	result = hid_set_nonblocking(device, nonblock);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result != 0)
		return luaL_error(L, "invalid result from hid_set_nonblocking");
	
	lua_pushboolean(L, 1);
	return 1;
}

BINDING(send_feature_report)
{
	hid_device* device;
	unsigned char report_id;
	const unsigned char* report_data;
	unsigned char* data;
	size_t length;
	int result;
	
	device = luahid_check_device(L, 1);
	report_id = (unsigned char)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	report_data = luaL_checklstring(L, 3, &length);
	
	data = lua_newuserdata(L, length + 1);
	data[0] = report_id;
	memcpy(&data[1], report_data, length);
	++length;
	
	result = hid_send_feature_report(device, data, length);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result < 0)
		return luaL_error(L, "invalid result from hid_send_feature_report");
	
	lua_pushnumber(L, result);
	return 1;
}

BINDING(get_feature_report)
{
	hid_device* device;
	unsigned char report_id;
	unsigned char* data;
	size_t length;
	int result;
	
	device = luahid_check_device(L, 1);
	report_id = (unsigned char)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	length = (size_t)luaL_checknumber(L, 3); /* :FIXME: handle overflow */
	
	data = lua_newuserdata(L, length + 1);
	data[0] = report_id;
	++length;
	
	result = hid_get_feature_report(device, data, length);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result < 0)
		return luaL_error(L, "invalid result from hid_get_feature_report");
	
	lua_pushlstring(L, data, result);
	return 1;
}

int luahid_device_gc(lua_State* L)
{
	hid_device** dev;
	
	dev = (hid_device**)lua_touserdata(L, 1);
	if (*dev)
	{
		hid_close(*dev);
		*dev = 0;
	}
	return 0;
}

BINDING(close)
{
	luahid_check_device(L, 1);
	return luahid_device_gc(L);
}

BINDING(get_manufacturer_string)
{
	hid_device* device;
	wchar_t* string;
	size_t maxlen;
	int result;
	
	device = luahid_check_device(L, 1);
	maxlen = (size_t)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	
	string = (wchar_t*)lua_newuserdata(L, sizeof(wchar_t) * maxlen);
	
	result = hid_get_manufacturer_string(device, string, maxlen);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result != 0)
		return luaL_error(L, "invalid result from hid_get_manufacturer_string");
	
	lua_pushwstring(L, string);
	return 1;
}

BINDING(get_product_string)
{
	hid_device* device;
	wchar_t* string;
	size_t maxlen;
	int result;
	
	device = luahid_check_device(L, 1);
	maxlen = (size_t)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	
	string = (wchar_t*)lua_newuserdata(L, sizeof(wchar_t) * maxlen);
	
	result = hid_get_product_string(device, string, maxlen);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result != 0)
		return luaL_error(L, "invalid result from hid_get_product_string");
	
	lua_pushwstring(L, string);
	return 1;
}

BINDING(get_serial_number_string)
{
	hid_device* device;
	wchar_t* string;
	size_t maxlen;
	int result;
	
	device = luahid_check_device(L, 1);
	maxlen = (size_t)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	
	string = (wchar_t*)lua_newuserdata(L, sizeof(wchar_t) * maxlen);
	
	result = hid_get_serial_number_string(device, string, maxlen);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result != 0)
		return luaL_error(L, "invalid result from hid_get_serial_number_string");
	
	lua_pushwstring(L, string);
	return 1;
}

BINDING(get_indexed_string)
{
	hid_device* device;
	int string_index;
	wchar_t* string;
	size_t maxlen;
	int result;
	
	device = luahid_check_device(L, 1);
	string_index = (int)luaL_checknumber(L, 2); /* :FIXME: handle overflow */
	maxlen = (size_t)luaL_checknumber(L, 3); /* :FIXME: handle overflow */
	
	string = (wchar_t*)lua_newuserdata(L, sizeof(wchar_t) * maxlen);
	
	result = hid_get_indexed_string(device, string_index, string, maxlen);
	if (result == -1)
		return luahid__lasterror(L, device);
	else if (result != 0)
		return luaL_error(L, "invalid result from hid_get_indexed_string");
	
	lua_pushwstring(L, string);
	return 1;
}

/****************************************************************************/

static luaL_Reg functions[] = {
	BIND(enumerate)
	BIND(open)
	BIND(open_path)
	{0, 0},
};

struct luaL_Reg device__methods[] = {
	BIND(write)
	BIND(read_timeout)
	BIND(read)
	BIND(set_nonblocking)
	BIND(send_feature_report)
	BIND(get_feature_report)
	BIND(close)
	BIND(get_manufacturer_string)
	BIND(get_product_string)
	BIND(get_serial_number_string)
	BIND(get_indexed_string)
	{0, 0},
};

struct luaL_Reg device_info__methods[] = {
	{0, 0},
};

/****************************************************************************/

static int module_gc(lua_State* L)
{
	hid_exit();
	return 0;
}

LUAMOD_API int luaopen_hid(lua_State* L)
{
	int i;
#if LUA_VERSION_NUM==501
	struct luaL_Reg empty[] = {{0,0}};
#endif
	
	i = hid_init();
	if (i != 0)
		return luaL_error(L, "error in hid_init");
	
	/* module */
#if LUA_VERSION_NUM==502
	lua_newtable(L);
#elif LUA_VERSION_NUM==501
	luaL_register(L, lua_tostring(L, 1), empty);
#else
#error unsupported Lua version
#endif
	lua_replace(L, 1); /* code below assumes module is at index 1 */
	lua_settop(L, 1); /* code below assumes exit userdata is at index 2 */
	
	/* exit userdata */
	lua_newuserdata(L, 0);
	lua_newtable(L);
	lua_pushcfunction(L, module_gc);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
	
	/* init structs */
	luahid_init_structs(L);
	
	/* module functions */
	lua_pushvalue(L, 1); /* ..., module */
	lua_pushvalue(L, 2); /* ..., module, udata */
	setfuncs(L, functions, 1); /* ..., module */
	
	lua_settop(L, 1); /* code below assumes exit userdata is at index 2 */
	/* return module */
	return 1;
}

