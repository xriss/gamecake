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

static int freetransfer(lua_State *L, ud_t *ud)
    {
    transfer_t *transfer = (transfer_t*)ud->handle;
    int submitted = IsSubmitted(ud);
//  freechildren(L, _MT, ud);
    if(!freeuserdata(L, ud, "transfer")) return 0;
    if(submitted) 
        (void)libusb_cancel_transfer(transfer);
    libusb_free_transfer(transfer);
    return 0;
    }

static ud_t *newtransfer(lua_State *L, int iso_packets, devhandle_t *devhandle)
    {
    ud_t *ud;
    transfer_t *transfer = libusb_alloc_transfer(iso_packets);
    if(!transfer) { luaL_error(L, "libusb_alloc_transfer() failed"); return NULL; }
    ud = newuserdata(L, transfer, TRANSFER_MT, "transfer");
    ud->parent_ud = userdata(devhandle);
    ud->context = userdata(devhandle)->context;
    ud->destructor = freetransfer;
    return ud;
    }

static int Get_status(lua_State *L)
    {
    transfer_t *transfer = checktransfer(L, 1, NULL);
    pushtransferstatus(L, transfer->status);
    return 1;
    }

static int Get_endpoint(lua_State *L)
    {
    transfer_t *transfer = checktransfer(L, 1, NULL);
    lua_pushinteger(L, transfer->endpoint);
    return 1;
    }

static int Submit(lua_State *L, transfer_t *transfer, ud_t *ud, int new_transfer)
    {
    int ec = libusb_submit_transfer(transfer);
    switch(ec)
        {
        case LIBUSB_SUCCESS:        break;
        case LIBUSB_ERROR_NO_DEVICE:
        case LIBUSB_ERROR_BUSY:     break; /* this will be learned in the callback */
        default: /* destroy the transfer object */
            if(new_transfer) lua_pop(L, 1);
            ud->destructor(L, ud);
            CheckError(L, ec);
            return 0;
        }
    MarkSubmitted(ud);
    return 1;
    }

static int Cancel(lua_State *L)
    {
    transfer_t *transfer = checktransfer(L, 1, NULL);
    /* This will cause the callback to be executed, and the transfer
     * to be deleted at the end of it */
    (void)libusb_cancel_transfer(transfer);
    return 0;
    }

static void Callback(transfer_t *transfer)
    {
#define L moonusb_L
    int rc, resubmit;
    int top = lua_gettop(L);
    ud_t *ud = userdata(transfer);
    if(!ud) { unexpected(L); return; }
    CancelSubmitted(ud);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref1);
    pushtransfer(L, transfer);
    pushtransferstatus(L, transfer->status);
    rc = lua_pcall(L, 2, 1, 0);
    if(rc!=LUA_OK)
        { lua_error(L); return; }
    resubmit = lua_toboolean(L, -1);
    lua_settop(L, top);
    if(resubmit)
        Submit(L, transfer, ud, 0);
    else
        ud->destructor(L, ud);
    return;
#undef L
    }

static int Submit_control_transfer(lua_State *L)
// transfer = f(devhandle, ptr, timeout, cb)
// expects the 8-bytes setup in ptr[0]...ptr[7]
    {
    ud_t *ud;
    transfer_t *transfer;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 2);
    int length = luaL_checkinteger(L, 3);
    unsigned int timeout = luaL_checkinteger(L, 4);
    struct libusb_control_setup *s = (struct libusb_control_setup*)ptr;
    if(length < (s->wLength + 8)) return argerror(L, 3, ERR_VALUE);
    if(!lua_isfunction(L, 5)) return argerror(L, 5, ERR_FUNCTION);
    ud = newtransfer(L, 0, devhandle);
    transfer = (transfer_t*)ud->handle;
    Reference(L, 5, ud->ref1);
    libusb_fill_control_transfer(transfer, devhandle, ptr, Callback, NULL, timeout);
    return Submit(L, transfer, ud, 1);
    }

