LOCAL_PATH := $(call my-dir)/

include $(CLEAR_VARS)

LOCAL_MODULE    := minizip

LOCAL_SRC_FILES := \
    ioapi.c           \
    miniunz.c       \
    minizip.c       \
    mztools.c       \
    unzip.c       \
    zip.c      


LOCAL_CFLAGS += -DAVOID_TABLES 
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

include $(BUILD_STATIC_LIBRARY)
