/* The MIT License (MIT)
 *
 * Copyright (c) 2021 Stefano Trettel
 *
 * Software repository: MoonUSB, https://github.com/stetre/moonusb
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

/*------------------------------------------------------------------------------*
 | Code<->string map for enumerations                                           |
 *------------------------------------------------------------------------------*/

/* code <-> string record */
#define rec_t struct rec_s
struct rec_s {
    RB_ENTRY(rec_s) CodeEntry;
    RB_ENTRY(rec_s) StringEntry;
    int domain;
    int code;  /* (domain, code) = search key in code tree */
    char     *str;  /* (domain, str) = search key in string tree */
};

/* compare functions */
static int cmp_code(rec_t *rec1, rec_t *rec2) 
    { 
    if(rec1->domain != rec2->domain)
        return (rec1->domain < rec2->domain ? -1 : rec1->domain > rec2->domain);
    return (rec1->code < rec2->code ? -1 : rec1->code > rec2->code);
    } 

static int cmp_str(rec_t *rec1, rec_t *rec2) 
    { 
    if(rec1->domain != rec2->domain)
        return (rec1->domain < rec2->domain ? -1 : rec1->domain > rec2->domain);
    return strcmp(rec1->str, rec2->str);
    } 

static RB_HEAD(CodeTree, rec_s) CodeHead = RB_INITIALIZER(&CodeHead);
RB_PROTOTYPE_STATIC(CodeTree, rec_s, CodeEntry, cmp_code) 
RB_GENERATE_STATIC(CodeTree, rec_s, CodeEntry, cmp_code) 

static RB_HEAD(StringTree, rec_s) StringHead = RB_INITIALIZER(&StringHead);
RB_PROTOTYPE_STATIC(StringTree, rec_s, StringEntry, cmp_str) 
RB_GENERATE_STATIC(StringTree, rec_s, StringEntry, cmp_str) 
 
static rec_t *code_remove(rec_t *rec) 
    { return RB_REMOVE(CodeTree, &CodeHead, rec); }
static rec_t *code_insert(rec_t *rec) 
    { return RB_INSERT(CodeTree, &CodeHead, rec); }
static rec_t *code_search(int domain, int code) 
    { rec_t tmp; tmp.domain = domain; tmp.code = code; return RB_FIND(CodeTree, &CodeHead, &tmp); }
static rec_t *code_first(int domain, int code) 
    { rec_t tmp; tmp.domain = domain; tmp.code = code; return RB_NFIND(CodeTree, &CodeHead, &tmp); }
static rec_t *code_next(rec_t *rec)
    { return RB_NEXT(CodeTree, &CodeHead, rec); }
#if 0
static rec_t *code_prev(rec_t *rec)
    { return RB_PREV(CodeTree, &CodeHead, rec); }
static rec_t *code_min(void)
    { return RB_MIN(CodeTree, &CodeHead); }
static rec_t *code_max(void)
    { return RB_MAX(CodeTree, &CodeHead); }
static rec_t *code_root(void)
    { return RB_ROOT(&CodeHead); }
#endif
 
static rec_t *str_remove(rec_t *rec) 
    { return RB_REMOVE(StringTree, &StringHead, rec); }
static rec_t *str_insert(rec_t *rec) 
    { return RB_INSERT(StringTree, &StringHead, rec); }
static rec_t *str_search(int domain, const char* str) 
    { rec_t tmp; tmp.domain = domain; tmp.str = (char*)str; return RB_FIND(StringTree, &StringHead, &tmp); }
#if 0
static rec_t *str_first(int domain, const char* str ) 
    { rec_t tmp; tmp.domain = domain; tmp.str = str; return RB_NFIND(StringTree, &StringHead, &tmp); }
static rec_t *str_next(rec_t *rec)
    { return RB_NEXT(StringTree, &StringHead, rec); }
static rec_t *str_prev(rec_t *rec)
    { return RB_PREV(StringTree, &StringHead, rec); }
static rec_t *str_min(void)
    { return RB_MIN(StringTree, &StringHead); }
static rec_t *str_max(void)
    { return RB_MAX(StringTree, &StringHead); }
static rec_t *str_root(void)
    { return RB_ROOT(&StringHead); }
#endif


static int enums_new(lua_State *L, int domain, int code, const char *str)
    {
    rec_t *rec;
    if((rec = (rec_t*)Malloc(L, sizeof(rec_t))) == NULL) 
        return luaL_error(L, errstring(ERR_MEMORY));

    memset(rec, 0, sizeof(rec_t));
    rec->domain = domain;
    rec->code = code;
    rec->str = Strdup(L, str);
    if(code_search(domain, code) || str_search(domain, str))
        { 
        Free(L, rec->str);
        Free(L, rec); 
        return unexpected(L); /* duplicate value */
        }
    code_insert(rec);
    str_insert(rec);
    return 0;
    }

