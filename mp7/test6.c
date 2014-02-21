/* 
 * CS 241
 * The University of Illinois
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "libmapreduce.h"

#define CHARS_PER_INPUT 30000
#define INPUTS_NEEDED 10

static const char *long_key = "long_key";

/* Takes input string and maps the longest
 * word to the key, long_key.
 */
void map(int fd, const char *data)
{
	int size = 128;

	char longest_word[size];

	while(1) {

		char *curr_word = malloc(size);
		int word_len = 0;

		const char *word_end = data;

		while(1) {

			if ( (word_len > 0 && (!isalpha(*word_end) || *word_end == ' ' || *word_end == '\n')) || *word_end == '\0' || word_len == size-1) {
				break;
			} else if (isalpha(*word_end) ) {
				curr_word[word_len++] = tolower(*word_end);	

			}

			word_end++;	 

		}

		if (*word_end == '\0') break;

		curr_word[word_len] = '\0';

		if (strlen(longest_word) < strlen(curr_word)) {
			
			strcpy(longest_word, curr_word);
		}
				

		data = word_end + 1;

	
	}

	char t[size*2];

	int the_length = snprintf(t, size*2, "%s: %s\n", long_key, longest_word);
	write(fd, t, the_length);

	close(fd);	 	
	


}

/* Takes two words and reduces to the longer of the two
 */
const char *reduce(const char *value1, const char *value2)
{

	int v1_len = strlen(value1);
	int v2_len = strlen(value2); 	

	char *result;

	if (v2_len > v1_len) {
		asprintf(&result, "%s", value2); 
	} else {
		
		asprintf(&result, "%s", value1);
	}

	return result;
}


int main()
{
	FILE *file = fopen("alice.txt", "r");
	char s[1024];
	int i;

	char **values = malloc(INPUTS_NEEDED * sizeof(char *));
	int values_cur = 0;
	
	values[0] = malloc(CHARS_PER_INPUT + 1);
	values[0][0] = '\0';

	while (fgets(s, 1024, file) != NULL)
	{
		if (strlen(values[values_cur]) + strlen(s) < CHARS_PER_INPUT)
			strcat(values[values_cur], s);
		else
		{
			values_cur++;
			values[values_cur] = malloc(CHARS_PER_INPUT + 1);
			values[values_cur][0] = '\0';
			strcat(values[values_cur], s);
		}
	}

	values_cur++;
	values[values_cur] = NULL;
	
	fclose(file);

	mapreduce_t mr;
	mapreduce_init(&mr, map, reduce);

	mapreduce_map_all(&mr, (const char **)values);
	mapreduce_reduce_all(&mr);
	
	const char *result_longest = mapreduce_get_value(&mr, long_key);

	if (result_longest == NULL) { printf("MapReduce returned (null).  The longest word was not found.\n"); }
	else { printf("Longest Word: %s\n", result_longest); free((void *)result_longest); }

	mapreduce_destroy(&mr);

	for (i = 0; i < values_cur; i++)
		free(values[i]);
	free(values);

	return 0;
}
