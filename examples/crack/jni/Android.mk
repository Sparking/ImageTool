project_path := $(CURDIR)
srcfiles := $(addprefix $(project_path)/,crack.c)
include $(CLEAR_VARS)
project_path := $(CURDIR)
LOCAL_PATH      := $(call my-dir)
LOCAL_MODULE    := ImageIool
LOCAL_SRC_FILES := $(srcfiles)
LOCAL_CFLAGS  := \
	-Wall \
	-Werror \
	-std=c99 \
	-D_GNU_SOURCE \

include $(BUILD_SHARED_LIBRARY)