static int Submit_bulk_transfer(lua_State *L)
    {
    ud_t *ud;
    transfer_t *transfer;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char endpoint = luaL_checkinteger(L, 2);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 3);
    int length = luaL_checkinteger(L, 4);
    unsigned int timeout = luaL_checkinteger(L, 5);
    if(!lua_isfunction(L, 6)) return argerror(L, 6, ERR_FUNCTION);
    ud = newtransfer(L, 0, devhandle);
    transfer = (transfer_t*)ud->handle;
    Reference(L, 6, ud->ref1);
    libusb_fill_bulk_transfer(transfer, devhandle, endpoint, ptr, length,
            Callback, NULL, timeout);
    return Submit(L, transfer, ud, 1);
    }

static int Submit_bulk_stream_transfer(lua_State *L)
    {
    ud_t *ud;
    transfer_t *transfer;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char endpoint = luaL_checkinteger(L, 2);
    uint32_t stream_id = luaL_checkinteger(L, 3);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 4);
    int length = luaL_checkinteger(L, 5);
    unsigned int timeout = luaL_checkinteger(L, 6);
    if(!lua_isfunction(L, 7)) return argerror(L, 7, ERR_FUNCTION);
    ud = newtransfer(L, 0, devhandle);
    transfer = (transfer_t*)ud->handle;
    Reference(L, 7, ud->ref1);
    libusb_fill_bulk_stream_transfer(transfer, devhandle, endpoint, stream_id, ptr, length,
            Callback, NULL, timeout);
    return Submit(L, transfer, ud, 1);
    }

static int Submit_interrupt_transfer(lua_State *L)
    {
    ud_t *ud;
    transfer_t *transfer;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char endpoint = luaL_checkinteger(L, 2);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 3);
    int length = luaL_checkinteger(L, 4);
    unsigned int timeout = luaL_checkinteger(L, 5);
    if(!lua_isfunction(L, 6)) return argerror(L, 6, ERR_FUNCTION);
    ud = newtransfer(L, 0, devhandle);
    transfer = (transfer_t*)ud->handle;
    Reference(L, 6, ud->ref1);
    libusb_fill_interrupt_transfer(transfer, devhandle, endpoint, ptr, length,
            Callback, NULL, timeout);
    return Submit(L, transfer, ud, 1);
    }

static int Submit_iso_transfer(lua_State *L)
    {
    ud_t *ud;
    transfer_t *transfer;
    devhandle_t *devhandle = checkdevhandle(L, 1, NULL);
    unsigned char endpoint = luaL_checkinteger(L, 2);
    unsigned char *ptr = (unsigned char*)checklightuserdata(L, 3);
    int length = luaL_checkinteger(L, 4);
    int num_iso_packets = luaL_checkinteger(L, 5);
    unsigned int iso_packet_length = luaL_checkinteger(L, 6); //@@ alt: a table of lengths?
    unsigned int timeout = luaL_checkinteger(L, 7);
    if(!lua_isfunction(L, 8)) return argerror(L, 8, ERR_FUNCTION);
    if(length != (int)(num_iso_packets*iso_packet_length)) return argerror(L, 3, ERR_VALUE);
    ud = newtransfer(L, num_iso_packets, devhandle);
    transfer = (transfer_t*)ud->handle;
    Reference(L, 8, ud->ref1);
    libusb_set_iso_packet_lengths(transfer, iso_packet_length);
    libusb_fill_iso_transfer(transfer, devhandle, endpoint, ptr, length,
           num_iso_packets, Callback, NULL, timeout);
    return Submit(L, transfer, ud, 1);
    }

/*------ Utilities to be used in callbacks-------------------------------------*/

static int Encode_control_setup_string(lua_State *L)
    {
    struct libusb_control_setup dst;
    checkcontrolsetup(L, 2, &dst);
    lua_pushlstring(L, (char*)&dst, sizeof(struct libusb_control_setup));
    return 1;
    }

