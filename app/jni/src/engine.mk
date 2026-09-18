LOCAL_PATH := $(JY_SOURCE_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := main$(JY_LUA_VERSION)

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL_image
SDL_TTF_PATH := ../SDL_ttf
SDL_MIXER_PATH := ../SDL_mixer
LUA_PATH := ../lua$(JY_LUA_VERSION)
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

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_ttf bass lua$(JY_LUA_VERSION) zlib minizip

# LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid

include $(BUILD_SHARED_LIBRARY)
