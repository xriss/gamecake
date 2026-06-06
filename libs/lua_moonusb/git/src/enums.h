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

#ifndef enumsDEFINED
#define enumsDEFINED

/* enums.c */
#define enums_free_all moonusb_enums_free_all
void enums_free_all(lua_State *L);
#define enums_test moonusb_enums_test
int enums_test(lua_State *L, int domain, int arg, int *err);
#define enums_opt moonusb_enums_opt
int enums_opt(lua_State *L, int domain, int arg, int defval);
#define enums_check moonusb_enums_check
int enums_check(lua_State *L, int domain, int arg);
#define enums_push moonusb_enums_push
int enums_push(lua_State *L, int domain, int code);
#define enums_values moonusb_enums_values
int enums_values(lua_State *L, int domain);
#define enums_checklist moonusb_enums_checklist
int* enums_checklist(lua_State *L, int domain, int arg, int *count, int *err);
#define enums_freelist moonusb_enums_freelist
void enums_freelist(lua_State *L, int *list);


/* Enum domains */
#define DOMAIN_TYPE                     0
#define DOMAIN_CLASS                    1
#define DOMAIN_SPEED                    2
#define DOMAIN_OPTION                   3
#define DOMAIN_LOG_LEVEL                5
#define DOMAIN_DIRECTION                6
#define DOMAIN_TRANSFER_TYPE            7
#define DOMAIN_STANDARD_REQUEST         8
#define DOMAIN_REQUEST_TYPE             9
#define DOMAIN_REQUEST_RECIPIENT        10
#define DOMAIN_ISO_SYNC_TYPE            11
#define DOMAIN_ISO_USAGE_TYPE           12
#define DOMAIN_CAPABILITY               13
#define DOMAIN_HOTPLUG_EVENT            14
#define DOMAIN_TRANSFER_STATUS          15
#define DOMAIN_BOS_TYPE                 16

/* Types for usb.sizeof() & friends */
#define MOONUSB_TYPE_CHAR         1
#define MOONUSB_TYPE_UCHAR        2
#define MOONUSB_TYPE_SHORT        3
#define MOONUSB_TYPE_USHORT       4
#define MOONUSB_TYPE_INT          5
#define MOONUSB_TYPE_UINT         6
#define MOONUSB_TYPE_LONG         7
#define MOONUSB_TYPE_ULONG        8
#define MOONUSB_TYPE_FLOAT        9 
#define MOONUSB_TYPE_DOUBLE       10

/* USB class codes, used instead of libusb_class_code.
 * (see https://www.usb.org/defined-class-codes). 
 */
#define CLASS_PER_INTERFACE             0x00    /* device */
#define CLASS_AUDIO                     0x01    /* interface */
#define CLASS_CDC                       0x02    /* device, interface */
#define CLASS_HID                       0x03    /* interface */
#define CLASS_PHYSICAL                  0x05    /* interface */
#define CLASS_IMAGE                     0x06    /* interface */
#define CLASS_PRINTER                   0x07    /* interface */
#define CLASS_MASS_STORAGE              0x08    /* interface */
#define CLASS_HUB                       0x09    /* device */
#define CLASS_CDC_DATA                  0x0a    /* interface */
#define CLASS_SMART_CARD                0x0b    /* interface */
#define CLASS_CONTENT_SECURITY          0x0d    /* interface */
#define CLASS_VIDEO                     0x0e    /* interface */
#define CLASS_PERSONAL_HEALTHCARE       0x0f    /* interface */
#define CLASS_AUDIO_VIDEO               0x10    /* interface */
#define CLASS_BILLBOARD                 0x11    /* device */
#define CLASS_TYPE_C_BRIDGE             0x12    /* interface */
#define CLASS_DIAGNOSTIC                0xdc    /* device, interface */
#define CLASS_WIRELESS                  0xe0    /* interface */
#define CLASS_MISCELLANEOUS             0xef    /* device, interface */
#define CLASS_APPLICATION_SPECIFIC      0xfe    /* interface */
#define CLASS_VENDOR_SPECIFIC           0xff    /* device, interface */

/* Device Capability Type Codes (rfr. USB 3.2 Specification, table 9.14) */
#define BOS_TYPE_WIRELESS_USB               0x01
#define BOS_TYPE_USB_2_0_EXTENSION          0x02
#define BOS_TYPE_SUPERSPEED_USB             0x03
#define BOS_TYPE_CONTAINER_ID               0x04
#define BOS_TYPE_PLATFORM                   0x05
#define BOS_TYPE_POWER_DELIVERY             0x06
#define BOS_TYPE_BATTERY_INFO               0x07
#define BOS_TYPE_PD_CONSUMER_PORT           0x08
#define BOS_TYPE_PD_PROVIDER_PORT           0x09
#define BOS_TYPE_SUPERSPEED_PLUS            0x0a
#define BOS_TYPE_PRECISION_TIME_MEASUREMENT 0x0b
#define BOS_TYPE_WIRELESS_USB_EXT           0x0c
#define BOS_TYPE_BILLBOARD                  0x0d
#define BOS_TYPE_AUTHENTICATION             0x0e
#define BOS_TYPE_BILLBOARD_EX               0x0f
#define BOS_TYPE_CONFIGURATION_SUMMARY      0x10

