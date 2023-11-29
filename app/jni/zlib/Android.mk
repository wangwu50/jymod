MY_LOCAL_PATH := $(call my-dir)


# Enable this if you want to support loading JPEG images
# The library path should be a relative path to this directory.

#MINIZIP_LIBRARY_PATH := contrib/minizip

#include $(MY_LOCAL_PATH)/$(MINIZIP_LIBRARY_PATH)/Android.mk



# Restore local path
LOCAL_PATH := $(MY_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := zlib

LOCAL_SRC_FILES :=  \
    adler32.c           \
    compress.c       \
    crc32.c       \
    deflate.c       \
    gzclose.c       \
    gzlib.c       \
    gzread.c       \
    gzwrite.c       \
    infback.c       \
    inffast.c       \
    inflate.c       \
    inftrees.c      \
    trees.c       \
    uncompr.c       \
    zutil.c  

LOCAL_CFLAGS := -DLOAD_BMP -DLOAD_GIF -DLOAD_LBM -DLOAD_PCX -DLOAD_PNM \
                -DLOAD_SVG -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV


LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MINIZIP_LIBRARY_PATH)

LOCAL_STATIC_LIBRARIES += minizip



LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
