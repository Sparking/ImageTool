AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := rc
CFLAGS   := -g -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing
CPPFLAGS := -Wall -Werror -ansi -MMD -I$(CURDIR) -DCHECK_ENTRY_PARAM
LDFLAGS  := -Wl,--as-needed -Wl,-gc-section -L$(CURDIR)
LIBS     :=

ifeq ($(OS),Windows_NT)
CPPFLAGS += -I$(CURDIR)/lib/gcc/include -I$(CURDIR)/lib/gcc/opencv/include
LDFLAGS  += -L$(CURDIR)/lib/gcc/lib
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
CPPFLAGS   += -I$(CURDIR)/common -DENABLE_MEMORY_POOL=1U

# 图形处理的基础库
IMAGE_LIB_SRC := image/image.c image/bitmap.c rf_edges.c
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

ifeq ($(OS),Windows_NT)
LIBS += -lpng.dll -lz.dll -lopencv_world411.dll
DLL_LIBS := zlib1.dll libpng16-16.dll libopencv_world411.dll opencv_videoio_ffmpeg411_64.dll
else
LIBS += -lz -lpng `pkg-config --libs opencv`
DLL_LIBS :=
endif

LIBS += -lqrencode -ljpeg -lm

USER_SRC := demo.cc
USER_OBJ := $(patsubst %.cc,%.o,$(USER_SRC))
USER_DEP := $(patsubst %.cc,%.d,$(USER_SRC))

define compile_c
@echo CC	$1
@$(CC) $(CPPFLAGS) -std=c99 $(CFLAGS) -c -o $1 $2
endef

define compile_cc
@echo CXX	$1
@$(CC) $(CPPFLAGS) -std=c++11 $(CFLAGS) -c -o $1 $2
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
$(OUT): $(USER_OBJ) $(COMMON_LIB) $(IMAGE_LIB) $(QR_DECODE_LIB) $(INIPARSER_LIB) $(DLL_LIBS)
	$(call link_objects,$@,$(USER_OBJ))
ifeq ($(OS), Windows_NT)
%.dll: $(CURDIR)/lib/gcc/lib/%.dll
	@echo COPY	$@
	@copy $(subst /,\, $<) $@ > .$@.copy.tmp
	@del .$@.copy.tmp
endif
# build objects
-include $(COMMON_LIB_DEP) $(IMAGE_LIB_DEP) $(QR_DECODE_LIB_DEP) $(INIPARSER_LIB_DEP) $(USER_DEP)
$(COMMON_LIB_OBJ) $(IMAGE_LIB_OBJ) $(QR_DECODE_LIB_OBJ) $(INIPARSER_LIB_OBJ): %.o: %.c
	$(call compile_c,$@,$<)
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
