/** @file parmake.c */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include "parser.h"
#include "queue.h"
#include "rule.h"

/**
 * Entry point to parmake.
 */


int num_rules = 0;

queue_t rule_q;
queue_t run_q;
queue_t finished_q;

pthread_mutex_t rule;
pthread_mutex_t run;
pthread_mutex_t finished;




void parsed_new_target(char * target)
{
	rule_t *new_rule = malloc(sizeof(rule_t));
	rule_init(new_rule);
	new_rule->target = strdup(target);
	queue_enqueue(&rule_q, (void *)new_rule);


}

void parsed_new_dependency(char * target, char * dependency)
{
	char *d = strdup(dependency);
	
	rule_t * temp = (rule_t *)( queue_at(&rule_q, queue_size(&rule_q)-1 ));	
	queue_enqueue( temp->deps, ((void *)d) );


}

void parsed_new_command(char * target, char * command)
{
	char *c = strdup(command);	

	rule_t * temp = (rule_t *)( queue_at(&rule_q, queue_size(&rule_q)-1 ));
	queue_enqueue( temp->commands, ((void *)c) );
 
}

void printdep(void *a, void *b)
{	
	printf("dependency: %s\n", ((char *)a) );
}

void printcom(void *a, void *b)
{
	printf("command: %s\n", ((char *)a) );

}


void print(void *a, void *b)
{
	printf("rule: %s\n", ((rule_t *)a)->target);
	queue_t *d = ((rule_t *)a)->deps;
	queue_iterate(d, printdep, NULL);
	queue_t *c = ((rule_t *)a)->commands;
	queue_iterate(c, printcom, NULL);		
	printf("\n");	
	
}


void free_coms(void *a, void *b)
{
	free( ((char *)a) );

}

void free_deps(void *a, void *b)
{
	free( ((char *)a));

}

void clean_up(void *a, void *b)
{
	queue_t *d = ((rule_t *)a)->deps;
	queue_iterate(d, free_deps, NULL);
	queue_t *c = ((rule_t *)a)->commands;
	queue_iterate(c, free_coms, NULL); 

	queue_destroy(d);
	queue_destroy(c);
	
	rule_destroy((rule_t *)a);
}

void run_commands(void *a, void *b)
{
	//printf("%s\n", (char *)a );
	if ( system( (char *)a ) != 0) exit(1);

}

void run_rule(void *a, void *b)
{
	rule_t * temp = (rule_t *)a;
	queue_t *c = temp->commands;
	queue_iterate(c, run_commands, NULL);
	
	queue_enqueue(&finished_q, (void *)temp);
	//pthread_cond_broadcast(&var);
}

int is_file(const char * target)
{
	return access(target, F_OK);
}

int is_rule(const char * target)
{
	int flag = 0;
	
	int i;

	for (i = 0; i < finished_q.size; i++) {
		
		if ( strcmp(((rule_t *)(queue_at(&finished_q, i)))->target, target) == 0) {
			
			flag = 1;
			return flag;
		}
	
	}

	for (i = 0; i < rule_q.size; i++) {

		if (strcmp(((rule_t *)(queue_at(&rule_q, i)))->target, target) == 0) {
			
			flag = 1;
			return flag;

		}
	
	}

	return flag;

}

//Takes a string as a parameter (from a rule's dependency queue)
//Iterates through the finished_q, compares input target to the target field
//of every rule in the finished_q
int is_dep_finished(const char * target)
{
	int flag = 0;
	
	int i;

	for (i = 0; i < finished_q.size; i++) {
		
		if ( strcmp(((rule_t *)(queue_at(&finished_q, i)))->target, target) == 0) {

			flag = 1;
			return flag;
		}
	
	}


	return flag;

}



int all_deps_files(rule_t * r)
{	
	queue_t *d = r->deps;
	
	int flag = 1;

	int i;

	for (i = 0; i < d->size; i++) {

		if ( access((char *)(queue_at(d, i)), F_OK) != 0 ) flag = 0;	

	}

	
	//printf("%s: %d\n", r->target, flag);

	return flag;

}

