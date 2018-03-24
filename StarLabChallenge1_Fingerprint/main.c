#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <string.h>
#include <inttypes.h>

//static uint32_t fingerprint = 0; 

void constructFingerPrint( uint32_t* fingerprint, uint32_t value, size_t numBits ) {
  uint32_t bitFilter = 1;
  bitFilter = (bitFilter<<numBits)-1;
  //all zeros then all ones
  bitFilter &= value;
  //all zeroes then the data from value

  //printf("bitFilter = 0x%08" PRIx32 "\n", bitFilter);

  *fingerprint=*fingerprint<<numBits;
  //previous data then all zeroes
  *fingerprint=*fingerprint | bitFilter;
  //previous data then the new data
  return;
}

//Creates and returns a 32 bit number consisting of the following:
//12 bits of CLOCK, then 10 bits of TIME, then 10 bits of MILLITIME
uint32_t calcFingerprintFunction( void ) {
  uint32_t fingerprint = 0; 
  
  clock_t myclock = clock();

  struct timeb mytime;
  ftime( &mytime );
  
  constructFingerPrint( &fingerprint, (unsigned int) myclock,        12 );
  constructFingerPrint( &fingerprint, (unsigned int) mytime.time,    10 );
  constructFingerPrint( &fingerprint, (unsigned int) mytime.millitm, 10 );
  
  //printf("%" PRIx32 "\n", fingerprint);

  return fingerprint;
}

static uint32_t fingerprintFunction( void ) {
  uint32_t fingerprint;
  //open a file
  //if it exists
    //ead the fingerprint from the file
    //return figerprint
  //else
    //calculated the fingerprint
    fingerprint = calcFingerprintFunction();
    //write the file
      //if successful
        return fingerprint;
      //else
      // error out
  //end
}

	/*int fingerprint = 0;

	for( int leaf = 0 ; leaf <= 4 ; leaf++ ) {
		int eax = 3, ebx = 0, ecx = 0, edx = 0;
		if (__get_cpuid(leaf, &eax, &ebx, &ecx, &edx)) {
        	printf("leaf=%d, eax=0x%x, ebx=0x%x, ecx=0x%x, edx=0x%x\n",
            		leaf, eax, ebx, ecx, edx);
        }
	}
	return fingerprint;
}*/

int main()
{
  uint32_t fingerprint = fingerprintFunction();
  printf("%" PRIx32 "\n", fingerprint);
  return 0;
}

