LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := os.storage

#sources
LOCAL_C_INCLUDES := include/
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/source/*.cpp)

#build static library
include $(BUILD_STATIC_LIBRARY)