static void enums_free(lua_State *L, rec_t* rec)
    {
    if(code_search(rec->domain, rec->code) == rec)
        code_remove(rec);
    if(str_search(rec->domain, rec->str) == rec)
        str_remove(rec);
    Free(L, rec->str);
    Free(L, rec);   
    }

void enums_free_all(lua_State *L)
    {
    rec_t *rec;
//    while((rec = code_first(0, 0)))
 //       enums_free(L, rec);
    }

#if 0
int enums_code(int domain, const char *str, int* found)
    {
    rec_t *rec = str_search(domain, str);
    if(!rec)
        { *found = 0; return 0; }
    *found = 1; 
    return rec->code;
    }

const char* enums_string(int domain, int code)
    {
    rec_t *rec = code_search(domain, code);
    if(!rec)
        return NULL;
    return rec->str;
    }

#endif

int enums_test(lua_State *L, int domain, int arg, int *err)
    {
    rec_t *rec;
    const char *s = luaL_optstring(L, arg, NULL);
    if(!s) { *err = ERR_NOTPRESENT; return 0; }
    rec = str_search(domain, s);
    if(!rec) { *err = ERR_VALUE; return 0; }
    *err = ERR_SUCCESS;
    return rec->code;
    }

int enums_opt(lua_State *L, int domain, int arg, int defval)
    {
    rec_t *rec;
    const char *s = luaL_optstring(L, arg, NULL);
    if(!s) { return defval; }
    rec = str_search(domain, s);
    if(!rec) return luaL_argerror(L, arg, badvalue(L, s));
    return rec->code;
    }

int enums_check(lua_State *L, int domain, int arg)
    {
    rec_t *rec;
    const char *s = luaL_checkstring(L, arg);
    rec = str_search(domain, s);
    if(!rec) return luaL_argerror(L, arg, badvalue(L, s));
    return rec->code;
    }

int enums_push(lua_State *L, int domain, int code)
    {
    rec_t *rec = code_search(domain, code);
    if(!rec) return unexpected(L);
    lua_pushstring(L, rec->str);
    return 1;
    }

int enums_values(lua_State *L, int domain)
    {
    int i;
    rec_t *rec;

    lua_newtable(L);
    i = 1;
    rec = code_first(domain, 0);
    while(rec)
        {
        if(rec->domain == domain)
            {
            lua_pushstring(L, rec->str);
            lua_rawseti(L, -2, i++);
            }
        rec = code_next(rec);
        }

    return 1;
    }


int* enums_checklist(lua_State *L, int domain, int arg, int *count, int *err)
    {
    int* list;
    int i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_NOTPRESENT; return NULL; }

    list = (int*)MallocNoErr(L, sizeof(int) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        list[i] = enums_test(L, domain, -1, err);
        lua_pop(L, 1);
        if(*err)
            { Free(L, list); *count = 0; return NULL; }
        }
    return list;
    }

void enums_freelist(lua_State *L, int *list)
    {
    if(!list) return;
    Free(L, list);
    }

/*------------------------------------------------------------------------------*
 |                                                                              |
 *------------------------------------------------------------------------------*/

static int Enum(lua_State *L)
/* { strings } = ode.enum('type') lists all the values for a given enum type */
    { 
    const char *s = luaL_checkstring(L, 1); 
#define CASE(xxx) if(strcmp(s, ""#xxx) == 0) return values##xxx(L)
    CASE(type);
    CASE(class);
    CASE(speed);
    CASE(option);
    CASE(loglevel);
    CASE(direction);
    CASE(transfertype);
    CASE(standardrequest);
    CASE(requesttype);
    CASE(requestrecipient);
    CASE(isosynctype);
    CASE(isousagetype);
    CASE(capability);
    CASE(hotplugevent);
    CASE(transferstatus);
    CASE(bostype);
#undef CASE
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "enum", Enum },
        { NULL, NULL } /* sentinel */
    };


