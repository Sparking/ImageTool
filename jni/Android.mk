project_path := $(CURDIR)
srcfiles := $(addprefix $(project_path)/,common/bitmatrix.c common/bitstream.c common/linkedlist.c common/queue.c common/stack.c common/rbtree.c common/maths.c common/port_memory.c image/image.c dotcode_detect_point.c decode/qr_v2/qr_position.c decode/qr_v2/qr_decode.c decode/rs/generic_gf.c decode/rs/rsdecode.c lib/iniparser/iniparser.c lib/kissfft-131/kiss_fft.c lib/kissfft-131/tools/kiss_fftnd.c lib/kissfft-131/tools/kiss_fftr.c lib/kissfft-131/tools/kiss_fftndr.c)
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
	-DENABLE_MEMORY_POOL=0U \
	-Dkiss_fft_scalar=float \
	-I$(project_path)/common \
	-I$(project_path)/image \
	-I$(project_path)/decode/qr_v2 \
	-I$(project_path)/decode/rs \
	-I$(project_path)/lib/iniparser \
	-I$(project_path)/lib/kissfft-131 \
	-I$(project_path)/lib/kissfft-131/tools
LOCAL_LIBS      := -jpeg -lpng -lz -lm
#APP_MODULES     := $(LOCAL_MODULE)

include $(BUILD_SHARED_LIBRARY)