static int Encode_control_setup(lua_State *L)
    {
    const char *ptr;
    struct libusb_control_setup *dst;
    if(lua_isnil(L, 1))
        return Encode_control_setup_string(L);
    ptr = checklightuserdata(L, 1);
    dst = (struct libusb_control_setup*)ptr;
    checkcontrolsetup(L, 2, dst);
    return 0;
    }

static int Decode_control_setup_string(lua_State *L)
    {
    size_t len;
    struct libusb_control_setup s;
    const char *ptr = luaL_checklstring(L, 1, &len);
    if(len < sizeof(struct libusb_control_setup))
        return argerror(L, 1, ERR_LENGTH);
    memcpy(&s, ptr, sizeof(struct libusb_control_setup));
    pushcontrolsetup(L, &s);
    return 1;
    }

static int Decode_control_setup(lua_State *L)
    {
    const char *ptr;
    struct libusb_control_setup *s;
    if(lua_isstring(L, 1))
        return Decode_control_setup_string(L);
    ptr = checklightuserdata(L, 1);
    s = (struct libusb_control_setup*)ptr;
    pushcontrolsetup(L, s);
    return 1;
    }

static int Get_iso_packet_descriptors(lua_State *L)
    {
    int i, offset=0;
    struct libusb_iso_packet_descriptor *s;
    transfer_t *transfer = checktransfer(L, 1, NULL);
    if(transfer->type != LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
        return luaL_argerror(L, 1, "not an isochronous transfer");
    if(transfer->status != LIBUSB_TRANSFER_COMPLETED) return 0;
    lua_newtable(L);
    for(i = 0; i<transfer->num_iso_packets; i++)
        {
        s = &transfer->iso_packet_desc[i];
        lua_newtable(L);
        pushtransferstatus(L, s->status); lua_setfield(L, -2, "status");
        lua_pushinteger(L, s->actual_length); lua_setfield(L, -2, "length");
        lua_pushinteger(L, offset); lua_setfield(L, -2, "offset");
        lua_rawseti(L, -2, i+1);
        offset = offset + s->length;
        }
    return 1;
    }

static int Get_actual_length(lua_State *L)
    {
    transfer_t *transfer = checktransfer(L, 1, NULL);
    lua_pushinteger(L, transfer->actual_length);
    return 1;
    }

static int Get_stream_id(lua_State *L)
    {
    transfer_t *transfer = checktransfer(L, 1, NULL);
    lua_pushinteger(L, libusb_transfer_get_stream_id(transfer));
    return 1;
    }

DESTROY_FUNC(transfer)

static const struct luaL_Reg Methods[] = 
    {
        { "cancel", Cancel },
        { "get_status", Get_status },
        { "get_endpoint", Get_endpoint },
        { "get_iso_packet_descriptors", Get_iso_packet_descriptors },
        { "get_actual_length", Get_actual_length },
        { "get_stream_id", Get_stream_id },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg DevhandleMethods[] = 
    {
        { "submit_control_transfer", Submit_control_transfer },
        { "submit_bulk_transfer", Submit_bulk_transfer },
        { "submit_bulk_stream_transfer", Submit_bulk_stream_transfer },
        { "submit_interrupt_transfer", Submit_interrupt_transfer },
        { "submit_iso_transfer", Submit_iso_transfer },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "encode_control_setup", Encode_control_setup },
        { "decode_control_setup", Decode_control_setup },
        { NULL, NULL } /* sentinel */
    };

void moonusb_open_transfer(lua_State *L)
    {
    udata_addmethods(L, DEVHANDLE_MT, DevhandleMethods);
    udata_define(L, TRANSFER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

#if 0
//unsigned char *libusb_control_transfer_get_data(struct libusb_transfer *transfer);
//struct libusb_control_setup *libusb_control_transfer_get_setup(struct libusb_transfer *transfer);
//void libusb_transfer_set_stream_id(struct libusb_transfer *transfer, uint32_t stream_id);
//unsigned char *libusb_get_iso_packet_buffer(struct libusb_transfer *transfer, unsigned int packet);
//unsigned char *libusb_get_iso_packet_buffer_simple(struct libusb_transfer *transfer, unsigned int packet);
#endif
