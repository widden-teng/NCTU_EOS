CC=gcc 
all: x86_build 
clean: x86_clean 

x86_build:
	$(CC) -o client client.c
	$(CC) -o server server.c

x86_clean:
	rm -rf client 
	rm -rf server 
