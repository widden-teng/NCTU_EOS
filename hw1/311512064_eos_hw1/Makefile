CC=gcc # or CC = /usr/bin/aarch64-linux-gnu-gcc
obj-m:= my_driver.o
all: x86_build # or RPI_build
clean: x86_clean # or RPI_clean
x86_build:
	make -C /lib/modules/5.15.0-86-generic/build M=$(PWD) modules
	$(CC) -o hw1 hw1.c

x86_clean:
	make -C /lib/modules/5.15.0-86-generic/build M=$(PWD) clean
	rm -rf hw1 hw1

RPI_build:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules
	$(CC) -o hw1 hw1.c

RPI_clean:
	make -ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean
	rm -rf hw1 reader