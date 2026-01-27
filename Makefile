TOOLCHAIN_PATH = /home/arno/insa/4A/SEE/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/
CC = $(TOOLCHAIN_PATH)arm-none-linux-gnueabihf-gcc

TARGET = sensor_bridge
SRC = test.c
CFLAGS = -Wall -O3 -march=armv7-a -mfloat-abi=hard

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean