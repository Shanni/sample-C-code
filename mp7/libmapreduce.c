/** @file libmapreduce.c */
/* 
 * CS 241
 * The University of Illinois
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#include "libmapreduce.h"
#include "libds/libds.h"


static const int BUFFER_SIZE = 2048;  /**< Size of the buffer used by read_from_fd(). */
//int fds[count][2] = NULL;

		


/**
 * Adds the key-value pair to the mapreduce data structure.  This may
 * require a reduce() operation.
 *
 * @param key
 *    The key of the key-value pair.  The key has been malloc()'d by
 *    read_from_fd() and must be free()'d by you at some point.
 * @param value
 *    The value of the key-value pair.  The value has been malloc()'d
 *    by read_from_fd() and must be free()'d by you at some point.
 * @param mr
 *    The pass-through mapreduce data structure (from read_from_fd()).
 */
static void process_key_value(const char *key, const char *value, mapreduce_t *mr)
{

	unsigned long ret = datastore_put(&(mr->DS), key, value);
	
	unsigned long rev;

	if (ret == 0) {
		
		const char * val = datastore_get(&(mr->DS), key, &rev);
		const char * V = mr->my_reduce(value, val);
		datastore_update(&(mr->DS), key, V, rev);

		free(val);
		free(V);
	}

	free(key);
	free(value);	

}


/**
 * Helper function.  Reads up to BUFFER_SIZE from a file descriptor into a
 * buffer and calls process_key_value() when for each and every key-value
 * pair that is read from the file descriptor.
 *
 * Each key-value must be in a "Key: Value" format, identical to MP1, and
 * each pair must be terminated by a newline ('\n').
 *
 * Each unique file descriptor must have a unique buffer and the buffer
 * must be of size (BUFFER_SIZE + 1).  Therefore, if you have two
 * unique file descriptors, you must have two buffers that each have
 * been malloc()'d to size (BUFFER_SIZE + 1).
 *
 * Note that read_from_fd() makes a read() call and will block if the
 * fd does not have data ready to be read.  This function is complete
 * and does not need to be modified as part of this MP.
 *
 * @param fd
 *    File descriptor to read from.
 * @param buffer
 *    A unique buffer associated with the fd.  This buffer may have
 *    a partial key-value pair between calls to read_from_fd() and
 *    must not be modified outside the context of read_from_fd().
 * @param mr
 *    Pass-through mapreduce_t structure (to process_key_value()).
 *
 * @retval 1
 *    Data was available and was read successfully.
 * @retval 0
 *    The file descriptor fd has been closed, no more data to read.
 * @retval -1
 *    The call to read() produced an error.
 */
static int read_from_fd(int fd, char *buffer, mapreduce_t *mr)
{
	/* Find the end of the string. */
	int offset = strlen(buffer);

	/* Read bytes from the underlying stream. */
	int bytes_read = read(fd, buffer + offset, BUFFER_SIZE - offset);
	if (bytes_read == 0)
		return 0;
	else if(bytes_read < 0)
	{
		fprintf(stderr, "error in read.\n");
		return -1;
	}

	buffer[offset + bytes_read] = '\0';

	/* Loop through each "key: value\n" line from the fd. */
	char *line;
	while ((line = strstr(buffer, "\n")) != NULL)
	{
		*line = '\0';

		/* Find the key/value split. */
		char *split = strstr(buffer, ": ");
		if (split == NULL)
			continue;

		/* Allocate and assign memory */
		char *key = malloc((split - buffer + 1) * sizeof(char));
		char *value = malloc((strlen(split) - 2 + 1) * sizeof(char));

		strncpy(key, buffer, split - buffer);
		key[split - buffer] = '\0';

		strcpy(value, split + 2);

		/* Process the key/value. */
		process_key_value(key, value, mr);

		/* Shift the contents of the buffer to remove the space used by the processed line. */
		memmove(buffer, line + 1, BUFFER_SIZE - ((line + 1) - buffer));
		buffer[BUFFER_SIZE - ((line + 1) - buffer)] = '\0';
	}

	return 1;
}


