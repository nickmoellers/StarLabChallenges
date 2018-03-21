/**
 * @file   test_fib.c
 * @author Nicholas Moellers
 * @date   17 March 2018
 * @version 1
 * @brief  A Linux user space program that communicates with the fib.c LKM. Reads
 * the fib device, passes in n, and expects the fibonacci of n returned. 
 * For this example to work the device must be called /dev/fib.
 * Starter code from http://www.derekmolloy.ie/
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 

static unsigned int fibonacci_of_n;
static unsigned int n;

void printIthFibonacci(unsigned int i, unsigned int fib_of_i) {
   printf("fibonacci[%d] = ", i);
   if( fib_of_i == -1) {
      printf("OVERFLOW");
   } else {
      printf("%u", fib_of_i);
   }
   printf("\n");
}

int main(){
   int ret, fd, n;

   printf("Openning device...");
   fd = open("/dev/fib", O_RDONLY);             // Open the device in readonly mode
   if (fd < 0){
      perror("Failed to open the device.\n");
      return errno;
   } else {
      printf("Success!\n");
   }

   printf("Testing one number...");
   n = 3;
   ret = read(fd, &fibonacci_of_n, n);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.\n");
      return errno;
   } else {
      printf("Success!\n");
   }
   printIthFibonacci(n, fibonacci_of_n);


   srand((unsigned int)time(NULL));
   printf("Randomly test 10 fibanacci numbers:\n");
   for( n = 0 ; n < 10 ; n++ ) {
      unsigned int randn = rand() % 50;
      ret = read(fd, &fibonacci_of_n, randn);        // Read the response from the LKM
      if (ret < 0){
         perror("Failed to read the message from the device.\n");
         return errno;
      }
      printIthFibonacci(randn, fibonacci_of_n); //printf("fibonacci[%u] = %u\n", randn, r);
   }

   printf("Sequentially test numbers 1 through 50:\n");
   for( n = 0 ; n <= 50 ; n++ ) {
      ret = read(fd, &fibonacci_of_n, n);        // Read the response from the LKM
      if (ret < 0){
         perror("Failed to read the message from the device.\n");
         return errno;
      }
      printIthFibonacci(n, fibonacci_of_n);
   }

   printf("End of the program\n");

   close(fd);

   return 0;
}