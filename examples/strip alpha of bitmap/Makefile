AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := rc
CFLAGS   := -g -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing
CPPFLAGS := -Wall -Werror -ansi -MMD -I"$(CURDIR)/../.." -DCHECK_ENTRY_PARAM
LDFLAGS  := -Wl,--as-needed -Wl,-gc-section -L"$(CURDIR)/../.."
LIBS     := -limage

ifeq ($(OS),Windows_NT)
CPPFLAGS += -I"$(CURDIR)/../.."/lib/gcc/include -I"$(CURDIR)/../.."/lib/gcc/opencv/include \
	-I"$(CURDIR)/../.."/image -I"$(CURDIR)/../.."/common
LDFLAGS  += -L"$(CURDIR)/../.."/lib/gcc/lib
else
CPPFLAGS += -D_GNU_SOURCE
endif

ifeq ($(OS),Windows_NT)
OUT      := run.exe
else
OUT      := run
endif

ifeq ($(OS),Windows_NT)
LIBS += -lpng.dll -lz.dll
DLL_LIBS := zlib1.dll libpng16-16.dll
else
LIBS += -lz -lpng `pkg-config --libs opencv`
DLL_LIBS :=
endif

LIBS += -ljpeg -lm

SRC := demo.c
OBJ := demo.o
DEP := demo.d

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

$(OUT): ../../libimage.a $(OBJ)
	$(call link_objects,$@,$(OBJ))

../../libimage.a: ../../Makefile
	$(MAKE) -C ../../ libimage.a

$(OBJ): $(SRC)
	$(call compile_c,$@,$<)

