/** @file alloc.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


typedef struct _dictionary_entry_t {
	size_t len;	//length of memory for user
	struct _dictionary_entry_t *next;	//next in free list
	char mem[];	//points to memory for user

} dictionary_entry_t;


dictionary_entry_t *free_list[2];
//dictionary_entry_t *free_list = NULL;

#define PTR_TO_DICT(p) (&((dictionary_entry_t *)(p))[-1])


void *calloc(size_t num, size_t size)
{
	/* Note: This function is complete. You do not need to modify it. */
	void *ptr = malloc(num * size);
	
	if (ptr)
		memset(ptr, 0x00, num * size);

	return ptr;
}


void *malloc(size_t size)
{
	dictionary_entry_t **entry = &free_list[size%2];
	dictionary_entry_t **found = NULL;

	for(; *entry; entry = &(*entry)->next) {
		if((*entry)->len >= size) {
			if (!found || (*entry)->len < (*found)->len) {
				found = entry;
			}
		}

	}

	if (found) {
		void *mem = (*found)->mem;
		*found = (*found)->next;
		return mem;
	}

	dictionary_entry_t *new_mem = sbrk(sizeof(dictionary_entry_t) + size);
	new_mem->len = size;
	new_mem->next = NULL;
	
	return new_mem->mem;
	
}


void free(void *ptr)
{
	// "If a null pointer is passed as argument, no action occurs."
	if (!ptr) return;
	dictionary_entry_t *entry = PTR_TO_DICT(ptr);
	entry->next = free_list[(entry->len)%2];
	free_list[(entry->len)%2] = entry;

	return;
}


void *realloc(void *ptr, size_t size)
{
	 // "In case that ptr is NULL, the function behaves exactly as malloc()"
	if (!ptr)
		return malloc(size);

	 // "In case that the size is 0, the memory previously allocated in ptr
	 //  is deallocated as if a call to free() was made, and a NULL pointer
	 //  is returned."
	if (!size)
	{
		free(ptr);
		return NULL;
	}

	dictionary_entry_t *entry = PTR_TO_DICT(ptr);
	
	void *new_mem = malloc(size);
	size_t copy_bytes = entry->len < size ? entry->len : size;

	memcpy(new_mem, ptr, copy_bytes);
	free(ptr);

	
   	return new_mem;


	
}
