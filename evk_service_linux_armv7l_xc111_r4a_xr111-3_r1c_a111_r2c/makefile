# Prevent make message "Nothing to be done for 'all'." by having a top level
# target which always does something (in this case a no-operation)
all_all : all
	@:

all :

BUILD_ALL  :=
BUILD_LIBS :=
BUILD_POST :=

CFLAGS  :=
LDFLAGS :=
LDLIBS  :=

OUT_LIB_DIR	:= out/lib
VPATH		+= $(OUT_LIB_DIR)
LDFLAGS		+= -L$(OUT_LIB_DIR)

include $(wildcard rule/makefile_target_*.inc)

TARGET := $(TARGET_OS)_$(TARGET_ARCHITECTURE)

AR := $(TOOLS_PREFIX)ar
AS := $(TOOLS_PREFIX)as
CC := $(TOOLS_PREFIX)gcc

VPATH   += include/ lib/ source/ user_include/ user_lib/ user_source/
CFLAGS  += -Iinclude/ -Isource/ -Iuser_include/ -Iuser_source/
LDFLAGS += -Llib/ -Luser_lib/

include $(wildcard rule/makefile_define_*.inc)
include $(wildcard rule/makefile_build_*.inc)
include $(wildcard user_rule/makefile_define_*.inc)
include $(wildcard user_rule/makefile_build_*.inc)

SOURCES := $(wildcard source/*.c) $(wildcard user_source/*.c)
DEPENDS := $(addprefix out/, $(notdir $(SOURCES:.c=.d)))

-include $(DEPENDS)

all : $(BUILD_ALL) $(BUILD_LIBS) $(BUILD_POST)

$(BUILD_ALL) : | $(BUILD_LIBS)

$(BUILD_POST) : | $(BUILD_ALL)

out/%.o : %.c
	@echo "    Compiling $(notdir $<)"
	@mkdir -p out
	@$(COMPILE.c) $(OUTPUT_OPTION) $<

.PHONY : clean
clean :
	@rm -rf out/
