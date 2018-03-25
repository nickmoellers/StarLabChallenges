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
   int fd;
   loff_t key;
   unsigned int value;

   printf("Openning device...\n");
   fd = open("/dev/comm", O_RDWR);   // Open the device with read/write access
   if (fd < 0){
      printf("Failed to open the device.\n");
      return errno;
   } else {
      printf("Success!\n");
   }

   printf("\nTesting one process write...\n");
   key = 3;
   if( (lseek( fd, key, SEEK_SET)) < 0 ) {
      perror("lseek error.\n");
      return errno;
   }
   value = 7;
   if ( (write(fd, &value, 0)) < 0){
      printf("Failure: failed to read the message from the device.\n");
      return errno;
   } else {
      printf("Success! Wrote %u to %ld.\n", value, key);
   }

   printf("Testing one process read...\n");
   value = -1;
   if ( (read(fd, &value, key)) < 0){
      printf("Failed to read the message from the device.\n");
      return errno;
   } else {
      printf("Success! Read %u from %ld.\n", value, key);
   }
   //return 0;

   //test two processes
   printf("\nTesting two processes; parent should write first then child should read...\n");
   pid = fork(); 
   key = 6;
   if( pid > 0 ) { // parent process 
      processName = "Parent";
      if( (lseek( fd, key, SEEK_SET)) < 0 ) {
         perror("lseek error.\n");
         return errno;
      }
      value = 7000;
      if ( (write(fd, &value, 0)) < 0){
         printf("Failure: %s failed to write the message to the device.\n", processName);
         return errno;
      } else {
         printf("Success! %s wrote %u to %ld.\n", processName, value, key); 
      }
   } else { //child process
      processName = "Child";
      if( (lseek( fd, key, SEEK_SET)) < 0 ) {
         perror("lseek error.\n");
         return errno;
      }
      value = -1;
      if ( (read(fd, &value, key)) < 0){
         printf("Failure: %s failed to read the message from the device.\n", processName);
         return errno;
      } else {
         printf("Success! %s read %u from %ld.\n", processName,  value, key);
      }
      //return because something's not thread safe or something..
      //return 0;  
   } 

   srand((unsigned int)(1+pid)*time(NULL)); //just using times makes both processes do the same thing. Boring.
   printf("\nRandomly test 10 reads and writes:\n");
   for( n = 0 ; n < 10 ; n++ ) {
      key = rand() % 8;
      if( rand() % 2 ) { //write
         if( (lseek( fd, key, SEEK_SET)) < 0 ) {
            perror("lseek error.\n");
            return errno;
         }
         //sleep(0); <- BAD
         value = rand() % INT_MAX-1;
         if ( (write(fd, &value, 0)) < 0){
            printf("Failure: %s failed to write the message to the device.\n", processName);
            return errno;
         } else {
            printf("Success! %s wrote %u to %ld.\n", processName, value, key);
         }
      } else { //read
         if( (lseek( fd, key, SEEK_SET)) < 0 ) {
            perror("lseek error.\n");
            return errno;
         }
         //sleep(0); <- BAD
         value = -1;
         if ( (read(fd, &value, 0)) < 0){
            printf("Failure: %s failed to read the message from the device.\n", processName);
            return errno;
         } else {
            printf("Success! %s read %u from %ld.\n", processName, value, key);
         }
      }
      sleep(0);
   }

   return 0;
}
