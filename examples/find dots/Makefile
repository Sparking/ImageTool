AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := rc
CFLAGS   := -g -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing
CPPFLAGS := -Wall -Werror -ansi -MMD -DCHECK_ENTRY_PARAM \
		-I"$(CURDIR)/../.." \
		-I"$(CURDIR)/../../image" \
		-I"$(CURDIR)/../../common" 
LDFLAGS  := -Wl,--as-needed -Wl,-gc-section -L"$(CURDIR)/../.."
LIBS     := -limage

ifeq ($(OS),Windows_NT)
CPPFLAGS += -I"$(CURDIR)/../.."/lib/gtk+-bundle_3.6.4-20130513_win64/include
LDFLAGS  += -L"$(CURDIR)/../.."/lib/gtk+-bundle_3.6.4-20130513_win64/lib
else
CPPFLAGS += -D_GNU_SOURCE
endif

ifeq ($(OS),Windows_NT)
OUT      := run.exe
else
OUT      := run
endif

ifeq ($(OS),Windows_NT)
LIBS += -ljpeg.dll -lpng.dll -lz.dll
DLL_LIBS := libjpeg-9.dll libpng15-15.dll zlib1.dll
else
LIBS += -ljpeg -lpng -lz
DLL_LIBS :=
endif

LIBS += -lm

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

$(OUT): ../../libimage.a $(OBJ) $(DLL_LIBS)
	$(call link_objects,$@,$(OBJ))
ifeq ($(OS), Windows_NT)
%.dll: ../../lib/gtk+-bundle_3.6.4-20130513_win64/bin/%.dll
	@echo COPY	$@
	@copy "$(subst /,\,$<)" $@ > .$@.copy.tmp
	@del .$@.copy.tmp
endif
../../libimage.a: ../../Makefile
	$(MAKE) -C ../../ libimage.a

$(OBJ): $(SRC)
	$(call compile_c,$@,$<)

.PHONY: clean
clean:
	$(RM) $(OUT)
	$(RM) $(OBJ)
