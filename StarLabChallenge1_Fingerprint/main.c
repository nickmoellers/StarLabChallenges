#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <string.h>

#include <cpuid.h>
#define _OPEN_SYS_EXT 1
//#include <sys/ps.h>

static int fingerprint = -1;
//static struct timeb *timeStruct

void calcFingerprintFunction( void ) {
  char* fingerprint;
  char* fingerprintPtr;
  int size;

  int ret = -1;
  clock_t myclock;
  struct timeb mytime;

  ret = ftime( &mytime );
  myclock = clock();

  /*printf("mytime = {\n \
          \ttime_t\t%ld\n \
          \tmillitm\t%hu\n \
          \ttimezone\t%hd\n \
          \tdstflag\t%hd\n \
          }\n",
          mytime.time, 
          mytime.millitm,
          mytime.timezone,
          mytime.dstflag
          );

  //printf("sizeof(clocK)" %d, sizeof(clock));
  printf("clock = %ld\n", (long int)myclock);

  printf("\nsizes:\n");
  printf("mytime = {\n \
          \ttime_t\t%lu\n \
          \tmillitm\t%lu\n \
          \ttimezone\t%lu\n \
          \tdstflag\t%lu\n \
          }\n",
          sizeof(mytime.time), 
          sizeof(mytime.millitm),
          sizeof(mytime.timezone),
          sizeof(mytime.dstflag)
          );
  printf("clock = %lu\n", sizeof(myclock));
  printf("char = %lu\n", sizeof(char));*/

  size = 0;
  size+=sizeof(mytime.time)/sizeof(char); //2
  size+=sizeof(mytime.millitm)/sizeof(char); //8
  size+=sizeof(myclock)/sizeof(char); //8

  fingerprint = calloc( size, sizeof(char));
  fingerprintPtr = fingerprint;
  
  size = sizeof(mytime.time)/sizeof(char); 
  memcpy( fingerprintPtr, &mytime.time, size);
  fingerprintPtr+=size;

  size = sizeof(mytime.millitm)/sizeof(char); 
  memcpy( fingerprintPtr, &mytime.millitm, size);
  fingerprintPtr+=size;

  size = sizeof(myclock)/sizeof(char); 
  memcpy( fingerprintPtr, &myclock, size);
  fingerprintPtr+=size;
  
  /*
  printf("%016lx\t", myclock);
  printf("%04x\t", mytime.millitm);
  printf("%016lx\n", mytime.time);*/

  while( (void*) fingerprint <= (void*)  --fingerprintPtr ) {
    printf("%02x ", *fingerprintPtr & 0xff);
  }
  printf("\n");
  return;
}

int fingerprintfunction() {
	int fingerprint = 0;

	for( int leaf = 0 ; leaf <= 4 ; leaf++ ) {
		int eax = 3, ebx = 0, ecx = 0, edx = 0;
		if (__get_cpuid(leaf, &eax, &ebx, &ecx, &edx)) {
        	printf("leaf=%d, eax=0x%x, ebx=0x%x, ecx=0x%x, edx=0x%x\n",
            		leaf, eax, ebx, ecx, edx);
        }
	}
	return fingerprint;
}

int main()
{
  calcFingerprintFunction();
   // printf() displays the string inside quotation
   //unsigned int fingerprint = 0;
   //printf("Your fingerprint is %d", fingerprintfunction());
   return 0;
}