void moonusb_open_enums(lua_State *L)
    {
    int domain;

    luaL_setfuncs(L, Functions, 0);

    /* Add all the code<->string mappings */
#define ADD(what, s) do { enums_new(L, domain, what, s); } while(0)
    domain = DOMAIN_TYPE; /* non-libusb */
    ADD(MOONUSB_TYPE_CHAR, "char");
    ADD(MOONUSB_TYPE_UCHAR, "uchar");
    ADD(MOONUSB_TYPE_SHORT, "short");
    ADD(MOONUSB_TYPE_USHORT, "ushort");
    ADD(MOONUSB_TYPE_INT, "int");
    ADD(MOONUSB_TYPE_UINT, "uint");
    ADD(MOONUSB_TYPE_LONG, "long");
    ADD(MOONUSB_TYPE_ULONG, "ulong");
    ADD(MOONUSB_TYPE_FLOAT, "float");
    ADD(MOONUSB_TYPE_DOUBLE, "double");

    domain = DOMAIN_CLASS;
    ADD(CLASS_PER_INTERFACE, "per interface");
    ADD(CLASS_AUDIO, "audio");
    ADD(CLASS_CDC, "cdc");
    ADD(CLASS_HID, "hid");
    ADD(CLASS_PHYSICAL, "physical");
    ADD(CLASS_IMAGE, "image");
    ADD(CLASS_PRINTER, "printer");
    ADD(CLASS_MASS_STORAGE, "mass storage");
    ADD(CLASS_HUB, "hub");
    ADD(CLASS_CDC_DATA, "cdc data");
    ADD(CLASS_SMART_CARD, "smart card");
    ADD(CLASS_CONTENT_SECURITY, "content security");
    ADD(CLASS_VIDEO, "video");
    ADD(CLASS_PERSONAL_HEALTHCARE, "personal healthcare");
    ADD(CLASS_AUDIO_VIDEO, "audio video");
    ADD(CLASS_BILLBOARD, "billboard");
    ADD(CLASS_TYPE_C_BRIDGE, "type c bridge");
    ADD(CLASS_DIAGNOSTIC, "diagnostic");
    ADD(CLASS_WIRELESS, "wireless");
    ADD(CLASS_MISCELLANEOUS, "miscellaneous");
    ADD(CLASS_APPLICATION_SPECIFIC, "application specific");
    ADD(CLASS_VENDOR_SPECIFIC, "vendor specific");

    domain = DOMAIN_SPEED; /* libusb_speed */
    ADD(LIBUSB_SPEED_UNKNOWN, "unknown");
    ADD(LIBUSB_SPEED_LOW, "low");
    ADD(LIBUSB_SPEED_FULL, "full");
    ADD(LIBUSB_SPEED_HIGH, "high");
    ADD(LIBUSB_SPEED_SUPER, "super");
    ADD(LIBUSB_SPEED_SUPER_PLUS, "super plus");

    domain = DOMAIN_OPTION; /* libusb_option */
    ADD(LIBUSB_OPTION_LOG_LEVEL, "log level");
    ADD(LIBUSB_OPTION_USE_USBDK, "use usbdk");
    ADD(LIBUSB_OPTION_WEAK_AUTHORITY, "weak authority");

    domain = DOMAIN_LOG_LEVEL; /* libusb_log_level */
    ADD(LIBUSB_LOG_LEVEL_NONE, "none");
    ADD(LIBUSB_LOG_LEVEL_ERROR, "error");
    ADD(LIBUSB_LOG_LEVEL_WARNING, "warning");
    ADD(LIBUSB_LOG_LEVEL_INFO, "info");
    ADD(LIBUSB_LOG_LEVEL_DEBUG, "debug");

    domain = DOMAIN_DIRECTION; /* libusb_endpoint_direction */
    ADD(LIBUSB_ENDPOINT_OUT, "out");
    ADD(LIBUSB_ENDPOINT_IN, "in");

    domain = DOMAIN_TRANSFER_TYPE; /* libusb_endpoint_transfer_type */
    ADD(LIBUSB_ENDPOINT_TRANSFER_TYPE_CONTROL, "control");
    ADD(LIBUSB_ENDPOINT_TRANSFER_TYPE_ISOCHRONOUS, "isochronous");
    ADD(LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK, "bulk");
    ADD(LIBUSB_ENDPOINT_TRANSFER_TYPE_INTERRUPT, "interrupt");

    domain = DOMAIN_STANDARD_REQUEST; /* libusb_standard_request */
    ADD(LIBUSB_REQUEST_GET_STATUS, "get status");
    ADD(LIBUSB_REQUEST_CLEAR_FEATURE, "clear feature");
    ADD(LIBUSB_REQUEST_SET_FEATURE, "set feature");
    ADD(LIBUSB_REQUEST_SET_ADDRESS, "set address");
    ADD(LIBUSB_REQUEST_GET_DESCRIPTOR, "get descriptor");
    ADD(LIBUSB_REQUEST_SET_DESCRIPTOR, "set descriptor");
    ADD(LIBUSB_REQUEST_GET_CONFIGURATION, "get configuration");
    ADD(LIBUSB_REQUEST_SET_CONFIGURATION, "set configuration");
    ADD(LIBUSB_REQUEST_GET_INTERFACE, "get interface");
    ADD(LIBUSB_REQUEST_SET_INTERFACE, "set interface");
    ADD(LIBUSB_REQUEST_SYNCH_FRAME, "synch frame");
    ADD(LIBUSB_REQUEST_SET_SEL, "set sel");
    ADD(LIBUSB_SET_ISOCH_DELAY, "set isoch delay");

    domain = DOMAIN_REQUEST_TYPE; /* libusb_request_type */
    ADD(LIBUSB_REQUEST_TYPE_STANDARD, "standard");
    ADD(LIBUSB_REQUEST_TYPE_CLASS, "class");
    ADD(LIBUSB_REQUEST_TYPE_VENDOR, "vendor");
    ADD(LIBUSB_REQUEST_TYPE_RESERVED, "reserved");

    domain = DOMAIN_REQUEST_RECIPIENT; /* libusb_request_recipient */
    ADD(LIBUSB_RECIPIENT_DEVICE, "device");
    ADD(LIBUSB_RECIPIENT_INTERFACE, "interface");
    ADD(LIBUSB_RECIPIENT_ENDPOINT, "endpoint");
    ADD(LIBUSB_RECIPIENT_OTHER, "other");

    domain = DOMAIN_ISO_SYNC_TYPE; /* libusb_iso_sync_type */
    ADD(LIBUSB_ISO_SYNC_TYPE_NONE, "none");
    ADD(LIBUSB_ISO_SYNC_TYPE_ASYNC, "async");
    ADD(LIBUSB_ISO_SYNC_TYPE_ADAPTIVE, "adaptive");
    ADD(LIBUSB_ISO_SYNC_TYPE_SYNC, "sync");

    domain = DOMAIN_ISO_USAGE_TYPE; /* libusb_iso_usage_type */
    ADD(LIBUSB_ISO_USAGE_TYPE_DATA, "data");
    ADD(LIBUSB_ISO_USAGE_TYPE_FEEDBACK, "feedback");
    ADD(LIBUSB_ISO_USAGE_TYPE_IMPLICIT, "implicit");

    domain = DOMAIN_CAPABILITY; /* libusb_capability */
    ADD(LIBUSB_CAP_HAS_HOTPLUG, "hotplug");
    ADD(LIBUSB_CAP_HAS_HID_ACCESS, "hid access");
    ADD(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER, "detach kernel driver");

    domain = DOMAIN_HOTPLUG_EVENT ; /* libusb_hotplug_event + libusb_hotplug_flags */
    ADD(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, "attached");
    ADD(LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, "detached");

    domain = DOMAIN_TRANSFER_STATUS; /* libusb_transfer_status */
    ADD(LIBUSB_TRANSFER_COMPLETED, "completed");
    ADD(LIBUSB_TRANSFER_ERROR, "error");
    ADD(LIBUSB_TRANSFER_TIMED_OUT, "timeout");
    ADD(LIBUSB_TRANSFER_CANCELLED, "cancelled");
    ADD(LIBUSB_TRANSFER_STALL, "stall");
    ADD(LIBUSB_TRANSFER_NO_DEVICE, "no device");
    ADD(LIBUSB_TRANSFER_OVERFLOW, "overflow");

    domain = DOMAIN_BOS_TYPE; /* libusb_bos_type */
    ADD(BOS_TYPE_WIRELESS_USB, "wireless usb");
    ADD(BOS_TYPE_USB_2_0_EXTENSION, "usb 2 0 extension");
    ADD(BOS_TYPE_SUPERSPEED_USB, "superspeed usb");
    ADD(BOS_TYPE_CONTAINER_ID, "container id");
    ADD(BOS_TYPE_PLATFORM, "platform");
    ADD(BOS_TYPE_POWER_DELIVERY, "power delivery");
    ADD(BOS_TYPE_BATTERY_INFO, "battery info");
    ADD(BOS_TYPE_PD_CONSUMER_PORT, "pd consumer port");
    ADD(BOS_TYPE_PD_PROVIDER_PORT, "pd provider port");
    ADD(BOS_TYPE_SUPERSPEED_PLUS, "superspeed plus");
    ADD(BOS_TYPE_PRECISION_TIME_MEASUREMENT, "precision time measurement");
    ADD(BOS_TYPE_WIRELESS_USB_EXT, "wireless usb ext");
    ADD(BOS_TYPE_BILLBOARD, "billboard");
    ADD(BOS_TYPE_AUTHENTICATION, "authentication");
    ADD(BOS_TYPE_BILLBOARD_EX, "billboard ex");
    ADD(BOS_TYPE_CONFIGURATION_SUMMARY, "configuration summary");
#undef ADD
    }

