/**
* @file   test_comm.c
* @author Nicholas Moellers
* @date   18 March 2017
* @version 1
* @brief  A Linux user space program that communicates with the comm.c LKM. It passes a
* key to the LKM and eiher a value to write or a buffer to read the value to. 
* For this example to work the device must be called /dev/comm.
* Starter code from http://www.derekmolloy.ie/
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h> 
#include <limits.h>
#include <unistd.h>

int main(void)
{
   pid_t  pid;
   int    n; 
   char * processName;

   //single process
   int ret, fd;
   unsigned int key, value;

   printf("Openning device...\n");
   fd = open("/dev/comm", O_RDWR);   // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device.\n");
      return errno;
   } else {
      printf("Success!\n");
   }

   printf("\nTesting one process read...\n");
   key = 3;
   value = 7;
   ret = write(fd, &value, key);
   if (ret < 0){
      perror("Failure: failed to read the message from the device.\n");
      return errno;
   } else {
      printf("Success! Wrote %d to %d.\n", value, key);
   }

   printf("Testing one process write...\n");
   ret = read(fd, &value, key);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.\n");
      return errno;
   } else {
      printf("Success! Read %d from %d.\n", value, key);
   }

   //test two processes
   printf("\nTesting two processes; parent should write first then child should read...\n");
   pid = fork(); 
   key = 6;
   if( pid > 0 ) { // parent process 
      processName = "Parent";
      value = 7000;
      ret = write(fd, &value, key);
      if (ret < 0){
         perror("Failure: %s failed to write the message to the device.\n", processName);
         return errno;
      } else {
         printf("Success! %s wrote %d to %d.\n", processName, value, key); 
      }
   } else { //child process
      processName = "Child";
      value = -1;
      ret = read(fd, &value, key);        // Read the response from the LKM
      if (ret < 0){
         perror("Failure: %s failed to read the message from the device.\n", processName);
         return errno;
      } else {
         printf("Success! %s read %d from %d.\n", processName,  value, key);
      }
      return 0;  
   } 

   srand((unsigned int)time(NULL));
   printf("\nRandomly test 10 reads and writes - this fails:\n");
   for( n = 0 ; n < 10 ; n++ ) {
      key = rand() % 256;
      if( rand() % 2 ) { //write
         value = rand() % INT_MAX-1;
         ret = write(fd, &value, key);
         if (ret < 0){
            perror("Failure: %s failed to write the message to the device.\n", processName);
            return errno;
         } else {
            printf("Success! %s wrote %d to %d.\n", processName, value, key);
         }
      } else { //read
         value = -1;
         ret = read(fd, &value, key);        // Read the response from the LKM
         if (ret < 0){
            perror("Failure: %s failed to read the message from the device.\n", processName);
            return errno;
         } else {
            printf("Success! %s read %d from %d.\n", processName, value, key);
         }
      }
   }

   return 0;
}
