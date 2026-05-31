
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"

#include <AL/al.h>
#include <AL/alc.h>

//
// we can use either these strings as a string identifier
// or the address as a light userdata identifier, both unique
//
const char *lua_alc_device_meta_name="alc*device*ptr";
const char *lua_alc_capture_device_meta_name="alc*capture_device*ptr";
const char *lua_alc_context_meta_name="alc*context*ptr";


/*

TODO:

alcGetEnumValue
alcGetString
alcGetIntegerv



These two seem to not actualy do anything...

alcSuspendContext
alcProcessContext


These to two ae not really necesary and it would break cleanup code to allow multiple pointers?

alcGetCurrentContext
alcGetContextsDevice


These are a bit too low level to be of use?

alcIsExtensionPresent
alcGetProcAddress


*/


// hax to be honest, all this to create a lua_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

#include "wet_types.h"

extern u8 * lua_toluserdata (lua_State *L, int idx, size_t *len);



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_OpenDevice(lua_State *l)
{
	ALCdevice **device;

// create a device userdata pointer pointer
	device = (ALCdevice**)lua_newuserdata(l, sizeof(ALCdevice**));	
	(*device)=0;
	luaL_getmetatable(l, lua_alc_device_meta_name);
	lua_setmetatable(l, -2);

//open the actual device
	(*device)=alcOpenDevice(NULL);
	if(!(*device)) { return 0; }
	

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **device and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice ** lua_alc_get_device_ptr (lua_State *l,int idx)
{
ALCdevice **device;
	device = (ALCdevice**)luaL_checkudata(l, idx , lua_alc_device_meta_name);
	return device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *device and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice * lua_alc_check_device (lua_State *l,int idx)
{	
ALCdevice **device;
	device = lua_alc_get_device_ptr (l, idx);
	if(!*device)
	{
		luaL_error(l,"alc device is null");
	}
	return *device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for device ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CloseDevice (lua_State *l)
{	
ALCdevice **device;

	device = lua_alc_get_device_ptr(l, 1 );
	
	if(*device)
	{
		alcCloseDevice(*device);
		(*device)=0;
	}
	
	return 0;
}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open capture device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CaptureOpenDevice(lua_State *l)
{
	ALCdevice **device;

// inputs, skip device name for now
int freq=lua_tonumber(l,2);		// 44100
int format=lua_tonumber(l,3);	// AL_FORMAT_MONO16
int length=lua_tonumber(l,4);	// 44100

// create a device userdata pointer pointer
	device = (ALCdevice**)lua_newuserdata(l, sizeof(ALCdevice**));	
	(*device)=0;
	luaL_getmetatable(l, lua_alc_capture_device_meta_name);
	lua_setmetatable(l, -2);

//open the actual device
	(*device)=alcCaptureOpenDevice(0,freq,format,length);
	if(!(*device)) { return 0; }
	
//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **capture_device and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice ** lua_alc_get_capture_device_ptr (lua_State *l,int idx)
{
ALCdevice **device;
	device = (ALCdevice**)luaL_checkudata(l, idx , lua_alc_capture_device_meta_name);
	return device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *capture_device and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice * lua_alc_check_capture_device (lua_State *l,int idx)
{	
ALCdevice **device;
	device = lua_alc_get_capture_device_ptr (l, idx);
	if(!*device)
	{
		luaL_error(l,"alc capture device is null");
	}
	return *device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get (capture or output) *device and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice * lua_alc_check_any_device (lua_State *l,int idx)
{	
ALCdevice **device;
	device = (ALCdevice**)lua_touserdata(l, idx);
	if(!device)
	{
		luaL_error(l,"alc any device not a userdata");
	}
	if(!*device)
	{
		luaL_error(l,"alc any device is null");
	}
	return *device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for device ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CaptureCloseDevice (lua_State *l)
{	
ALCdevice **device;

	device = lua_alc_get_capture_device_ptr(l, 1 );
	
	if(*device)
	{
		alcCaptureCloseDevice(*device);
		(*device)=0;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// start capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CaptureStart (lua_State *l)
{	
ALCdevice *device;

	device = lua_alc_check_capture_device(l, 1 );
	
	alcCaptureStart(device);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stop capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CaptureStop (lua_State *l)
{	
ALCdevice *device;

	device = lua_alc_check_capture_device(l, 1 );
	
	alcCaptureStop(device);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// capture some data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CaptureSamples (lua_State *l)
{	
ALCdevice *device;

const u8 *ptr=0;
size_t len=0;
size_t lenmax=0;

const u8 *tmp=0;

	device = lua_alc_check_capture_device(l, 1 );
	
	if(lua_isuserdata(l,2)) // must check for light first...
	{
		ptr=lua_toluserdata(l,2,&lenmax);
		len=lua_tonumber(l,3);
		if(len>lenmax) { len=lenmax; }
	}

	if( (ptr==0) || (len==0) )
	{
		len=lua_tonumber(l,3);
		tmp=malloc(len*2);
		ptr=tmp;
	}

	if(ptr)
	{
		alcCaptureSamples(device,(void*)ptr,len);
	}

	if(tmp) // return data string
	{
		lua_pushlstring(l,(char*)tmp,len*2);
		free(tmp);
	}
	else // return userdata passed in
	{
		lua_pushvalue(l,2);
	}
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get error number
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_GetError (lua_State *l)
{
	ALCdevice *device=0;

	if( ! lua_isnoneornil(l,1) )
	{
		device = lua_alc_check_any_device(l, 1 );
	}

	lua_pushnumber(l,alcGetError(device));
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CreateContext(lua_State *l)
{
	ALCdevice *device;
	ALCcontext **context;
	ALint attr[] = { ALC_FREQUENCY, 48000, 0 };

// must pass in a device
	device = lua_alc_check_device(l, 1);

// create a context userdata pointer pointer
	context = (ALCcontext**)lua_newuserdata(l, sizeof(ALCcontext**));	
	(*context)=0;
	luaL_getmetatable(l, lua_alc_context_meta_name);
	lua_setmetatable(l, -2);

//open the actual context
	(*context)=alcCreateContext(device,attr);							// request 48k mixer by default
	if(!(*context)) { (*context)=alcCreateContext(device,NULL); }	 	// try again with no attribs
	if(!(*context)) { return 0; } 										// fail at this point

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **context and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCcontext ** lua_alc_get_context_ptr (lua_State *l,int idx)
{
ALCcontext **context;
	context = (ALCcontext**)luaL_checkudata(l, idx , lua_alc_context_meta_name);
	return context;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *context and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCcontext * lua_alc_check_context (lua_State *l,int idx)
{	
ALCcontext **context;
	context = lua_alc_get_context_ptr (l,idx);
	if(!*context)
	{
		luaL_error(l,"alc context is null");
	}
	return *context;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for context ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_DestroyContext (lua_State *l)
{	
ALCcontext **context;

	context = lua_alc_get_context_ptr(l, 1 );
	
	if(*context)
	{
		alcDestroyContext(*context);
		(*context)=0;
	}
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Select the current context
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_MakeContextCurrent (lua_State *l)
{	
ALCcontext *context=0;

	if( lua_isuserdata(l,1) ) // allow passing in of null to clear current context
	{
		context = lua_alc_check_context(l, 1);
	}
	
	alcMakeContextCurrent(context);
	
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a def into a property type and size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_alc_get_prop_info (int def, char *flag, int *num)
{
	switch(def)
	{
// int[1]
		case ALC_CAPTURE_SAMPLES:
			*num=1; *flag='i';
		break;
		case ALC_DEFAULT_DEVICE_SPECIFIER:
			*num=1; *flag='s';
		break;
		case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
			*num=1; *flag='s';
		break;
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// start capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_Get (lua_State *l)
{	
ALCdevice *device=0;

char flag=' ';
int num;

int vi[16]={0};
const char *cp=0;

	device = lua_alc_check_any_device(l, 1); // try input or output device
	
	int def=lua_tonumber(l,2);
	
	lua_alc_get_prop_info(def,&flag,&num);
	
	if(flag=='i')
	{
		alcGetIntegerv(device,def,1,vi);
		lua_pushnumber(l,(double)vi[0]);
		return 1;
	}
	else
	if(flag=='s')
	{
		cp=alcGetString(device,def);
		lua_pushstring(l,cp);
		return 1;
	}

	return 0;
	
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_alc_core(lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"GetError",			lua_alc_GetError},

		{"Get",					lua_alc_Get},
		
		{"OpenDevice",			lua_alc_OpenDevice},
		{"CloseDevice",			lua_alc_CloseDevice},

		{"CreateContext",		lua_alc_CreateContext},
		{"DestroyContext",		lua_alc_DestroyContext},
		
		{"MakeContextCurrent",	lua_alc_MakeContextCurrent},

// capture

		{"CaptureOpenDevice",	lua_alc_CaptureOpenDevice},
		{"CaptureCloseDevice",	lua_alc_CaptureCloseDevice},
		{"CaptureStart",		lua_alc_CaptureStart},
		{"CaptureStop",			lua_alc_CaptureStop},
		{"CaptureSamples",		lua_alc_CaptureSamples},

		{0,0}
	};
	const luaL_Reg meta_capture_device[] =
	{
		{"__gc",			lua_alc_CaptureCloseDevice},
		{0,0}
	};	const luaL_Reg meta_device[] =
	{
		{"__gc",			lua_alc_CloseDevice},
		{0,0}
	};
	const luaL_Reg meta_context[] =
	{
		{"__gc",			lua_alc_DestroyContext},
		{0,0}
	};

	luaL_newmetatable(l, lua_alc_capture_device_meta_name);
	luaL_openlib(l, NULL, meta_capture_device, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_alc_device_meta_name);
	luaL_openlib(l, NULL, meta_device, 0);
	lua_pop(l,1);
		
	luaL_newmetatable(l, lua_alc_context_meta_name);
	luaL_openlib(l, NULL, meta_context, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

