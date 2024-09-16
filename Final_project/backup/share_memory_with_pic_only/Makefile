CC=gcc
CFLAGS=-I/usr/include/x86_64-linux-gnu/ImageMagick-6 -I/usr/include/ImageMagick-6 -DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE=0
LIBS=-lMagickWand-6.Q16 -lMagickCore-6.Q16

all: x86_build

clean: x86_clean

x86_build:
	$(CC) $(CFLAGS)  -o get_share_pic get_share_pic.c $(LIBS)
	$(CC) $(CFLAGS)  -o pic_in_shm pic_in_shm.c $(LIBS)

x86_clean:
	rm -rf get_share_pic
	rm -rf pic_in_shm
