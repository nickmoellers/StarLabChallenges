#include <stdio.h>
#include <cpuid.h>
#define _OPEN_SYS_EXT 1
//#include <sys/ps.h>

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
   // printf() displays the string inside quotation
   unsigned int fingerprint = 0;
   printf("Your fingerprint is %d", fingerprintfunction());
   return 0;
}

