TOOLS_PREFIX := arm-linux-gnueabihf-

CFLAGS  += -std=c99   -MMD -MP -O3 -g -fPIC -fno-var-tracking-assignments \
	   -DTARGET_OS_linux -DTARGET_ARCH_armv7l \
	   -pthread
LDFLAGS += -pthread
LDLIBS  += -ldl -lm -lrt

# Uncomment to build for gprof profiling
#CFLAGS  += -pg
#LDFLAGS += -pg

TARGET_OS           := linux
TARGET_ARCHITECTURE := armv7l
