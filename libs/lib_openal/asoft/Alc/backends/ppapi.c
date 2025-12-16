/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <stdlib.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"

#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_audio.h"
#include "ppapi/c/ppb_audio_config.h"
#include "ppapi/c/ppb_core.h"

/* The output buffer will be this multiple of the OpenAL device update size.
 * This needs to be at least 2 or we can't buffer output properly.
 */
const ALuint kBufferPadMult = 4*4;
/* How many samples for each frame will be buffered to Pepper.
 * Keep this low for low latency, but not too low or we will be CPU bound.
 */
const ALuint kRequestedFrameCount = 512;


typedef struct {
    /* Simple circular buffer (single producer/consumer) to buffer data output
     * from OpenAL before it's handed off to PPAPI.
     */
    ALvoid *buffer;
    ALuint size;

    PPB_AudioConfig* audio_config;
    PPB_Audio* audio;
    PPB_Core* core;

    PP_Resource audio_config_resource;
    PP_Resource audio_resource;
    ALuint sample_frame_count;
    
    volatile ALuint read_ptr;
    volatile ALuint write_ptr;

    ALCdevice *device;

    volatile int killNow;
    ALvoid *thread;
    volatile ALuint initialized;
    ALuint buffer_ready;
} ppapi_data;

static PP_Instance gInstance;
static PPB_GetInterface gGetInterface;

extern void alSetPpapiInfo(PP_Instance instance, PPB_GetInterface get_interface);
void alSetPpapiInfo(PP_Instance instance, PPB_GetInterface get_interface)
{
    gInstance = instance;
    gGetInterface = get_interface;
    
}

/* This is the callback from PPAPI to fill in the audio buffer. */
void PPAPI_Audio_Callback(void *sample_buffer,
                          uint32_t buffer_size_in_bytes,
                          void* user_data)
{
    ppapi_data* data = (ppapi_data*)user_data;
    if (!data->buffer_ready) return;

    ALuint write_proxy = data->write_ptr;
    if (data->read_ptr > data->write_ptr)
        write_proxy = data->write_ptr + data->size;

    if (data->read_ptr + buffer_size_in_bytes > write_proxy)
    {
        AL_PRINT("buffer underrun\n");
        return;
    }

    if (data->read_ptr + buffer_size_in_bytes > data->size)
    {
        /* This read straddles the buffer boundary, split it up. */
        memcpy(sample_buffer,
               data->buffer + data->read_ptr,
               data->size - data->read_ptr);
        memcpy(sample_buffer + (data->size - data->read_ptr),
               data->buffer,
               buffer_size_in_bytes - (data->size - data->read_ptr));
    }
    else
    {
        memcpy(sample_buffer,
               data->buffer + data->read_ptr,
               buffer_size_in_bytes);
    }

    data->read_ptr += buffer_size_in_bytes;
    if (data->read_ptr >= data->size)
        data->read_ptr -= data->size;
}

static const ALCchar ppapiDevice[] = "PPAPI Output";

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static ALuint PpapiProc(ALvoid *ptr)
{
    ALCdevice *Device = (ALCdevice*)ptr;
    ppapi_data *data = (ppapi_data*)Device->ExtraData;

    ALuint UpdateSizeInBytes = Device->UpdateSize * 4;
    ALuint SampleFrameInBytes = data->sample_frame_count * 4;
    /* Start to buffer when less than this many bytes are buffered. Keep this
     * small for low latency but large enough so we don't starve Pepper.
     */
    const ALuint MinBufferSizeInBytes =
        MIN(MAX(SampleFrameInBytes*4, UpdateSizeInBytes), data->size/2);

    while(!data->killNow && Device->Connected)
    {
        ALuint write_proxy = data->write_ptr;
        if (data->read_ptr > data->write_ptr)
            write_proxy = data->write_ptr + data->size;

        if (data->read_ptr + MinBufferSizeInBytes >= write_proxy)
        {
            aluMixData(Device,
                       data->buffer + data->write_ptr,
                       Device->UpdateSize);
            if (data->write_ptr + UpdateSizeInBytes > data->size)
            {
                /* Spilled over the edge, copy the last bits to the beginning
                 * of the buffer.
                 */
                memcpy(data->buffer,
                       data->buffer + data->size,
                       UpdateSizeInBytes - (data->size - data->write_ptr));
            }

            data->write_ptr += UpdateSizeInBytes;
            if (data->write_ptr >= data->size)
                data->write_ptr -= data->size;
        }
        /* Small sleep so we don't use too much CPU time. */
        Sleep(1);
    }

    return 0;
}

/* This needs to be called on the main PPAPI thread. */
static void ppapi_open_playback_main_thread(void* user_data, int32_t result)
{
    ppapi_data *data = (ppapi_data*)user_data;
    (void)result;
	if(!data->initialized)
	{
		data->
		sample_frame_count =
			data->
			audio_config->
			RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100,
														  kRequestedFrameCount);

		PP_AudioSampleRate rate = PP_AUDIOSAMPLERATE_44100;
		if (data->device->Frequency == 48000)
			rate = PP_AUDIOSAMPLERATE_48000;

		data->
		audio_config_resource =
			data->
			audio_config->
			CreateStereo16Bit(gInstance,
												  rate,
												  data->sample_frame_count);

		data->audio_resource = data->audio->Create(gInstance,
												   data->audio_config_resource,
												   PPAPI_Audio_Callback,
												   (void*)data);

		data->audio->StartPlayback(data->audio_resource);
		data->initialized = 1;
	}

}

static ALCenum ppapi_open_playback(ALCdevice *device,
                                      const ALCchar *deviceName)
{
    ppapi_data *data;

    if(!deviceName)
        deviceName = ppapiDevice;
    else if(strcmp(deviceName, ppapiDevice) != 0)
        return ALC_INVALID_VALUE;

    int channels = ChannelsFromDevFmt(device->FmtChans);
    int bytes = BytesFromDevFmt(device->FmtType);

    if (channels != 2)
    {
        AL_PRINT("PPAPI only supports 2 channel output\n");
        return ALC_INVALID_VALUE;
    }
    if (bytes != 2)
    {
printf("format=%s\n",DevFmtTypeString(device->FmtType));
        AL_PRINT("PPAPI only supports 16-bit output\n");
        return ALC_INVALID_VALUE;
    }
    if (device->Frequency != 44100 && device->Frequency != 44800)
    {
        AL_PRINT("PPAPI only supports 44100 and 44800 sample frequencies\n");
        return ALC_INVALID_VALUE;
    }

    data = (ppapi_data*)calloc(1, sizeof(*data));

    device->szDeviceName = strdup(deviceName);
    device->ExtraData = data;
    data->device = device;

    data->audio_config =
        (PPB_AudioConfig*)gGetInterface(PPB_AUDIO_CONFIG_INTERFACE);
    if (!data->audio_config)
    {
        free(data);
        return ALC_INVALID_VALUE;
    }

    data->audio = (PPB_Audio*)gGetInterface(PPB_AUDIO_INTERFACE);
    if (!data->audio)
    {
        free(data);
        return ALC_INVALID_VALUE;
    }

    data->core = (PPB_Core*)gGetInterface(PPB_CORE_INTERFACE);
    if (!data->core)
    {
        free(data);
        return ALC_INVALID_VALUE;
    }
/*
    struct PP_CompletionCallback cb =
        PP_MakeCompletionCallback(ppapi_open_playback_main_thread, data);
    data->core->CallOnMainThread(0, cb, 0);
*/
// assume we are on themain thread otherwise we lock because sleep does not sleep?
ppapi_open_playback_main_thread(data,0);

    return ALC_NO_ERROR;
}


/* This needs to be called on the main PPAPI thread. */
static void ppapi_close_playback_main_thread(void* user_data, int32_t result)
{
    ppapi_data *data = (ppapi_data*)user_data;
    (void)result;

    data->audio->StopPlayback(data->audio_resource);

    data->core->ReleaseResource(data->audio_resource);
    data->core->ReleaseResource(data->audio_config_resource);

    if (data->buffer)
      free(data->buffer);
    free(data);
}

static void ppapi_close_playback(ALCdevice *device)
{
    ppapi_data *data = (ppapi_data*)device->ExtraData;

    struct PP_CompletionCallback cb =
        PP_MakeCompletionCallback(ppapi_close_playback_main_thread, data);
    data->core->CallOnMainThread(0, cb, 0);

    device->ExtraData = NULL;
}

static ALCboolean ppapi_reset_playback(ALCdevice *device)
{

    ppapi_data *data = (ppapi_data*)device->ExtraData;
    /* Initialization via 'open_playback' happens asychronously, so we
     * need to wait until this happens in order for data to be ready
     */
    while (!data->initialized) Sleep(100);
    

    /* 4 is 2-channels, 2-bytes per sample. */
    ALuint UpdateSizeInBytes = device->UpdateSize * 4;
    /* kBufferPadMult is added to protect against buffer underruns. */
    data->size = UpdateSizeInBytes * kBufferPadMult;
    /* Extra UpdateSize added so we can read off the end of the buffer in one
     * shot from aluMixData, but treat the buffer like it's of size data->size.
     */
    data->buffer = calloc(1, data->size + UpdateSizeInBytes);
    if(!data->buffer)
    {
        AL_PRINT("buffer malloc failed\n");
        return ALC_FALSE;
    }
    SetDefaultWFXChannelOrder(device);

    data->read_ptr = 0;
    data->write_ptr = 0;
    data->thread = StartThread(PpapiProc, device);
    if(data->thread == NULL)
    {
        free(data->buffer);
        data->buffer = NULL;
        return ALC_FALSE;
    }
    data->buffer_ready = 1;
    return ALC_TRUE;
}

static ALCboolean ppapi_start_playback(ALCdevice *device)
{
    ppapi_data *data = (ppapi_data*)device->ExtraData;
    
	ppapi_open_playback_main_thread(data,0);
	ppapi_reset_playback(device);

    return ALC_TRUE;
}

static void ppapi_stop_playback(ALCdevice *device)
{
    ppapi_data *data = (ppapi_data*)device->ExtraData;

    if(!data->thread)
        return;

    data->killNow = 1;
    StopThread(data->thread);
    data->thread = NULL;

    data->killNow = 0;

    data->buffer_ready = 0;
    free(data->buffer);
    data->buffer = NULL;
}


static ALCenum ppapi_open_capture(ALCdevice *device,
                                     const ALCchar *deviceName)
{
    (void)device;
    (void)deviceName;
    return ALC_INVALID_VALUE;
}


BackendFuncs ppapi_funcs = {
    ppapi_open_playback,
    ppapi_close_playback,
    ppapi_reset_playback,
    ppapi_start_playback,
    ppapi_stop_playback,
    ppapi_open_capture,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

ALCboolean alc_ppapi_init(BackendFuncs *func_list)
{
    *func_list = ppapi_funcs;
    
    return ALC_TRUE;
}

void alc_ppapi_deinit(void)
{
}

void alc_ppapi_probe(enum DevProbe type)
{
//    if(type == DEVICE_PROBE)
//        AppendDeviceList(ppapiDevice);
//        AppendAllDeviceList(ppapiDevice);
//    else
    if(type == ALL_DEVICE_PROBE)
        AppendAllDeviceList(ppapiDevice);
}