int can_run(rule_t * r)
{
	struct stat my_rule;
	stat( (r->target), &my_rule);
	struct stat my_dep;
	int sat_count = 0;

	queue_t *d = r->deps;

	
	if (all_deps_files(r) == 1) {
		
		if (access(r->target, F_OK) != -1) {	
			int j;
			for (j = 0; j < d->size; j++) {
				stat( (char *)(queue_at(d,j)), &my_dep);
	

				if ( difftime(my_rule.st_mtime, my_dep.st_mtime) <= 0) {
					return 1;
				}
				
			}
			return 2;
		}



		return 1;
		
	}

	int i;

	for (i = 0; i < d->size; i++) {

		stat( (char *)(queue_at(d, i)), &my_dep);

		
		if (is_rule((char *)(queue_at(d,i))) == 1) {
			if (is_dep_finished((char *)(queue_at(d,i))) == 1) {
				sat_count++;
				continue;
			}
			
		} else {
			if (is_file((char *)(queue_at(d,i))) == 0) {
				if (is_file(r->target) == 0) {
					if ( difftime(my_rule.st_mtime, my_dep.st_mtime) > 0) {

						sat_count++;
						continue;
					}	
				}
			}
		}


		if (is_rule((char *)(queue_at(d,i))) == 0) {
			sat_count++;
		}


	}

	if (sat_count == d->size) {
 		return 1;
	} else {
		return 0;
	}

}

void * main_loop(void *a)
{

/*	while( cond ) // protect
 *
 *	lock( queue )
 *	if( queuesize == 0 )
 *		return
 *	var
 *	for( i )
 *		if( can run == 1 )
 *			var true;
 *	unlock( queue )
 *
 *	if( rule != ture )
 *		cond-wait - 
 *	else
 *		run
 *		cond-broadcast -
 *
 */


	while (finished_q.size < num_rules) {

			
		int i;

		//pthread_mutex_lock(&rule);
		//pthread_mutex_lock(&run);
	
		for (i = 0; i < rule_q.size; i++) {	
			
			if ( can_run( (rule_t *)(queue_at(&rule_q, i))) == 1 ) {
				
				queue_enqueue(&run_q, queue_remove_at(&rule_q, i));
				i--;
				continue;
			}

			if (can_run( (rule_t *)(queue_at(&rule_q, i))) == 2) {

				queue_enqueue(&finished_q, queue_remove_at(&rule_q, i));
				i--;
				continue;
			}

		}
		
		//pthread_mutex_unlock(&run);
		//pthread_mutex_unlock(&rule);

		//pthread_mutex_lock(&run);
		//
		//if (run_q.size == 0) pthread_cond_wait(&var);

		queue_iterate(&run_q, run_rule, NULL);
			
		//pthread_mutex_unlock(&run);

		while (run_q.size != 0) {
			
			queue_dequeue(&run_q);

		}
	
	
		
	
	}

	return NULL;

}

int main(int argc, char **argv)
{
	queue_init(&rule_q);
	queue_init(&run_q);
	queue_init(&finished_q);

	pthread_mutex_init(&rule, NULL);
	pthread_mutex_init(&run, NULL);
	pthread_mutex_init(&finished, NULL);

		
	int option;
	char *file = NULL;
	int num_threads = 1;

	char **targets = NULL;



	while ((option = getopt(argc, argv, "f:j:")) != -1) {
	
		switch(option) {
		
			case 'f':
				file = optarg;	
				break;
			case 'j':
				num_threads = atoi(optarg);
				break;
	
		}

	}

	

	if (optind < argc) {

		targets = malloc(sizeof(char *) *(argc-optind));

		int j;

		for (j = optind; j < argc; j++) {

			targets[j-optind] = argv[j];		
		
		}


	}

		
	if (file == NULL) {

		if (access("Makefile", F_OK) == 0) {
			file = "Makefile";
		}

		if ( access("makefile", F_OK) == 0) {
			file = "makefile";
		}


		if (file == NULL) return -1;

	}


	if ( access(file, F_OK) == -1) {
		
		printf("%s: No such file or directory", file);
		return -1;

	}

	pthread_t threads[num_threads];

	parser_parse_makefile(file, targets, parsed_new_target, parsed_new_dependency, parsed_new_command);

	//printf("rule_q:\n");
	//queue_iterate(&rule_q, print, NULL);

	num_rules = rule_q.size;
	
	int j;
	for (j = 0; j < num_threads; j++) {

		pthread_create(&threads[j], NULL, main_loop, NULL);


	}	

	for (j = 0; j < num_threads; j++) {

		pthread_join(threads[j], NULL);

	}



	//printf("rule_q:\n");
	//queue_iterate(&rule_q, print, NULL);
	/*printf("run_q:\n");
	queue_iterate(&run_q, print, NULL);
	printf("finished_q:\n");
	queue_iterate(&finished_q, print, NULL);*/
	
	//queue_destroy(&rule_q);
	//queue_destroy(&run_q);

	queue_iterate(&rule_q, clean_up, NULL);
	queue_iterate(&run_q, clean_up, NULL);
	queue_iterate(&finished_q, clean_up, NULL);

	//queue_destroy(&rule_q);
	//queue_destroy(&run_q);
	//queue_destroy(&finished_q);

	//free(targets);
	
	return 0; 
}



