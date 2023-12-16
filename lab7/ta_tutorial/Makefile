CC=gcc 
all: x86_build 
clean: x86_clean 

x86_build:
	$(CC) -o sig_catch sig_catch.c
	$(CC) -o sig_count sig_count.c
	$(CC) -o timer timer.c
	$(CC) -o timer_diff timer_diff.c
	$(CC) -o reaper reaper.c

x86_clean:
	rm -rf sig_catch 
	rm -rf sig_count 
	rm -rf timer 
	rm -rf timer_diff 
	rm -rf reaper 
