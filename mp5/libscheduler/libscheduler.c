/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/



//START GLOBALS and STRUCTS
//
//
//
priqueue_t job_queue;
priqueue_t finished;
int num_cores;
scheme_t the_scheme;
int job_count = 0;


typedef struct _job_t
{
	int priority;
	int job_number;
	int remaining_time;
	int running_time;

	int arrival_time;
	int first_runtime;
	int last_runtime;

	int waiting_time;
	int turnaround_time;
	int response_time;

} job_t;

typedef struct _core_t
{
	job_t *core_job;		

} core_t;

core_t * core_arr;
//
//
//
//END GLOBALS and STRUCTS



//START COMPARE FUNCTIONS
//
//
//comp_FCFS is used for FCFS
int comp_FCFS(const void *a, const void *b) {

	return ( ((job_t*)a)->arrival_time - ((job_t*)b)->arrival_time );	

}

//comp_RR is used for RR
int comp_RR(const void *a, const void *b) {
	
	return 1;

}

//comp_SJF is used for SJF and PSJF
int comp_SJF(const void *a, const void *b) {
	
	

	if ( (((job_t*)a)->remaining_time - ((job_t*)b)->remaining_time ) != 0) {

		return ( ((job_t*)a)->remaining_time - ((job_t*)b)->remaining_time );

	}

	
	return ( ((job_t*)a)->arrival_time - ((job_t*)b)->arrival_time );	


}


//comp_PRI is used for PRI and PPRI
int comp_PRI(const void *a, const void *b) {


	 if ( (((job_t*)a)->priority - ((job_t*)b)->priority ) != 0) {

		return ( ((job_t*)a)->priority - ((job_t*)b)->priority );

	}

	
	return ( ((job_t*)a)->arrival_time - ((job_t*)b)->arrival_time );	


}

//
//
//
//END COMPARE FUNCTIONS

	
	



