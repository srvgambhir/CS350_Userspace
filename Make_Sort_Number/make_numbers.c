#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>


long mod(long x, long y) {
	long ret;
	if (x < 0 && y != 0) {
		ret = (x % y + y) % y;
	}
	else if (y == 0) {
		ret = 0;
	}
	else {
		ret = x % y;
	}
	return ret;
}

int main(int argc, char** argv) 
{
	long n = atol(argv[1]);
	long lo = atol(argv[2]);
	long hi = atol(argv[3]);

	if (n < 0 || n >= INT_MAX) {
		return 1;
	}
	if (lo < INT_MIN || lo > INT_MAX){
		return 1;
	}
	if (hi < INT_MIN || hi > INT_MAX) {
		return 1;
	}
	if (lo > hi) {
		return 1;
	}

	FILE *fptr;
	fptr = fopen("log.txt", "w");
	
	fprintf(fptr, "%ld\n", n);
	
	srand(getpid());
	long gen;
	for (int i = 1; i <= n; ++i) {
		gen = rand() + rand();
		gen = lo + mod(gen, hi - lo + 1);
		fprintf(fptr, "%ld\n", gen);
	}

	return 0;
}
