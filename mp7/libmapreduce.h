/** @file libmapreduce.h */
/* 
 * CS 241
 * The University of Illinois
 */
 
#ifndef _LIBMAPREDUCE_H_
#define _LIBMAPREDUCE_H_

#include <pthread.h>
#include "libds/libds.h"

typedef struct _mapreduce_t
{
	// You will need to add some stuff here! :)

	pthread_t worker;		
	datastore_t DS;

	int ep_fd;
	struct epoll_event *ev;
	int **fds;
	char **buffs;

	int count;	

	void (*my_map)(int, const char *);
	const char *(*my_reduce)(const char *, const char *);		

		
} mapreduce_t;


/**
 * This function will be the first call made to the libmapreduce library. 
 * You should put any initialization logic here
 * @param mymap map function
 * @param myreduce reduce function
 * @return Void
 */
void mapreduce_init(mapreduce_t *mr, void (*mymap)(int, const char *), const char *(*myreduce)(const char *, const char *));


/**
 * Main function of this MP.
 * @param values contains a NULL-terminated array of C-strings.
 * @return Void
 */
void mapreduce_map_all(mapreduce_t *mr, const char **values);


/**
 * This function should block until all map()'ing and reduce()'ing has completed.
 */
void mapreduce_reduce_all(mapreduce_t *mr);


/**
 * This function should return the current value of result_key. If the result_key does not exist, return NULL. 
 */
const char *mapreduce_get_value(mapreduce_t *mr, const char *result_key);


/**
* Free all memory. Will always be called last.
*/
void mapreduce_destroy(mapreduce_t *mr);

#endif
