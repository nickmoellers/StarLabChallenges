/**
 * @file   fib.c
 * @author Nicholas Moellers
 * @date   17 March 2018
 * @version 1
 * @brief   A readonly character driver which creates an array of fibonacci numbers which can be accessed
 * by the read call. This module maps to /dev/fib 
 * Currently len
 * Starter code from http://www.derekmolloy.ie/
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/slab.h>           // needed for kmalloc and kree 
#define  DEVICE_NAME "fib"    ///< The device will appear at /dev/fib using this value
#define  CLASS_NAME  "fib"        ///< The device class -- this is a character device driver

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Nicholas Moellers");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver which returns the fibonacci of n");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
//static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
//static               ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  fibClass  = NULL; ///< The device-driver class struct pointer
static struct device* fibDevice = NULL; ///< The device-driver device struct pointer

//My vars
static unsigned int * fibonacciArray;
//static unsigned int * fibonacciArrayOffset; 
static loff_t fibonacciArrayLength=0;
static loff_t fibonacciArrayN=0;

static void freeFibonacci( void );
static void calcFibonacci( unsigned int );
static unsigned int fibonacci( unsigned int );

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static loff_t dev_llseek(struct file *, loff_t , int );

//static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   //.write = dev_write,
   .release = dev_release,
   .llseek = dev_llseek,
};

void freeFibonacci() {
   kfree(fibonacciArray);
   fibonacciArrayLength=0;
}

void calcFibonacci( unsigned int n ) {
   unsigned int fibonacciArrayLengthRequired = MAX(n+1, 2);
   unsigned int i = 0;

   //Initialize array
   if( fibonacciArrayLength < 2 ) { 
      fibonacciArray = (unsigned int*) kmalloc( fibonacciArrayLengthRequired * sizeof(int), GFP_KERNEL );
      fibonacciArray[0]=0;
      fibonacciArray[1]=1;
      fibonacciArrayLength=2;
   //OR Reinitialize Array
   } else if( fibonacciArrayLength < fibonacciArrayLengthRequired ) {
      unsigned int * newFibonacciArray;
      newFibonacciArray = kmalloc( fibonacciArrayLengthRequired*sizeof(int) , GFP_KERNEL);
      memcpy( newFibonacciArray, fibonacciArray, fibonacciArrayLength*sizeof(int) );
      kfree(fibonacciArray);
      fibonacciArray=newFibonacciArray;
      //fibonacciArray = (unsigned int*) realloc( fibonacciArray, fibonacciArrayLengthRequired*sizeof(int));
   }

   if( fibonacciArray == NULL ) {
      fibonacciArrayLength = 0;
      return ;
   }

   //Fill the array
   for( i = fibonacciArrayLength; i < fibonacciArrayLengthRequired ; i ++ ) {

      //Calculate the next fibonacci number
      fibonacciArray[i]=fibonacciArray[i-1]+fibonacciArray[i-2];

      //Check for overflow
      if( fibonacciArray[i] < fibonacciArray[i-1] ) {
         fibonacciArray[i] = -1;
      }
      
   }
   fibonacciArrayLength = fibonacciArrayLengthRequired;
}

unsigned int fibonacci( unsigned int n ) {
   calcFibonacci(n);
   return fibonacciArray[n];
}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init fib_init(void){
   printk(KERN_INFO "Fib: Initializing the fib LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Fib: failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Fib: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   fibClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(fibClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Fib: Failed to register device class\n");
      return PTR_ERR(fibClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "Fib: device class registered correctly\n");

   // Register the device driver
   fibDevice = device_create(fibClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(fibDevice)){               // Clean up if there is an error
      class_destroy(fibClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Fib: Failed to create the device\n");
      return PTR_ERR(fibDevice);
   }
   printk(KERN_INFO "Fib: device class created correctly\n"); // Made it! device was initialized

   printk(KERN_INFO "Fib: calculating fibonacci!\n"); // Made it! device was initialized
   calcFibonacci(10); //initialize with something reasonable.
   
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit fib_exit(void){
   freeFibonacci();

   device_destroy(fibClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(fibClass);                          // unregister the device class
   class_destroy(fibClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);         // unregister the major number
   printk(KERN_INFO "Fib: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "Fib: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len Ignored 
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   unsigned int fibonacci_of_n = -1;
   //unsigned int n = len;
   short  size_of_message;

   printk( KERN_NOTICE "Fib: in dev_read()" );

   fibonacci_of_n = fibonacci(fibonacciArrayN);
   size_of_message = sizeof(int)/sizeof(char);
   printk( KERN_NOTICE "Fib: copying : %d to %p of size %d\n",
      fibonacci_of_n, &buffer, size_of_message );

   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, &fibonacci_of_n, size_of_message);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "Fib: Sent %d to the user\n", fibonacci_of_n);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "Fib: Failed to send %d to the user\n", fibonacci_of_n);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

static loff_t dev_llseek(struct file *filep, loff_t offset, int type ){

   printk( KERN_NOTICE "Fib: in dev_llseek()" );
   switch( type ) {
      case SEEK_SET:
         fibonacciArrayN = offset;
         break;
      case SEEK_CUR:
         fibonacciArrayN += offset;
         break;
      default:
         printk(KERN_ALERT "Fib: Seek type %d not supported.\n", type);
         return -1;
   }

   printk(KERN_INFO "Fib: Seeked to fibonacci of %lld.\n", offset);
   return fibonacciArrayN;
}

//unneeded
/*static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   //size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "Fib: Received %zu characters from the user\n", len);
   return len;
}*/

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Fib: Device successfully closed\n");
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(fib_init);
module_exit(fib_exit);