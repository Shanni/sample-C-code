/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->size = 0;	
	q->comp = comparer;
	q->head = NULL;
		

		
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	pri_node_t * new_node = malloc(sizeof(pri_node_t));
	new_node->data = ptr;
	new_node->next = NULL;
	new_node->prev = NULL;
	
	pri_node_t * curr = q->head;
	int index = 0;
		

	if (curr == NULL) {
		q->head = new_node;
		q->size++;
		return 0;

	}


	while (curr != NULL) {

		if (q->comp(ptr, curr->data) <= 0 && curr == q->head) {

			new_node->next = curr;
			curr->prev = new_node;			
			q->head = new_node;
			q->size++;
			return 0;		
		}

		if (q->comp(ptr, curr->data) <= 0 && curr != q->head) {
			
			new_node->next = curr;
			curr->prev->next = new_node;
			new_node->prev = curr->prev;
			curr->prev = new_node;
			q->size++;	
			return index;	
		}

	    	if (curr->next == NULL) {
            
            		curr->next = new_node;
            		new_node->prev = curr;
           		new_node->next = NULL;
            		q->size++;
            		return index;

        	}
	
		index++;
		curr = curr->next;
	}	
	
		
				

	return 0;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{

	if (q->head == NULL || q->size == 0) {
		return NULL;

	} else {

		return q->head->data;
	}

	
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	
	if (q->head == NULL || q->size == 0) return NULL;

	

	pri_node_t * temp = q->head;
	void * temp_data = temp->data;
	q->head = temp->next;


	if (q->head != NULL) q->head->prev = NULL;
	temp->next = NULL;
	free(temp);
	
	q->size--;

	return temp_data;	


}


/**
  Returns the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{

	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	
	pri_node_t * curr = q->head;
	
	pri_node_t * temp = NULL;


	while (curr != NULL) {

		temp = curr->next;

		free(curr);
		curr = temp;

	}



}




