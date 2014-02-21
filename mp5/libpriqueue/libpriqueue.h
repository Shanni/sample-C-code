/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
	int size;
	int (*comp)(const void *, const void *); 		
	struct _pri_node_t * head;
		
   
} priqueue_t;


typedef struct _pri_node_t
{

	void * data;	
	struct _pri_node_t *next;
	struct _pri_node_t *prev;

	
} pri_node_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
