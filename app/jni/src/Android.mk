LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main52

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL_image
SDL_TTF_PATH := ../SDL_ttf
SDL_MIXER_PATH := ../SDL_mixer
LUA_PATH := ../LUA52
BASS_PATH := ../Bass
ZIP_PATH := ../zlib

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
										$(LOCAL_PATH)/$(SDL_PATH)/include \
										$(LOCAL_PATH)/$(SDL_IMAGE_PATH) \
										$(LOCAL_PATH)/$(SDL_TTF_PATH) \
										$(LOCAL_PATH)/$(SDL_MIXER_PATH) \
										$(LOCAL_PATH)/$(LUA_PATH) \
										$(LOCAL_PATH)/$(BASS_PATH)/include \
										$(LOCAL_PATH)/$(ZIP_PATH) \
										$(LOCAL_PATH)/$(ZIP_PATH)/contrib/minizip

# Add your application source files here...

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.cpp) \
	$(SDL_PATH)/src/main/android/SDL_android_main.c )

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_ttf bass lua52 zlib minizip

# LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := main54

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL_image
SDL_TTF_PATH := ../SDL_ttf
SDL_MIXER_PATH := ../SDL_mixer
LUA_PATH := ../LUA54
BASS_PATH := ../Bass
ZIP_PATH := ../zlib

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
										$(LOCAL_PATH)/$(SDL_PATH)/include \
										$(LOCAL_PATH)/$(SDL_IMAGE_PATH) \
										$(LOCAL_PATH)/$(SDL_TTF_PATH) \
										$(LOCAL_PATH)/$(SDL_MIXER_PATH) \
										$(LOCAL_PATH)/$(LUA_PATH) \
										$(LOCAL_PATH)/$(BASS_PATH)/include \
										$(LOCAL_PATH)/$(ZIP_PATH) \
										$(LOCAL_PATH)/$(ZIP_PATH)/contrib/minizip

# Add your application source files here...

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.cpp) \
	$(SDL_PATH)/src/main/android/SDL_android_main.c )

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_ttf bass lua54 zlib minizip

# LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid

include $(BUILD_SHARED_LIBRARY)
