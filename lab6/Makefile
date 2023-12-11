CC=gcc 
all: x86_build 
clean: x86_clean 

x86_build:
	$(CC) -o client client.c
	$(CC) -o server_mutex server_mutex.c
	$(CC) -o server_binary_semophore server_binary_semophore.c

x86_clean:
	rm -rf client 
	rm -rf server_mutex 
	rm -rf server_binary_semophore
