/* this is a generated file, see gen.lua */
#include <lua.h>
#include <hidapi.h>

struct hid_device_info* luahid_to_device_info(lua_State* L, int index);
struct hid_device_info* luahid_check_device_info(lua_State* L, int index);
void luahid_push_device_info(lua_State* L, const struct hid_device_info* value, int owner);
hid_device* luahid_to_device(lua_State* L, int index);
hid_device* luahid_check_device(lua_State* L, int index);
void luahid_push_device(lua_State* L, const hid_device* value, int owner);
void luahid_init_structs(lua_State* L);
