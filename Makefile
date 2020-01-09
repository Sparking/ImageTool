AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := crus
CFLAGS   := -g -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing
CXXFLAGS := -std=c++11 $(CFLAGS)
CFLAGS   := -std=c99 $(CFLAGS)
CPPFLAGS := -Wall -Werror -ansi -MMD -I$(CURDIR)
LDFLAGS  := -Wl,--as-needed -Wl,-gc-section -L$(CURDIR)
LIBS     :=

ifeq ($(OS),Windows_NT)
CPPFLAGS += -I$(CURDIR)/lib/gtk+-bundle_3.6.4-20130513_win64/include
LDFLAGS  += -L$(CURDIR)/lib/gtk+-bundle_3.6.4-20130513_win64/lib
else
CPPFLAGS += -D_GNU_SOURCE
endif

ifeq ($(OS),Windows_NT)
OUT      := run.exe
else
OUT      := run
endif

# 基础通用库
COMMON_LIB_SRC := \
	common/bitmatrix.c \
	common/bitstream.c \
	common/linkedlist.c \
	common/queue.c \
	common/stack.c \
	common/rbtree.c \
	common/maths.c \
	common/port_memory.c
COMMON_LIB_OBJ := $(patsubst %.c,%.o,$(COMMON_LIB_SRC))
COMMON_LIB_DEP := $(patsubst %.c,%.d,$(COMMON_LIB_SRC))

COMMON_LIB := libcommon.a
CPPFLAGS   += -I$(CURDIR)/common -DENABLE_MEMORY_POOL=0U

# 图形处理的基础库
IMAGE_LIB_SRC := image/image.c dotcode_detect_point.c
IMAGE_LIB_OBJ := $(patsubst %.c,%.o,$(IMAGE_LIB_SRC))
IMAGE_LIB_DEP := $(patsubst %.c,%.d,$(IMAGE_LIB_SRC))

IMAGE_LIB := libimage.a
CPPFLAGS  += -I$(CURDIR)/image -I$(CURDIR)/image/bitmap

# qr解码的库
QR_DECODE_LIB_SRC := \
	decode/qr_v2/qr_position.c \
	decode/qr_v2/qr_decode.c \
	decode/rs/generic_gf.c \
	decode/rs/rsdecode.c
QR_DECODE_LIB_OBJ := $(patsubst %.c,%.o,$(QR_DECODE_LIB_SRC))
QR_DECODE_LIB_DEP := $(patsubst %.c,%.d,$(QR_DECODE_LIB_SRC))

QR_DECODE_LIB := libqrdecode.a
CPPFLAGS      += -I$(CURDIR)/decode/qr_v2 -I$(CURDIR)/decode/rs
LIBS          += -lqrdecode

INIPARSER_LIB_SRC := lib/iniparser/iniparser.c
INIPARSER_LIB_OBJ := $(patsubst %.c,%.o,$(INIPARSER_LIB_SRC))
INIPARSER_LIB_DEP := $(patsubst %.c,%.d,$(INIPARSER_LIB_SRC))

INIPARSER_LIB := libiniparser.a
CPPFLAGS += -I$(CURDIR)/lib/iniparser
LIBS     += -liniparser

KISSFFT_LIB_SRC_PATH := lib/kissfft-131
KISSFFT_LIB_SRC := \
    $(KISSFFT_LIB_SRC_PATH)/kiss_fft.c \
    $(KISSFFT_LIB_SRC_PATH)/tools/kiss_fftnd.c \
    $(KISSFFT_LIB_SRC_PATH)/tools/kiss_fftr.c \
    $(KISSFFT_LIB_SRC_PATH)/tools/kiss_fftndr.c
KISSFFT_LIB_OBJ := $(patsubst %.c,%.o,$(KISSFFT_LIB_SRC))
KISSFFT_LIB_DEP := $(patsubst %.c,%.d,$(KISSFFT_LIB_SRC))
KISSFFT_LIB := libkissfft131.a
CPPFLAGS    += -I$(KISSFFT_LIB_SRC_PATH) -I$(KISSFFT_LIB_SRC_PATH)/tools
LIBS        += -lkissfft131
KISSFFT_CPPFLAGS := \
    -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return \
    -Wcast-align -Wcast-qual -Wnested-externs -Wshadow -Wbad-function-cast \
    -Wwrite-strings -Dkiss_fft_scalar=float \
    -pedantic -ffast-math -fomit-frame-pointer

