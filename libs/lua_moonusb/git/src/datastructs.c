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

unsigned char* checkucharlist(lua_State *L, int arg, int *count, int *err)
/* freesizelist() --> Free() */
    {
    unsigned char* list;
    int i;
    int isnum;
    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (unsigned char*)MallocNoErr(L, sizeof(unsigned char)*(*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }
    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        list[i] = lua_tointegerx(L, -1, &isnum);
        lua_pop(L, 1);
        if(!isnum)
            { Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        }
    return list;
    }

int checkcontrolsetup(lua_State *L, int arg, struct libusb_control_setup *dst)
    {
    uint8_t recipient, direction, type;
    if(!lua_istable(L, arg)) return argerror(L, arg, ERR_TABLE);
    lua_getfield(L, arg, "request_recipient");
    recipient = checkrequestrecipient(L, -1); lua_pop(L, 1);
    lua_getfield(L, arg, "request_type");
    type = checkrequesttype(L, -1); lua_pop(L, 1);
    lua_getfield(L, arg, "direction");
    direction = checkdirection(L, -1); lua_pop(L, 1);
    dst->bmRequestType = recipient|type|direction;
    lua_getfield(L, arg, "request");
    if(type == LIBUSB_REQUEST_TYPE_STANDARD)
        dst->bRequest = checkstandardrequest(L, -1);
    else
        dst->bRequest = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, arg, "value"); dst->wValue = luaL_checkinteger(L, -1); lua_pop(L, 1);
    lua_getfield(L, arg, "index"); dst->wIndex = luaL_checkinteger(L, -1); lua_pop(L, 1);
    lua_getfield(L, arg, "length"); dst->wLength = luaL_optinteger(L, -1, 0); lua_pop(L, 1);
    return 0;
    }

void pushcontrolsetup(lua_State *L, const struct libusb_control_setup *s)
    {
    lua_newtable(L);
    pushrequesttype(L, s->bmRequestType & 0x60); lua_setfield(L, -2, "request_type");
    pushrequestrecipient(L, s->bmRequestType & 0x1f); lua_setfield(L, -2, "request_recipient");
    pushdirection(L, s->bmRequestType & 0x80); lua_setfield(L, -2, "direction");
    if((s->bmRequestType & 0x60)==LIBUSB_REQUEST_TYPE_STANDARD)
        pushstandardrequest(L, s->bRequest);
    else
        lua_pushinteger(L,  s->bRequest);
    lua_setfield(L, -2, "request");
    lua_pushinteger(L, s->wValue); lua_setfield(L, -2, "value");
    lua_pushinteger(L, s->wIndex); lua_setfield(L, -2, "index");
    lua_pushinteger(L, s->wLength); lua_setfield(L, -2, "length");
    }

static void pushbcdversion(lua_State *L, uint16_t val)
    {
    lua_pushfstring(L, "%d%d.%d%d", (val>>12)&0x000f, (val>>8)&0x000f, (val>>4)&0x000f, (val)&0x000f);
    }

int pushdevicedescriptor(lua_State *L, const struct libusb_device_descriptor *s, device_t *device, context_t *context)
    {
    int i, ec;
    struct libusb_config_descriptor *conf;
    lua_newtable(L);
    pushbcdversion(L, s->bcdUSB);
    lua_setfield(L, -2, "usb_version");
    pushclass(L, s->bDeviceClass);
    lua_setfield(L, -2, "class");
    lua_pushinteger(L, s->bDeviceSubClass);
    lua_setfield(L, -2, "subclass");
    lua_pushinteger(L, s->bDeviceProtocol);
    lua_setfield(L, -2, "protocol");
    lua_pushinteger(L, s->bMaxPacketSize0);
    lua_setfield(L, -2, "max_packet_size_0");
    lua_pushinteger(L, s->idVendor);
    lua_setfield(L, -2, "vendor_id");
    lua_pushinteger(L, s->idProduct);
    lua_setfield(L, -2, "product_id");
    pushbcdversion(L, s->bcdDevice);
    lua_setfield(L, -2, "release_number");
    lua_pushinteger(L, s->iManufacturer);
    lua_setfield(L, -2, "manufacturer_index");
    lua_pushinteger(L, s->iProduct);
    lua_setfield(L, -2, "product_index");
    lua_pushinteger(L, s->iSerialNumber);
    lua_setfield(L, -2, "serial_number_index");
    lua_pushinteger(L, s->bNumConfigurations);
    lua_setfield(L, -2, "num_configurations");
    lua_newtable(L);
    for(i=0; i<s->bNumConfigurations; i++)
        {
        ec = libusb_get_config_descriptor(device, i, &conf);
        CheckError(L, ec);
        pushconfigdescriptor(L, conf, context);
        lua_rawseti(L, -2, i+1);
        libusb_free_config_descriptor(conf);
        }
    lua_setfield(L, -2, "configuration");
    return 0;
    }

