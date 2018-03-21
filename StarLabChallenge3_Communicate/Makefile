obj-m+=comm.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test_comm.c -o test_comm.o

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	-rm test_comm.o

test:

	# Remove the module if it exists
	-sudo rmmod comm
	# Clear the kernel log 
	sudo dmesg -C
	# Insert the module
	sudo insmod comm.ko
	# Display the init code
	#dmesg
	#sudo dmesg -C
	# Test the user code
	sudo ./test_comm.o
	# Display the reads
	#dmesg
	#sudo dmesg -C
	# Remove the module
	sudo rmmod comm
