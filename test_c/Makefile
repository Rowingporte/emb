# Path to your external toolchain
TOOLCHAIN_PATH = /home/arno/insa/4A/SEE/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/
CC = $(TOOLCHAIN_PATH)arm-none-linux-gnueabihf-gcc

# Path to Buildroot sysroot (the root of the target filesystem headers/libs)
BR_SYSROOT = /home/arno/insa/4A/emb/buildroot-2025.02.7/output/host/arm-buildroot-linux-gnueabihf/sysroot

TARGET = sensor_bri
SRC = sensor.c

# Use --sysroot so the compiler finds libc and libgpiod correctly
CFLAGS = -Wall -O3 -march=armv7-a -mfloat-abi=hard --sysroot=$(BR_SYSROOT)
LDFLAGS = -lgpiod

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean