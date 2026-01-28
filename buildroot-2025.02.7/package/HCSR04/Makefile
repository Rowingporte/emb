obj-m += HCSR04.o

BR_ROOT = /home/arno/insa/4A/emb/buildroot-2025.02.7
KDIR = $(BR_ROOT)/output/build/linux-6.9.8

CROSS_PATH = $(BR_ROOT)/output/host/bin/arm-buildroot-linux-gnueabihf-

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_PATH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean