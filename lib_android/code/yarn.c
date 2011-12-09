/* San Angeles Observation OpenGL ES version example
 * Copyright 2009 The Android Open Source Project
 * All rights reserved.
 
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 */
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>

#include "com_wetgenes_Yarn.h"


JNIEXPORT void JNICALL
	Java_com_wetgenes_Yarn_setup(
		JNIEnv*  env,
		jclass class
	)
{
	int i=1;
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Yarn_clean(
		JNIEnv*  env,
		jclass class
	)
{
	int i=1;
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Yarn_c_1update(
		JNIEnv*  env,
		jclass class
	)
{
	int i=1;
}

JNIEXPORT jstring JNICALL
	Java_com_wetgenes_Yarn_getstring(
		JNIEnv*  env,
		jclass class
	)
{
	const char *s="Hello Yarn";
	
	return (*env)->NewStringUTF(env, s);
}

JNIEXPORT jint JNICALL
	JNI_OnLoad(
		JavaVM *vm,
		void *reserved
	)
{
void *jni_export_funcs[]=
{
	Java_com_wetgenes_Yarn_setup,
	Java_com_wetgenes_Yarn_clean,
	Java_com_wetgenes_Yarn_c_1update,
	Java_com_wetgenes_Yarn_getstring,
	0
};

    __android_log_print(ANDROID_LOG_INFO, "Yarn", "Booting Yarn...");

	return JNI_VERSION_1_6;
}


#if 0

int   gAppAlive   = 1;

static int  sWindowWidth  = 320;
static int  sWindowHeight = 480;
static int  sDemoStopped  = 0;
static long sTimeOffset   = 0;
static int  sTimeOffsetInit = 0;
static long sTimeStopped  = 0;

static long
_getTime(void)
{
    struct timeval  now;

    gettimeofday(&now, NULL);
    return (long)(now.tv_sec*1000 + now.tv_usec/1000);
}

/* Call to initialize the graphics state */
void
Java_com_example_SanAngeles_DemoRenderer_nativeInit( JNIEnv*  env )
{
    importGLInit();
    appInit();
    gAppAlive    = 1;
    sDemoStopped = 0;
    sTimeOffsetInit = 0;
}

void
Java_com_example_SanAngeles_DemoRenderer_nativeResize( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    sWindowWidth  = w;
    sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "SanAngeles", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
void
Java_com_example_SanAngeles_DemoRenderer_nativeDone( JNIEnv*  env )
{
    appDeinit();
    importGLDeinit();
}

/* This is called to indicate to the render loop that it should
 * stop as soon as possible.
 */
void
Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env )
{
    sDemoStopped = !sDemoStopped;
    if (sDemoStopped) {
        /* we paused the animation, so store the current
         * time in sTimeStopped for future nativeRender calls */
        sTimeStopped = _getTime();
    } else {
        /* we resumed the animation, so adjust the time offset
         * to take care of the pause interval. */
        sTimeOffset -= _getTime() - sTimeStopped;
    }
}

/* Call to render the next GL frame */
void
Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env )
{
    long   curTime;

    /* NOTE: if sDemoStopped is TRUE, then we re-render the same frame
     *       on each iteration.
     */
    if (sDemoStopped) {
        curTime = sTimeStopped + sTimeOffset;
    } else {
        curTime = _getTime() + sTimeOffset;
        if (sTimeOffsetInit == 0) {
            sTimeOffsetInit = 1;
            sTimeOffset     = -curTime;
            curTime         = 0;
        }
    }

    //__android_log_print(ANDROID_LOG_INFO, "SanAngeles", "curTime=%ld", curTime);

    appRender(curTime, sWindowWidth, sWindowHeight);
}

#endif
