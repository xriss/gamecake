LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL

LOCAL_SRC_FILES := \
    importgl.c \
    demo.c \
    app-android.c \

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

LOCAL_SHARED_LIBRARIES := luandroid


include $(BUILD_SHARED_LIBRARY)