static void pushssendpointcompaniondescriptor(lua_State *L, const struct libusb_ss_endpoint_companion_descriptor *s)
    {
    lua_newtable(L);
    lua_pushinteger(L, s->bMaxBurst); lua_setfield(L, -2, "max_burst");
    lua_pushinteger(L, s->bmAttributes); lua_setfield(L, -2, "attributes");
    lua_pushinteger(L, s->wBytesPerInterval); lua_setfield(L, -2, "bytes_per_interval");
    }

static void pushendpointdescriptor(lua_State *L, const struct libusb_endpoint_descriptor *s, context_t *context)
    {
    int transfer_type, ec;
    struct libusb_ss_endpoint_companion_descriptor *ep_comp;
    lua_newtable(L);
    lua_pushinteger(L, s->bEndpointAddress);
    lua_setfield(L, -2, "address");
    lua_pushinteger(L, s->bEndpointAddress&0x0f);
    lua_setfield(L, -2, "number");
    pushdirection(L, s->bEndpointAddress&0x80);
    lua_setfield(L, -2, "direction");
    transfer_type =  s->bmAttributes&0x03;
    pushtransfertype(L, transfer_type);
    lua_setfield(L, -2, "transfer_type");
    if(transfer_type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
        {
        pushisosynctype(L, (s->bmAttributes>>2)&0x03);
        lua_setfield(L, -2, "iso_sync_type");
        pushisousagetype(L, (s->bmAttributes>>4)&0x03);
        lua_setfield(L, -2, "iso_usage_type");
        }
    lua_pushinteger(L, s->wMaxPacketSize);
    lua_setfield(L, -2, "max_packet_size");
    lua_pushinteger(L, s->bInterval);
    lua_setfield(L, -2, "interval");
    lua_pushinteger(L, s->bRefresh);
    lua_setfield(L, -2, "refresh");
    lua_pushinteger(L, s->bSynchAddress);
    lua_setfield(L, -2, "synch_address");
    if(s->extra && s->extra_length>0)
        {
        lua_pushlstring(L, (char*)s->extra, s->extra_length);
        lua_setfield(L, -2, "extra");
        }
    ec = libusb_get_ss_endpoint_companion_descriptor(context, s, &ep_comp);
    if(!ec)
        {
        pushssendpointcompaniondescriptor(L, ep_comp);
        lua_setfield(L, -2, "ss_endpoint_companion_descriptor");
        libusb_free_ss_endpoint_companion_descriptor(ep_comp);
        }
    }

static void pushinterfacedescriptor(lua_State *L, const struct libusb_interface_descriptor *s, context_t *context)
    {
    lua_newtable(L);
    lua_pushinteger(L, s->bInterfaceNumber);
    lua_setfield(L, -2, "number");
    lua_pushinteger(L, s->bAlternateSetting);
    lua_setfield(L, -2, "alt_setting");
    pushclass(L, s->bInterfaceClass);
    lua_setfield(L, -2, "class");
    lua_pushinteger(L, s->bInterfaceSubClass);
    lua_setfield(L, -2, "subclass");
    lua_pushinteger(L, s->bInterfaceProtocol);
    lua_setfield(L, -2, "protocol");
    lua_pushinteger(L, s->iInterface);
    lua_setfield(L, -2, "index");
    lua_pushinteger(L, s->bNumEndpoints);
    lua_setfield(L, -2, "num_endpoints");
    lua_newtable(L);
    for(unsigned int i=0; i<s->bNumEndpoints; i++)
        {
        pushendpointdescriptor(L, &s->endpoint[i], context);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "endpoint");
    if(s->extra && s->extra_length>0)
        {
        lua_pushlstring(L, (char*)s->extra, s->extra_length);
        lua_setfield(L, -2, "extra");
        }
    }

static void pushinterfacealtsettings(lua_State *L, const struct libusb_interface *s, context_t *context)
    {
    lua_newtable(L);
    for(int i=0; i<s->num_altsetting; i++)
        {
        pushinterfacedescriptor(L, &s->altsetting[i], context);
        lua_rawseti(L, -2, i+1);
        }
    }

void pushconfigdescriptor(lua_State *L, const struct libusb_config_descriptor *s, context_t *context)
    {
    lua_newtable(L);
    lua_pushinteger(L, s->bNumInterfaces);
    lua_setfield(L, -2, "num_interfaces");
    lua_pushinteger(L, s->bConfigurationValue);
    lua_setfield(L, -2, "value");
    lua_pushinteger(L, s->iConfiguration);
    lua_setfield(L, -2, "index");
    lua_pushboolean(L, s->bmAttributes & 0x40);
    lua_setfield(L, -2, "self_powered");
    lua_pushboolean(L, s->bmAttributes & 0x20);
    lua_setfield(L, -2, "remote_wakeup");
    lua_pushinteger(L, s->MaxPower);
    lua_setfield(L, -2, "max_power");
    lua_newtable(L);
    for(unsigned int i=0; i<s->bNumInterfaces; i++)
        {
        pushinterfacealtsettings(L, &s->interface[i], context);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "interface");
    if(s->extra && s->extra_length>0)
        {
        lua_pushlstring(L, (char*)s->extra, s->extra_length);
        lua_setfield(L, -2, "extra");
        }
    }

#if 0 // not used
static void pushusb_2_0_extension_descriptor(lua_State *L, const struct libusb_usb_2_0_extension_descriptor *s)
    {
    lua_newtable(L);
    pushbostype(L, s->bDevCapabilityType); lua_setfield(L, -2, "type");
    lua_pushinteger(L, s->bmAttributes); lua_setfield(L, -2, "attributes");
    }

static void pushssusbdevicecapabilitydescriptor(lua_State *L, const struct libusb_ss_usb_device_capability_descriptor *s)
    {
    lua_newtable(L);
    pushbostype(L, s->bDevCapabilityType); lua_setfield(L, -2, "type");
    lua_pushinteger(L, s->bmAttributes); lua_setfield(L, -2, "attributes");
    lua_pushinteger(L, s->wSpeedSupported); lua_setfield(L, -2, "speed_supported");
    lua_pushinteger(L, s->bFunctionalitySupport); lua_setfield(L, -2, "functionality_support");
    lua_pushinteger(L, s->bU1DevExitLat); lua_setfield(L, -2, "u1_device_exit_latency");
    lua_pushinteger(L, s->bU2DevExitLat); lua_setfield(L, -2, "u2_device_exit_latency");
    }

static void pushcontaineriddescriptor(lua_State *L, const struct libusb_container_id_descriptor *s)
    {
    int i;
    lua_newtable(L);
    pushbostype(L, s->bDevCapabilityType); lua_setfield(L, -2, "type");
    lua_newtable(L);
    for(i=0; i<16; i++)
        { lua_pushinteger(L, s->ContainerID[i]); lua_rawseti(L, -2, i+1); }
    lua_setfield(L, -2, "container_id");
    }

static int pushbosdevcapabilitydescriptordecoded(lua_State *L, struct libusb_bos_dev_capability_descriptor *s, context_t *context)
    {
    int ec;
    struct libusb_usb_2_0_extension_descriptor *usb_2_0_extension;
    struct libusb_ss_usb_device_capability_descriptor *ss_usb_device_cap;
    struct libusb_container_id_descriptor *container_id;
    switch(s->bDevCapabilityType)
        {
        case LIBUSB_BT_USB_2_0_EXTENSION:
            ec = libusb_get_usb_2_0_extension_descriptor(context, s, &usb_2_0_extension);
            CheckError(L, ec);
            pushusb_2_0_extension_descriptor(L, usb_2_0_extension);
            libusb_free_usb_2_0_extension_descriptor(usb_2_0_extension);
            break;
        case LIBUSB_BT_SS_USB_DEVICE_CAPABILITY:
            ec = libusb_get_ss_usb_device_capability_descriptor(context, s, &ss_usb_device_cap);
            CheckError(L, ec);
            pushssusbdevicecapabilitydescriptor(L, ss_usb_device_cap);
            libusb_free_ss_usb_device_capability_descriptor(ss_usb_device_cap);
            break;
        case LIBUSB_BT_CONTAINER_ID:
            ec = libusb_get_container_id_descriptor(context, s, &container_id);
            CheckError(L, ec);
            pushcontaineriddescriptor(L, container_id);
            libusb_free_container_id_descriptor(container_id);
            break;
        //case LIBUSB_BT_WIRELESS_USB_DEVICE_CAPABILITY: 
        default:
            lua_pushnil(L);
            break;
        }
    return 1;
    }
#endif

static int pushbosdevcapabilitydescriptor(lua_State *L, struct libusb_bos_dev_capability_descriptor *s, context_t *context)
    {
    size_t len = s->bLength - 3;
    lua_newtable(L);
    lua_pushinteger(L, s->bDevCapabilityType);
    lua_setfield(L, -2, "type");
    if(len>0)
        {
        lua_pushlstring(L, (char*)&s->dev_capability_data, len);
        lua_setfield(L, -2, "data");
        (void)context;
#if 0
        /* add decoding for known types @@NO: add a function to decode them, see USB 3.2 Spec 9.6 */
        pushbosdevcapabilitydescriptordecoded(L, s, context);
        lua_setfield(L, -2, "decoded");
#endif
        }
    return 0;
    }

void pushbosdescriptor(lua_State *L, struct libusb_bos_descriptor *s, context_t *context)
    {
    uint8_t i;
    lua_newtable(L);
    lua_pushinteger(L, s->bNumDeviceCaps);
    lua_setfield(L, -2, "num_capabilities");
    lua_newtable(L);
    for(i=0; i<s->bNumDeviceCaps; i++)
        {
        pushbosdevcapabilitydescriptor(L, s->dev_capability[i], context);
        lua_rawseti(L, -2, i+1);
        }
    lua_setfield(L, -2, "capability");
    }