#define testtype(L, arg, err) enums_test((L), DOMAIN_TYPE, (arg), (err))
#define opttype(L, arg, defval) enums_opt((L), DOMAIN_TYPE, (arg), (defval))
#define checktype(L, arg) enums_check((L), DOMAIN_TYPE, (arg))
#define pushtype(L, val) enums_push((L), DOMAIN_TYPE, (int)(val))
#define valuestype(L) enums_values((L), DOMAIN_TYPE)

#define testclass(L, arg, err) enums_test((L), DOMAIN_CLASS, (arg), (err))
#define optclass(L, arg, defval) enums_opt((L), DOMAIN_CLASS, (arg), (defval))
#define checkclass(L, arg) enums_check((L), DOMAIN_CLASS, (arg))
#define pushclass(L, val) enums_push((L), DOMAIN_CLASS, (int)(val))
#define valuesclass(L) enums_values((L), DOMAIN_CLASS)

#define testspeed(L, arg, err) enums_test((L), DOMAIN_SPEED, (arg), (err))
#define optspeed(L, arg, defval) enums_opt((L), DOMAIN_SPEED, (arg), (defval))
#define checkspeed(L, arg) enums_check((L), DOMAIN_SPEED, (arg))
#define pushspeed(L, val) enums_push((L), DOMAIN_SPEED, (int)(val))
#define valuesspeed(L) enums_values((L), DOMAIN_SPEED)

#define testoption(L, arg, err) enums_test((L), DOMAIN_OPTION, (arg), (err))
#define optoption(L, arg, defval) enums_opt((L), DOMAIN_OPTION, (arg), (defval))
#define checkoption(L, arg) enums_check((L), DOMAIN_OPTION, (arg))
#define pushoption(L, val) enums_push((L), DOMAIN_OPTION, (int)(val))
#define valuesoption(L) enums_values((L), DOMAIN_OPTION)

#define testloglevel(L, arg, err) enums_test((L), DOMAIN_LOG_LEVEL, (arg), (err))
#define optloglevel(L, arg, defval) enums_opt((L), DOMAIN_LOG_LEVEL, (arg), (defval))
#define checkloglevel(L, arg) enums_check((L), DOMAIN_LOG_LEVEL, (arg))
#define pushloglevel(L, val) enums_push((L), DOMAIN_LOG_LEVEL, (int)(val))
#define valuesloglevel(L) enums_values((L), DOMAIN_LOG_LEVEL)

#define testdirection(L, arg, err) enums_test((L), DOMAIN_DIRECTION, (arg), (err))
#define optdirection(L, arg, defval) enums_opt((L), DOMAIN_DIRECTION, (arg), (defval))
#define checkdirection(L, arg) enums_check((L), DOMAIN_DIRECTION, (arg))
#define pushdirection(L, val) enums_push((L), DOMAIN_DIRECTION, (int)(val))
#define valuesdirection(L) enums_values((L), DOMAIN_DIRECTION)

#define testtransfertype(L, arg, err) enums_test((L), DOMAIN_TRANSFER_TYPE, (arg), (err))
#define opttransfertype(L, arg, defval) enums_opt((L), DOMAIN_TRANSFER_TYPE, (arg), (defval))
#define checktransfertype(L, arg) enums_check((L), DOMAIN_TRANSFER_TYPE, (arg))
#define pushtransfertype(L, val) enums_push((L), DOMAIN_TRANSFER_TYPE, (int)(val))
#define valuestransfertype(L) enums_values((L), DOMAIN_TRANSFER_TYPE)

#define teststandardrequest(L, arg, err) enums_test((L), DOMAIN_STANDARD_REQUEST, (arg), (err))
#define optstandardrequest(L, arg, defval) enums_opt((L), DOMAIN_STANDARD_REQUEST, (arg), (defval))
#define checkstandardrequest(L, arg) enums_check((L), DOMAIN_STANDARD_REQUEST, (arg))
#define pushstandardrequest(L, val) enums_push((L), DOMAIN_STANDARD_REQUEST, (int)(val))
#define valuesstandardrequest(L) enums_values((L), DOMAIN_STANDARD_REQUEST)

#define testrequesttype(L, arg, err) enums_test((L), DOMAIN_REQUEST_TYPE, (arg), (err))
#define optrequesttype(L, arg, defval) enums_opt((L), DOMAIN_REQUEST_TYPE, (arg), (defval))
#define checkrequesttype(L, arg) enums_check((L), DOMAIN_REQUEST_TYPE, (arg))
#define pushrequesttype(L, val) enums_push((L), DOMAIN_REQUEST_TYPE, (int)(val))
#define valuesrequesttype(L) enums_values((L), DOMAIN_REQUEST_TYPE)

