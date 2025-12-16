--
-- (C) 2013 Kriss@XIXs.com
--
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

local alc={}

local core=require("alc.core")

local base={}
local meta={}
meta.__index=base

setmetatable(alc,meta)

-- copypasta from ALC header
local import=[[
/* Enumerant values begin at column 50. No tabs. */

/* Boolean False. */
#define ALC_FALSE                                0

/* Boolean True. */
#define ALC_TRUE                                 1

/**
 * followed by <int> Hz
 */
#define ALC_FREQUENCY                            0x1007

/**
 * followed by <int> Hz
 */
#define ALC_REFRESH                              0x1008

/**
 * followed by AL_TRUE, AL_FALSE
 */
#define ALC_SYNC                                 0x1009

/**
 * followed by <int> Num of requested Mono (3D) Sources
 */
#define ALC_MONO_SOURCES                         0x1010

/**
 * followed by <int> Num of requested Stereo Sources
 */
#define ALC_STEREO_SOURCES                       0x1011

/**
 * errors
 */

/**
 * No error
 */
#define ALC_NO_ERROR                             ALC_FALSE

/**
 * No device
 */
#define ALC_INVALID_DEVICE                       0xA001

/**
 * invalid context ID
 */
#define ALC_INVALID_CONTEXT                      0xA002

/**
 * bad enum
 */
#define ALC_INVALID_ENUM                         0xA003

/**
 * bad value
 */
#define ALC_INVALID_VALUE                        0xA004

/**
 * Out of memory.
 */
#define ALC_OUT_OF_MEMORY                        0xA005


/**
 * The Specifier string for default device
 */
#define ALC_DEFAULT_DEVICE_SPECIFIER             0x1004
#define ALC_DEVICE_SPECIFIER                     0x1005
#define ALC_EXTENSIONS                           0x1006

#define ALC_MAJOR_VERSION                        0x1000
#define ALC_MINOR_VERSION                        0x1001

#define ALC_ATTRIBUTES_SIZE                      0x1002
#define ALC_ALL_ATTRIBUTES                       0x1003


/**
 * Capture extension
 */
#define ALC_EXT_CAPTURE 1
#define ALC_CAPTURE_DEVICE_SPECIFIER             0x310
#define ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER     0x311
#define ALC_CAPTURE_SAMPLES                      0x312


/**
 * ALC_ENUMERATE_ALL_EXT enums
 */
#define ALC_ENUMERATE_ALL_EXT 1
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#define ALC_ALL_DEVICES_SPECIFIER                0x1013

]]
-- parse the above string for constants, makes updates as easy as a cutnpaste from original source code

alc.defs={}

for l in import:gmatch("([^\n]*)") do
	local define,value
	local state="start"
	for w in l:gmatch("([^%s]+)") do
		if state=="start" then
			if w=="#define" then
				state="define"
			else
				break
			end
		elseif state=="define" then
			define=w
			state="value"
		elseif state=="value" then
			value=w
				if define:sub(1,4)=="ALC_" then -- sanity check
					define=define:sub(5)
					
					if value:sub(1,4)=="ALC_" then -- allow lookback
						value=alc.defs[value:sub(5)]
					end
					
					alc.defs[define]=tonumber(value)
				end
			break
		else
			break
		end
	end
end
import=nil -- free it just because

for i,v in pairs(alc.defs) do -- copy vals into base for shorthand al.FALSE use
	base[i]=v
	base[v]=i
end

for n,v in pairs(core) do -- expot all core functions
	alc[n]=v
end

--TODO: include device and context options
function alc.setup(opts)
	opts=opts or {}
	local dc={}
	dc.alc=alc
	dc.opts=opts
--print("AL","OpenDevice")
	dc.device=alc.OpenDevice()
--print("AL","CreateContext")
	dc.context=alc.CreateContext(dc.device)
	dc.clean=alc.clean
--print("AL","MakeContextCurrent")
	alc.MakeContextCurrent(dc.context)
	
	if opts.capture then
--print("AL","CaptureOpenDevice")
--		dc.capture_device=alc.CaptureOpenDevice(nil,44100,alc.CAPTURE_SAMPLES,44100)
		dc.capture_device=alc.CaptureOpenDevice(nil,48000,al.FORMAT_MONO16,32768)
	end
	
--print("AL","setup done","freq",	alc.Get(dc.device,alc.FREQUENCY),
--			dc.capture_device and alc.Get(dc.capture_device,alc.FREQUENCY) )
	
	return dc
end

function alc.clean(dc)
-- cleanup must happen in this order
	alc.MakeContextCurrent()
	if dc.context then dc.context=alc.DestroyContext(dc.context) end
	if dc.device then dc.device=alc.CloseDevice(dc.device) end
	
	if dc.capture_device then
		alc.CaptureCloseDevice(dc.capture_device)
	end
	
	return dc
end

return alc