/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
	
	num_cores = cores;
	core_arr = malloc(sizeof(core_t) *cores);

	int i;

	for (i = 0; i < cores; i++) {
		
		core_arr[i].core_job = NULL;	

	}


	the_scheme = scheme;

	if (scheme == FCFS) {
		priqueue_init(&finished, comp_RR);	
		priqueue_init(&job_queue, comp_FCFS);
	}

	if (scheme == RR) {
		priqueue_init(&finished, comp_RR);
		priqueue_init(&job_queue, comp_RR);

	}

	if (scheme == SJF || scheme == PSJF) {
		priqueue_init(&finished, comp_RR);
		priqueue_init(&job_queue, comp_SJF);
	}

	if (scheme == PRI || scheme == PPRI) {
		priqueue_init(&finished, comp_RR);
		priqueue_init(&job_queue, comp_PRI);
	}

}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{

	job_t * new_job = malloc(sizeof(job_t));
	
	
	new_job->job_number = job_number;
	new_job->priority = priority;
	new_job->remaining_time = running_time;
	new_job->running_time = running_time;


	new_job->arrival_time = time;
	new_job->first_runtime = -1;
	new_job->last_runtime = -1;
	
	job_count++;
	
	job_t *worst = NULL;
	int worst_index = -1;

	
	int i;

	for (i = 0; i < num_cores; i++) {

		if (core_arr[i].core_job != NULL) {
			
			core_arr[i].core_job->remaining_time = core_arr[i].core_job->remaining_time - (time - core_arr[i].core_job->last_runtime);
			core_arr[i].core_job->last_runtime = time;

		}

	}

	
	for (i = 0; i < num_cores; i++) {
	
		if (core_arr[i].core_job == NULL) {
			new_job->first_runtime = time;
			new_job->last_runtime = time;
			core_arr[i].core_job = new_job;
			return i;
		}

	}

	if (the_scheme == FCFS || the_scheme == SJF || the_scheme == PRI) {
			
		priqueue_offer(&job_queue, new_job);
		return -1;	

	}

	if (the_scheme == RR) {
		
		priqueue_offer(&job_queue, new_job);
		return -1;

	}
	
	if (the_scheme == PSJF || the_scheme == PPRI) {
		
		for (i = 0; i < num_cores; i++) {
			
			if (worst != NULL && job_queue.comp(worst, core_arr[i].core_job) < 0) {
									
				worst = core_arr[i].core_job;
				worst_index = i;
			}

			if (worst == NULL && job_queue.comp(new_job, core_arr[i].core_job) < 0) {

				worst = core_arr[i].core_job;
				worst_index = i;
			}

		}
		

		if (worst != NULL) {
			
			if (worst->first_runtime == time) {
				worst->first_runtime = -1;
			}

			if (worst->last_runtime == time) {
				worst->last_runtime = -1;
			}
			
			
			new_job->first_runtime = time;
			new_job->last_runtime = time;	
			core_arr[worst_index].core_job = new_job;
			priqueue_offer(&job_queue, worst);
			return worst_index;

		}
		
		if (worst == NULL) {

			priqueue_offer(&job_queue, new_job);
			return -1;		

		}


	}



	return -1;

}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{

	core_arr[core_id].core_job->waiting_time = (time - core_arr[core_id].core_job->arrival_time) - core_arr[core_id].core_job->running_time;
	
	core_arr[core_id].core_job->response_time = core_arr[core_id].core_job->first_runtime - core_arr[core_id].core_job->arrival_time;

	core_arr[core_id].core_job->turnaround_time = time - core_arr[core_id].core_job->arrival_time;

		
		
	priqueue_offer(&finished, core_arr[core_id].core_job);

	core_arr[core_id].core_job = priqueue_poll(&job_queue);
			
	

	if (core_arr[core_id].core_job != NULL) {
		
		if ( core_arr[core_id].core_job->first_runtime == -1) {
			core_arr[core_id].core_job->first_runtime = time;
		}

		core_arr[core_id].core_job->last_runtime = time;
			
		return core_arr[core_id].core_job->job_number;

	}	

	return -1;



				
			
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	
	priqueue_offer(&job_queue, core_arr[core_id].core_job);

	core_arr[core_id].core_job = priqueue_poll(&job_queue);	

	if (core_arr[core_id].core_job != NULL) {

		if (core_arr[core_id].core_job->first_runtime == -1) {
			core_arr[core_id].core_job->first_runtime = time;
		}

		core_arr[core_id].core_job->last_runtime = time;	
			
		return core_arr[core_id].core_job->job_number;
	}
	

	return -1;

}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{

	float total_waiting = 0.0;

	pri_node_t *curr = finished.head;

	while (curr != NULL) {

		total_waiting += (float)(((job_t*)(curr->data))->waiting_time);

		curr = curr->next;

	}


	return total_waiting/((float)job_count);
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	
	float total_turnaround = 0.0;

	pri_node_t *curr = finished.head;

	while (curr != NULL) {

		total_turnaround += (float)(((job_t*)(curr->data))->turnaround_time);

		curr = curr->next;
	
	}

	return total_turnaround/((float)job_count);
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	
	float total_response = 0.0;

	pri_node_t *curr = finished.head;

	while (curr != NULL) {
		
		total_response += (float)(((job_t*)(curr->data))->response_time);

		curr = curr->next;

	}
	
	return total_response/((float)job_count);
}


/**
  Free any memory associated with your scheduler.
 
  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	

	int i;

	for (i = 0; i < num_cores; i++) {

		if (core_arr[i].core_job != NULL) {

			free(core_arr[i].core_job);
		}

	}

	free(core_arr);

	pri_node_t *curr = job_queue.head;

	while (curr != NULL) {

		if (curr->data != NULL) {

			free(curr->data);

		}

		curr = curr->next;

	}

	curr = finished.head;

	while (curr != NULL) {

		if (curr->data != NULL) {
			
			free(curr->data);

		}

		curr = curr->next;
		
	}

	priqueue_destroy(&job_queue);
		
	priqueue_destroy(&finished);	


}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
	pri_node_t *curr = job_queue.head;
	

	while (curr != NULL) {

		printf("%d(%d) ", ((job_t*)(curr->data))->job_number, -1);
		curr = curr->next;
	}
		
	int i;
	
	for (i = 0; i < num_cores; i++) {
	
		if (core_arr[i].core_job != NULL) {	
			printf("%d(%d) ", core_arr[i].core_job->job_number, i);
		}

		
	}


	

}
