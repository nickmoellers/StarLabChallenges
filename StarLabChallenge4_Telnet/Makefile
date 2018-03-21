obj-m+=fib.o
 
all:
	$(CC) telnet.c -o telnet.o

clean:
	-rm telnet.o

test:
	./telnet.o &
	telnet localhost 4567