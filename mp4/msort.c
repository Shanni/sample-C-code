/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INTS 1024
#define CHAR_MEM 512

typedef struct _seg_t {

	int *start;
	int size;

} seg_t;

typedef struct _merge_seg_t {

	int *seg1;
	int  *seg2;
	int size1;
	int size2;


} merge_seg_t;

int compare(const void *a, const void *b) {

	return ( *(int*)a - *(int*)b );
}

void * merge(void * input) {
		
	merge_seg_t *m_seg = (merge_seg_t *)input;
	
	int *left = m_seg->seg1;
	int *right = m_seg->seg2;
	int *begin = left;

	int L_size = m_seg->size1;
	int R_size = m_seg->size2;
	int L_count = 0;
	int R_count = 0;
	int * temp = malloc(sizeof(int) *(L_size + R_size));
	
	
		
	//memcpy(temp, begin, (sizeof(int) *(L_size + R_size)));
	
	int dupes = 0;
	int j = 0;



	while (L_count < L_size && R_count < R_size) {

		if ( *left < *right ) {	
			
			temp[j] = *left;
			L_count++;
			left++;
	
		} else {
			
			if (*left == *right) dupes++;
			temp[j] = *right;
			R_count++;
			right++;

		}

		j++;

	}


	while (L_count < L_size)  {
		
		temp[j] = *left;
		L_count++;
		left++;
		j++;
	}

	while (R_count < R_size) {
		
		temp[j] = *right;
		R_count++;
		right++;
		j++;

	}

	/*for (j = 0; j < (L_size + R_size); j++) {
		
		printf("%i\n", temp[j]);
	}*/

	memcpy(begin, temp, (sizeof(int)* (L_size + R_size)));
	free(temp);
	
		

	printf("Merged %d and %d elements with %d duplicates\n", L_size, R_size, dupes);
	return NULL;	

}

void * sort(void * p) {
	
	qsort( ((seg_t *)p)->start, ((seg_t *)p)->size, sizeof(int), compare);
	fprintf(stderr, "Sorted %d elements. \n", ((seg_t *)p)->size);
	return NULL;

}

int main(int argc, char **argv) {

	if (argc < 2) return 0;

	char *line_ct;
	size_t size = CHAR_MEM;

	int segment_ct = atoi(argv[1]);
	int values_per_segment;

	char *line = malloc(CHAR_MEM);
	pthread_t *threads = malloc(sizeof(pthread_t) * segment_ct);
	seg_t *segments = malloc(sizeof(seg_t) * segment_ct);
	merge_seg_t *merge_segments = malloc(sizeof(merge_seg_t) * segment_ct);
	int *list = malloc(sizeof(int) * NUM_INTS);

	int input_ct = 0;

	for (;;) {
		
		line_ct = ( fgets(line, size, stdin));
		if (line_ct == NULL) break;
		list[input_ct] = atoi(line_ct);
		
		input_ct++;
		
		if (input_ct == NUM_INTS-1) {

			list = realloc(list, sizeof(int)*(NUM_INTS*2));
		}

	}
	
	if (input_ct%segment_ct == 0) {
		values_per_segment = input_ct/segment_ct;
	} else {
		values_per_segment = (input_ct/segment_ct) + 1;
	}

	int i;

	for (i = 0; i < segment_ct; i++) {	
		
		if (i == (segment_ct-1)) {

			segments[i].start = &list[0 + i*values_per_segment];
			segments[i].size = input_ct - (values_per_segment*i);
			
		} else {

			segments[i].start = &list[0 + i*values_per_segment];
			segments[i].size = values_per_segment;
		}
		
		pthread_create(&threads[i], NULL, sort, (void *)&segments[i]);
	}

	for (i = 0; i < segment_ct; i++) {

		pthread_join(threads[i], NULL);
	}

	int *end = segments[segment_ct-1].start;
	int round_ct = segment_ct;
	int max = segment_ct-1;
	int offset = 2;

	while (round_ct != 1) {
	
		round_ct /= 2;

		for (i = 0; i < round_ct; i++) {
			
			merge_segments[i].seg1 = list + i*values_per_segment*2;
			merge_segments[i].seg2 = list + i*values_per_segment*2 +values_per_segment;
			merge_segments[i].size1 = values_per_segment;

			if ( (2*values_per_segment)*(i+1) > input_ct ) {

				merge_segments[i].size2 = input_ct - (values_per_segment*(2*i+1));
			} else {
 
				merge_segments[i].size2 = values_per_segment;
			}

			pthread_create(&threads[i], NULL, merge, (void *)&merge_segments[i]);
			
		}

		for (i = 0; i < round_ct; i++) {
			pthread_join(threads[i], NULL);
		}

		if (round_ct%2 != 0 && round_ct !=1) {		
			
			merge_segments[0].seg1 = list + (values_per_segment*2*(round_ct-1));
			merge_segments[0].seg2 = end;
			merge_segments[0].size1 = values_per_segment*2;
			merge_segments[0].size2 = input_ct - (values_per_segment* round_ct*2);
			
			pthread_create(&threads[0], NULL, merge, (void *)&merge_segments[0]);
			pthread_join(threads[0], NULL);
		
			end = merge_segments[0].seg1;	
		}

		values_per_segment *= 2;
		
		

	}

	if (segment_ct %2 != 0) {

		merge_segments[0].seg1 = list;
		merge_segments[0].size1 = values_per_segment;
		merge_segments[0].seg2 = end;
		merge_segments[0].size2 = input_ct - values_per_segment;;
		

		pthread_create(&threads[0], NULL, merge, (void *)&merge_segments[0]);

		pthread_join(threads[0], NULL);

	}

	

	
	//printf("FUUUUUUCK\n");

	for (i = 0; i < input_ct; i++) {
		printf("%i\n", list[i]);
	}

	free(list);
	free(threads);
	free(segments);
	free(line);
	free(merge_segments);

	return 0;

}

