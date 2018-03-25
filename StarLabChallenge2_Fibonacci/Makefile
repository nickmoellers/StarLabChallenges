obj-m+=fib.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) -Wall test_fib.c -o test_fib.o

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	-rm test_fib.o

test:

	# Remove the module if it exists
	-sudo rmmod fib
	# Clear the kernel log 
	#sudo dmesg -C
	# Insert the module
	sudo insmod fib.ko
	# Display the init code
	#dmesg
	#sudo dmesg -C
	# Test the user code
	sudo ./test_fib.o
	# Display the reads
	#dmesg
	#sudo dmesg -C
	# Remove the module
	sudo rmmod fib