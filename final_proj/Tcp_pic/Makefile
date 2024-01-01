CC=gcc 
all: x86_build 
clean: x86_clean 

x86_build:
	$(CC) -o pic_client pic_client.c
	$(CC) -o pic_server pic_server.c
x86_clean:
	rm -rf pic_client 
	rm -rf pic_server