/**
 * Initialize the mapreduce data structure, given a map and a reduce
 * function pointer.
 */
void mapreduce_init(mapreduce_t *mr, 
                    void (*mymap)(int, const char *), 
                    const char *(*myreduce)(const char *, const char *))
{
	
	mr->my_map = mymap;
	mr->my_reduce = myreduce;
	datastore_init(&(mr->DS));
	
}


void * work(void *a)
{
	mapreduce_t *mr = (mapreduce_t *)a;
	int counter = mr->count;
	while (counter != 0) {

		struct epoll_event curr_ev;
		
		epoll_wait(mr->ep_fd, &curr_ev, 1, -1); 

		int index;		

		int i;
		for (i = 0; i < mr->count; i++) {
		
			if (curr_ev.data.fd == mr->ev[i].data.fd) {
				
				index = i;
			}
		}

			
		ssize_t bytes = read_from_fd(curr_ev.data.fd, mr->buffs[index], mr);
		
		if (bytes == 0) {

			counter--;
			epoll_ctl(mr->ep_fd, EPOLL_CTL_DEL, curr_ev.data.fd, NULL);	
			
		}

			
	}	
		

}

/**
 * Starts the map() processes for each value in the values array.
 * (See the MP description for full details.)
 */
void mapreduce_map_all(mapreduce_t *mr, const char **values)
{
	int i = 0;
	int count = 0;

	while (values[i] != NULL) {	
		count++;
		i++;
	}
	mr->count = count;

	(mr->buffs) = malloc(sizeof(char *)*count);
	
	mr->ep_fd = epoll_create(count);

	(mr->ev) = malloc(sizeof(struct epoll_event) *count);

	(mr->fds) = malloc(sizeof(int *)*count);

	for (i = 0; i < count; i++) {
		mr->buffs[i] = malloc(BUFFER_SIZE+1);
		mr->buffs[i][0] = '\0';
		mr->fds[i] = malloc(sizeof(int)*2);
		pipe(mr->fds[i]);
	}


	for (i = 0; i < count; i++) {

		int read_fd = mr->fds[i][0];
		int write_fd = mr->fds[i][1];
	
		pid_t pid = fork();
		//printf("pid: %i\n", pid);
		
		if (pid == 0) {
			
			int k;
			for (k = 0; k < count; k++) {
				/*close read file descriptors in child process*/
				close(mr->fds[k][0]); 
			}

			mr->my_map(write_fd, values[i]);
	
			exit(0);

		} else if (pid > 0) {

			close(write_fd);	
		}

		mr->ev[i].events = EPOLLIN;
		mr->ev[i].data.fd = read_fd;
		epoll_ctl(mr->ep_fd, EPOLL_CTL_ADD, read_fd, &(mr->ev[i]));

	
	}
	

	pthread_create(&(mr->worker), NULL, work, (void *)mr);
	


}



/**
 * Blocks until all the reduce() operations have been completed.
 * (See the MP description for full details.)
 */
void mapreduce_reduce_all(mapreduce_t *mr)
{

	pthread_join(mr->worker, NULL);

}


/**
 * Gets the current value for a key.
 * (See the MP description for full details.)
 */
const char *mapreduce_get_value(mapreduce_t *mr, const char *result_key)
{	
	return datastore_get( &(mr->DS), result_key, 0); 
}


/**
 * Destroys the mapreduce data structure.
 */
void mapreduce_destroy(mapreduce_t *mr)
{

	free(mr->ev);

	int i;

	for (i = 0; i < mr->count; i++) {

		free(mr->buffs[i]);
		free(mr->fds[i]);
	
	}

	free(mr->buffs);
	free(mr->fds);
	
	datastore_destroy(&(mr->DS));


}
