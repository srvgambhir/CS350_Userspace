#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int cmp (const void *p, const void *q) {
	int a = *(int *) p;
	int b = *(int *) q;

	if (a > b) return 1;
	if (a < b) return -1;
	return 0;
}

int main(int argc, char** argv) {
	int n;

	FILE *fread;
	fread = fopen("log.txt", "r");

	FILE *fwrite;
	fwrite = fopen("sorted.txt", "w");

	fscanf(fread, "%d\n", &n);

	fprintf(fwrite, "%d\n", n);
	
	int arr[n];

	for (int i = 0; i < n; ++i) {
		fscanf(fread, "%d\n", (arr + i));
	}

	struct timeval tv1;
	struct timeval tv2;
	
	gettimeofday(&tv1, NULL);
	
	qsort(arr, n, sizeof(int), cmp);
	gettimeofday(&tv2, NULL);

	long runtime = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
	printf("%ld\n", runtime);
	
	for (int i = 0; i < n; ++i) {
		fprintf(fwrite, "%d\n", arr[i]);
	}

	return 0;
}
