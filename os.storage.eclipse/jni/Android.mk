LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE    := os.storage
LOCAL_C_INCLUDES := ../include
LOCAL_SRC_FILES := $(wildcard ../source/*.cpp)
include $(BUILD_STATIC_LIBRARY)
