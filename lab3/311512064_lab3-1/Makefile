obj-m += lab3-1_driver.o
KDIR = /home/widden/linux
CC = /usr/bin/aarch64-linux-gnu-gcc
all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules
	$(CC) -o lab3-1_writer_test lab3-1_writer.c
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean
	$(CC) -o lab3-1_writer_test lab3-1_writer.c