ifeq ($(OS),Windows_NT)
LIBS += -ljpeg.dll -lpng.dll -lz.dll
DLL_LIBS := libjpeg-9.dll libpng15-15.dll zlib1.dll
else
LIBS += -ljpeg -lpng -lz
DLL_LIBS :=
endif

LIBS += -lm

USER_SRC := demo.cc
USER_OBJ := $(patsubst %.cc,%.o,$(USER_SRC))
USER_DEP := $(patsubst %.cc,%.d,$(USER_SRC))

define compile_c
@echo CC	$1
@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $1 $2
endef

define compile_kissfft
@echo CC	$1
@$(CC) $(CPPFLAGS) $(KISSFFT_CPPFLAGS) $(CFLAGS) -c -o $1 $2
endef

define compile_cc
@echo CXX	$1
@$(CC) $(CPPFLAGS) $(CXXFLAGS) -c -o $1 $2
endef

define ar_lib
@echo AR	$1
@$(AR) $(ARFLAGS) $1 $2
@$(RANLIB) $1
endef

define link_objects
@echo LD	$1
@$(CXX) $(CFLAGS) $(LDFLAGS) -o $1 $2 $(LIBS)
endef

.PHONY: all
all: $(OUT)
$(OUT): $(USER_OBJ) $(COMMON_LIB) $(IMAGE_LIB) $(QR_DECODE_LIB) $(INIPARSER_LIB) $(KISSFFT_LIB) $(DLL_LIBS)
	$(call link_objects,$@,$(USER_OBJ))
ifeq ($(OS), Windows_NT)
%.dll: $(CURDIR)/lib/gtk+-bundle_3.6.4-20130513_win64/bin/%.dll
	@echo COPY	$@
	@copy "$(subst /,\,$<)" $@ > .$@.copy.tmp
	@del .$@.copy.tmp
endif
# build objects
-include $(COMMON_LIB_DEP) $(IMAGE_LIB_DEP) $(QR_DECODE_LIB_DEP) $(INIPARSER_LIB_DEP) $(KISSFFT_LIB_DEP) $(USER_DEP)
$(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ) $(QR_DECODE_LIB_OBJ) $(INIPARSER_LIB_OBJ): %.o: %.c
	$(call compile_c,$@,$<)
$(KISSFFT_LIB_OBJ): %.o: %.c
	$(call compile_kissfft,$@,$<)
$(USER_OBJ): %.o: %.cc
	$(call compile_cc,$@,$<)
# build libraries
$(COMMON_LIB): $(COMMON_LIB_OBJ)
	$(call ar_lib,$@,$^)
$(IMAGE_LIB): $(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ)
	$(call ar_lib,$@,$^)
$(QR_DECODE_LIB): $(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ) $(QR_DECODE_LIB_OBJ)
	$(call ar_lib,$@,$^)
$(INIPARSER_LIB): $(INIPARSER_LIB_OBJ)
	$(call ar_lib,$@,$^)
$(KISSFFT_LIB): $(KISSFFT_LIB_OBJ)
	$(call ar_lib,$@,$^)

.PHONY: clean
clean:
ifeq ($(OS), Windows_NT)
	@for %%I in ($(subst /,\,$(COMMON_LIB_DEP) $(IMAGE_LIB_DEP) $(QR_DECODE_LIB_DEP) $(USER_DEP) $(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ) $(QR_DECODE_LIB_OBJ) $(USER_OBJ)) $(COMMON_LIB) $(IMAGE_LIB) $(QR_DECODE_LIB) $(DLL_LIBS) $(OUT)) do if exist %%I del /f /q %%I
	@for %%I in ($(subst /,\,$(INIPARSER_LIB) $(INIPARSER_LIB_OBJ) $(INIPARSER_LIB_DEP))) do if exist %%I del /f /q %%I
else
	@$(RM) $(COMMON_LIB_DEP) $(IMAGE_LIB_DEP) $(QR_DECODE_LIB_DEP) $(USER_DEP)
	@$(RM) $(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ) $(QR_DECODE_LIB_OBJ) $(USER_OBJ)
	@$(RM) $(COMMON_LIB) $(IMAGE_LIB) $(QR_DECODE_LIB)
	@$(RM) $(INIPARSER_LIB) $(INIPARSER_LIB_OBJ) $(INIPARSER_LIB_DEP)
	@$(RM) $(OUT)
endif

.PHONY: debug
debug: $(OUT)
	@gdb $(OUT)
