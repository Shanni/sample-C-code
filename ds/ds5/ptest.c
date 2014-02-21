/**
 * Multi-threaded prime number test
 *
 * Copyright (C) 2013  Your Name <your_netid@illinois.edu> 
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _ptest_t {
	int start;
	int end;

	int prime;

} ptest_t;

#define IS_PRIME 0

/**
 * Tests if a value is prime by dividing the canidate prime by
 * every number in the inclusive range: [start, end].
 * 
 * @param ptr
 *    A pointer to the ptest_t data structure.
 *
 * @return
 *    A pointer to an integer type storing
 *    either the number that the prime canidate is divisible
 *    by or 0 if the prime canidate is not divisible by
 *    any number in the range [start, end].
 */
void *prime_tester(void *ptr)
{
	ptest_t *test = ptr;
	int *result = malloc(sizeof(int));

	int i;
	for (i = test->start; i <= test->end; i++) {
		if (test->prime % i == 0) {
			*result = i;
			return result;
		}		

	}
	*result = IS_PRIME;
  	return result;
}

int main(int argc, char **argv)
{	
	if (argc < 3) {
		fprintf(stderr, "usage: %s [# threads] [prime]\n", argv[0]);
		return 1;
	
	}

	int n = atoi(argv[1]);
	int prime = atoi(argv[2]);

	int max = 1;
	while (max * max <= prime) max++;
	max--;
	
	if (max < 2) {
		printf("Found Prime: %i\n", prime);
		return 0;
	}

	if (n > max - 1) n = max - 1;

	int testsPerThread = (max - 1) / n;

	pthread_t *thread = malloc(sizeof(pthread_t) * n);
	ptest_t *tests = malloc(sizeof(ptest_t) *n);

	int i;
	for (i = 0; i < n; i++) {
		tests[i].start = i * testsPerThread + 2;
		tests[i].end = (i + 1) * testsPerThread + 1;
		tests[i].prime = prime;

		if (i == n - 1) tests[i].end = max;
		
		pthread_create(&thread[i], NULL, prime_tester, &tests[i]);
		
	}

	int *ret, found = 1;
	
	for (i = 0; i < n; i++) {
		pthread_join(thread[i], (void **)&ret);

		if (*ret != IS_PRIME) {
			printf("Not a prime (%i divides %i)\n", *ret, prime);
			found = 0;
			free(ret);
			break;
		}

		free(ret);
	}

	while (i < n -1) {
		pthread_join(thread[++i], (void **)&ret);
		free(ret);
	
	}

	free(tests);
	free(thread);
	
	if (found) printf("Found Prime: %i\n", prime);


	return 0;
}

