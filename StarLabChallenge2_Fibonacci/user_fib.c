#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static unsigned int * fibonacciArray;
static unsigned int fibonacciArrayLength=0;

void printIthFibonacci(unsigned int i ) {
	printf("fibonacci[%d] = ", i);
	if( fibonacciArray[i] == -1) {
		printf("OVERFLOW");
	} else {
		printf("%u", fibonacciArray[i]);
	}
	printf("\n");
}

void printFibonacci() {
	for( unsigned int i = 0 ; i < fibonacciArrayLength ; i ++ ) {
		printIthFibonacci( i );
	}
}

void freeFibonacci() {
	free(fibonacciArray);
	fibonacciArrayLength=0;
}

void calcFibonacci( unsigned int n ) {
	unsigned int fibonacciArrayLengthRequired = MAX(n+1, 2);

	//Initialize array
	if( fibonacciArrayLength < 2 ) { 
		fibonacciArray = (unsigned int*) malloc( fibonacciArrayLengthRequired * sizeof(int));
		fibonacciArray[0]=0;
		fibonacciArray[1]=1;
		fibonacciArrayLength=2;
	//OR Reinitialize Array
	} else if( fibonacciArrayLength < fibonacciArrayLengthRequired ) {
		fibonacciArray = (unsigned int*) realloc( fibonacciArray, fibonacciArrayLengthRequired*sizeof(int));
	}


	//Fill the array
	for( unsigned int i = fibonacciArrayLength; i < fibonacciArrayLengthRequired ; i ++ ) {

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

int main()
{
	printf("Sequentially calulating fibanacci numbers 1 through overflow:\n");
   	for( unsigned int n = 0 ; n <= 50 ; n++ ) {
   		unsigned int r = fibonacci( n );
   		printIthFibonacci(n); //printf("fibonacci[%u] = %u\n", n, r);
   	}
   	printf("Clear the memoized fibonacci numbers.\n");
   	freeFibonacci();
   	srand((unsigned int)time(NULL));
   	printf("Randomly test 10 fibanacci numbers:\n");
   	for( unsigned int n = 0 ; n < 10 ; n++ ) {
   		unsigned int randn = rand() % 50;
   		unsigned int r = fibonacci( randn );
   		printIthFibonacci(randn); //printf("fibonacci[%u] = %u\n", randn, r);

   	}
   	freeFibonacci();
   	return 0;
}