#define testrequestrecipient(L, arg, err) enums_test((L), DOMAIN_REQUEST_RECIPIENT, (arg), (err))
#define optrequestrecipient(L, arg, defval) enums_opt((L), DOMAIN_REQUEST_RECIPIENT, (arg), (defval))
#define checkrequestrecipient(L, arg) enums_check((L), DOMAIN_REQUEST_RECIPIENT, (arg))
#define pushrequestrecipient(L, val) enums_push((L), DOMAIN_REQUEST_RECIPIENT, (int)(val))
#define valuesrequestrecipient(L) enums_values((L), DOMAIN_REQUEST_RECIPIENT)

#define testisosynctype(L, arg, err) enums_test((L), DOMAIN_ISO_SYNC_TYPE, (arg), (err))
#define optisosynctype(L, arg, defval) enums_opt((L), DOMAIN_ISO_SYNC_TYPE, (arg), (defval))
#define checkisosynctype(L, arg) enums_check((L), DOMAIN_ISO_SYNC_TYPE, (arg))
#define pushisosynctype(L, val) enums_push((L), DOMAIN_ISO_SYNC_TYPE, (int)(val))
#define valuesisosynctype(L) enums_values((L), DOMAIN_ISO_SYNC_TYPE)

#define testisousagetype(L, arg, err) enums_test((L), DOMAIN_ISO_USAGE_TYPE, (arg), (err))
#define optisousagetype(L, arg, defval) enums_opt((L), DOMAIN_ISO_USAGE_TYPE, (arg), (defval))
#define checkisousagetype(L, arg) enums_check((L), DOMAIN_ISO_USAGE_TYPE, (arg))
#define pushisousagetype(L, val) enums_push((L), DOMAIN_ISO_USAGE_TYPE, (int)(val))
#define valuesisousagetype(L) enums_values((L), DOMAIN_ISO_USAGE_TYPE)

#define testcapability(L, arg, err) enums_test((L), DOMAIN_CAPABILITY, (arg), (err))
#define optcapability(L, arg, defval) enums_opt((L), DOMAIN_CAPABILITY, (arg), (defval))
#define checkcapability(L, arg) enums_check((L), DOMAIN_CAPABILITY, (arg))
#define pushcapability(L, val) enums_push((L), DOMAIN_CAPABILITY, (int)(val))
#define valuescapability(L) enums_values((L), DOMAIN_CAPABILITY)

#define testhotplugevent(L, arg, err) enums_test((L), DOMAIN_HOTPLUG_EVENT, (arg), (err))
#define opthotplugevent(L, arg, defval) enums_opt((L), DOMAIN_HOTPLUG_EVENT, (arg), (defval))
#define checkhotplugevent(L, arg) enums_check((L), DOMAIN_HOTPLUG_EVENT, (arg))
#define pushhotplugevent(L, val) enums_push((L), DOMAIN_HOTPLUG_EVENT, (int)(val))
#define valueshotplugevent(L) enums_values((L), DOMAIN_HOTPLUG_EVENT)

#define testtransferstatus(L, arg, err) enums_test((L), DOMAIN_TRANSFER_STATUS, (arg), (err))
#define opttransferstatus(L, arg, defval) enums_opt((L), DOMAIN_TRANSFER_STATUS, (arg), (defval))
#define checktransferstatus(L, arg) enums_check((L), DOMAIN_TRANSFER_STATUS, (arg))
#define pushtransferstatus(L, val) enums_push((L), DOMAIN_TRANSFER_STATUS, (int)(val))
#define valuestransferstatus(L) enums_values((L), DOMAIN_TRANSFER_STATUS)

#define testbostype(L, arg, err) enums_test((L), DOMAIN_BOS_TYPE, (arg), (err))
#define optbostype(L, arg, defval) enums_opt((L), DOMAIN_BOS_TYPE, (arg), (defval))
#define checkbostype(L, arg) enums_check((L), DOMAIN_BOS_TYPE, (arg))
#define pushbostype(L, val) enums_push((L), DOMAIN_BOS_TYPE, (int)(val))
#define valuesbostype(L) enums_values((L), DOMAIN_BOS_TYPE)

#if 0 /* scaffolding 8yy */
#define testxxx(L, arg, err) enums_test((L), DOMAIN_XXX, (arg), (err))
#define optxxx(L, arg, defval) enums_opt((L), DOMAIN_XXX, (arg), (defval))
#define checkxxx(L, arg) enums_check((L), DOMAIN_XXX, (arg))
#define pushxxx(L, val) enums_push((L), DOMAIN_XXX, (int)(val))
#define valuesxxx(L) enums_values((L), DOMAIN_XXX)
    CASE(xxx);
#define DOMAIN_XXX 

#endif

#endif /* enumsDEFINED */

