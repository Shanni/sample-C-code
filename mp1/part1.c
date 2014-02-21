/** @file part1.c */

/*
 * Machine Problem #1
 * CS 241 Fall2013
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mp1-functions.h"

/**
 * (Edit this function to print out the ten "Illinois" lines in mp1-functions.c in order.)
 */
int main()
{
	first_step(81);	
	
	int *c = malloc(sizeof(int));
	*c = 132; 
	second_step(c);	
	free(c);
		
	
	int **d = malloc(sizeof(int*));
	d[0] = malloc(sizeof(int));
	*d[0] = 8942; 
	double_step(d);
	free(d[0]);
	free(d);	


	int *e = NULL;
	strange_step(e);	

	char *f = malloc(sizeof(char) * 4);
	f[3] = 0;
	empty_step(f); 	
	free(f);	


	void *g;
	char* h = malloc(sizeof(char) * 4);
	g = h;
	h[3] = 'u';
	two_step(g, h);	 
	free(h);


	char *m = malloc(sizeof(char) * 5);	
	char *first = m;
	char *second = m + 2;
	char *third = m + 4;
	three_step(first, second, third);		
	free(m);	


	char *one = malloc(sizeof(char) *2);
	char *two = malloc(sizeof(char) *3);
	char *three = malloc(sizeof(char) *4);
	one[1] = 0;
	two[2] = one[1] + 8;		
	three[3] = two[2] + 8;
	step_step_step(one, two, three);
	free(one);
	free(two);
	free(three);

	char *x = malloc(sizeof(char));
	*x = 'a';
	int y = (int)(*x);
	it_may_be_odd(x, y);
	free(x);		 


	char *orange = malloc(sizeof(char) * 4);
	orange[0] = 1;
	orange[1] = 0;
	orange[2] = 0;
	orange[3] = 2;
	the_end(orange, orange);
	free(orange);	
	
	/*int orange = 16777218;
	printf("%d\n", k);
	int* p = malloc(sizeof(int));
	p = &orange;
	printf("%d\n",((char*)p)[0]);
	the_end(&orange, &orange);*/			
	
	return 0;
}
