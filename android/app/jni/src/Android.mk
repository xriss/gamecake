LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := pb-SDL2
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../../../exe/android/lib/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := pb-hidapi
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../../../exe/android/lib/$(TARGET_ARCH_ABI)/libhidapi.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := pb-gamecake
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../../../exe/android/lib/$(TARGET_ARCH_ABI)/libgamecake.so
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../exe/android/include
LOCAL_SRC_FILES := SDL_main.c
LOCAL_SHARED_LIBRARIES := pb-SDL2 pb-gamecake
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog
include $(BUILD_SHARED_LIBRARY)

