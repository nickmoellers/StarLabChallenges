/**
 * @file   comm.c
 * @author Nicholas Moellers
 * @date   18 March 2017
 * @version 1
 * @brief   An  character driver which reads or writes the integer at (len) offset.to support the second article of my series on
 * Linux loadable kernel module (LKM) development. This module maps to /dev/comm and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 * Starter code from http://www.derekmolloy.ie/
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/slab.h>           // needed for kmalloc and kree 
#define  DEVICE_NAME "comm"    ///< The device will appear at /dev/comm using this value
#define  CLASS_NAME  "ebb"        ///< The device class -- this is a character device driver

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Nicholas Moellers");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver which reads and writes ints at an offset n");  ///< The description -- see modinfo
MODULE_VERSION("1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  commClass  = NULL; ///< The device-driver class struct pointer
static struct device* commDevice = NULL; ///< The device-driver device struct pointer

//My vars
static unsigned int * keyValuePairs;
static unsigned int keyValuePairsLength=0;
static loff_t key;

static void freeKeyValuePairs( void );
static void initKeyValuePairs( unsigned int );

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static loff_t dev_llseek(struct file *, loff_t , int );

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
   .llseek = dev_llseek,
};

void freeKeyValuePairs() {
   kfree(keyValuePairs);
   keyValuePairsLength=0;
}

void initKeyValuePairs( unsigned int length ) {
   int i;
   keyValuePairs = (unsigned int*) kmalloc( length * sizeof(unsigned int), GFP_KERNEL );
   if( keyValuePairs !=NULL ) {
      keyValuePairsLength = length;
      for( i = 0 ; i < keyValuePairsLength ; i++ ) {
         keyValuePairs[i]=0;
         //printk(KERN_ALERT "Comm: keyValuePairs[%d]=%u\n", i, keyValuePairs[i]);
      }
   }
}


/*static void print_all(void) {
   int i;
   //printk(KERN_ALERT "Comm: ")
   for( i = 0; i < keyValuePairsLength ; i ++ ) {
      printk(KERN_INFO "Comm: keyValuePairs[%d]=%u\n", i, keyValuePairs[i]);
   }
}*/

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init comm_init(void){
   printk(KERN_INFO "Comm: Initializing the comm LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Comm: failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Comm: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   commClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(commClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Comm: Failed to register device class\n");
      return PTR_ERR(commClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "Comm: device class registered correctly\n");

   // Register the device driver
   commDevice = device_create(commClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(commDevice)){               // Clean up if there is an error
      class_destroy(commClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Comm: Failed to create the device\n");
      return PTR_ERR(commDevice);
   }
   printk(KERN_INFO "Comm: device class created correctly\n"); // Made it! device was initialized

   //allocate memory for the key value pairs and set all values to 0
   printk(KERN_INFO "Comm: initializing key value pairs!\n"); 
   initKeyValuePairs( 8 ); 
   //print_all();  
   
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit comm_exit(void){
   freeKeyValuePairs();

   device_destroy(commClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(commClass);                          // unregister the device class
   class_destroy(commClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "Comm: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "Comm: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief Uses the copy_to_user() function to send the value to the user dicated by the key, which is len
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 *  NOTE: CURRENLTY len IS THE key AND *offset is not used
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count; //= 0;
   unsigned int value; //= keyValuePairs[key];
   short  size_of_message; // = sizeof(value)/sizeof(char); 

   printk( KERN_INFO "Comm: in dev_read()" );
   //print_all();

   error_count = 0;
   value = keyValuePairs[key];
   size_of_message = sizeof(value)/sizeof(char); 

   printk( KERN_INFO "Comm: Key: %lld\t Value: %u\n", key, value );
  
   printk( KERN_INFO "Comm: Reading: %d from %p to %p of size %d\n",
      value, &value, (int*)buffer, size_of_message );

   error_count = copy_to_user((int*)buffer, &value, size_of_message);

   printk( KERN_INFO "Comm: Key: %lld\t Value: %u\n", key, value );

   //print_all();

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "Comm: Success!\n");
      return 0; 
   }
   else {
      printk(KERN_ALERT "Comm: Failure!\n");
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}


/** @brief Uses the copy_from_user() function to grab the value from the user and put it in the location
 *  dicated by the key, which is len
 *  @param filep A pointer to a file object
 *  @param buffer A pointer to the int in user space
 *  @param len ignored 
 *  @param offset ignored
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   int error_count;// = 0;
   unsigned int value;
   short size_of_message;// = sizeof(value)/sizeof(char);

   printk( KERN_INFO "Comm: in dev_write()" );

   //print_all();

   error_count = 0;
   value = -1;
   size_of_message = sizeof(value)/sizeof(char);

   printk( KERN_INFO "Comm: Key: %lld\t *Buffer: %u\n", key, *((int*)buffer) );

   //size_of_message =
   printk( KERN_INFO "Comm: Writing: %d from %p to %p of size %d\n",
      *((int*)buffer), buffer, keyValuePairs+key, size_of_message );

   // copy_from_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_from_user( keyValuePairs+key, buffer, size_of_message);

   value = keyValuePairs[key];
   printk( KERN_INFO "Comm: Key: %lld\t Value: %u\n", key, value );

   //print_all();

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "Comm: Success");
      return 0; 
   }
   else {
      printk(KERN_ALERT "Comm: Failure");
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

static loff_t dev_llseek(struct file *filep, loff_t offset, int type ){

   printk( KERN_NOTICE "Comm: in dev_llseek()" );
   switch( type ) {
      case SEEK_SET:
         key = offset;
         break;
      case SEEK_CUR:
         key += offset;
         break;
      default:
         printk(KERN_ALERT "Comm: Seek type %d not supported.\n", type);
         return -1;
   }

   key %= keyValuePairsLength;

   printk(KERN_INFO "Comm: Seeked to key %lld.\n", key);
   return key;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Comm: Device successfully closed\n");
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(comm_init);
module_exit(comm_exit);