#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/prctl.h>
// #include <sys/time.h>
#include "lift.h"
#include "draw.h"
#include "si_ui.h"
#include "debug.h"

// Unfortunately the rand() function is not thread-safe. However, the
// rand_r() function is thread-safe, but need a pointer to an int to
// store the current state of the pseudo-random generator.  This
// pointer needs to be unique for every thread that might call
// rand_r() simultaneously. The functions below are wrappers around
// rand_r() which should work in the environment encountered in
// assignment 3.
//

static unsigned int rand_r_state[MAX_N_PERSONS];
// Get a random value between 0 and maximum_value. The passenger_id
// parameter is used to ensure that the rand_r() function is called in
// a thread-safe manner.

static int get_random_value(int passenger_id, int maximum_value)
{
	return rand_r(&rand_r_state[passenger_id]) % (maximum_value + 1);
}

static lift_type Lift;

static sem_t passenger_semaphore;

static sem_t passengers_done;

static int total_time;

// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
	int i;
	prctl(PR_SET_NAME,"Lift Thread",0,0,0); // Sets a name shown in debuggers
	for(i=0; i < MAX_N_PERSONS; i++){
		// Use this statement if the same random number sequence
		// shall be used for every execution of the program.
		rand_r_state[i] = i;

		// Use this statement if the random number sequence
		// shall differ between different runs of the
		// program. (As the current time (as measured in
		// seconds) is used as a seed.)
		rand_r_state[i] = i+time(NULL);
	}
}

static void *lift_thread(void *unused)
{
	/* this thread should call: */
	/* lift_next_floor() */
	/* lift_move() */
	/* lift_has_arrived() */
	
	int next_floor;
	int change_direction = 0;
	
	while(1)
	{
		lift_next_floor(Lift, &next_floor, &change_direction);
		lift_move(Lift, next_floor, change_direction);
		lift_has_arrived(Lift);
		change_direction = 0;
	}
	return NULL;
}

static void *passenger_thread(void *idptr)
{
	// Code that reads the passenger ID from the idptr pointer
	// (due to the way pthread_create works we first need to cast
	// the void pointer to an int pointer).

	int *tmp = (int *) idptr;
	int id = *tmp;

	// We've read the id, so the user_thread can change the ID
	sem_post(&passenger_semaphore);

	// Sets a unique name shown in debuggers
	char buf[100];
	sprintf(buf, "Passenger #%d", id);
	prctl(PR_SET_NAME,buf,0,0,0);

	// iterations
	int i = 0;

	// Get start time
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	
	while(i < 10000)
	{
		// * Select random floors
		int from_floor = get_random_value(id, N_FLOORS - 1);
		int to_floor = get_random_value(id, N_FLOORS - 1);
	
		while(from_floor == to_floor)
		{
			to_floor = get_random_value(id, N_FLOORS - 1);
		}
		
		debug_check_override(id, &from_floor, &to_floor);

		// * Travel between these floors
		lift_travel(Lift, id, from_floor, to_floor);

		// * Wait a little while (~5 seconds)
		//sleep(5);
		i++;
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &end);

	uint delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

	// printf("%i\n", delta_us / 1000);

	total_time += delta_us / 1000;

	sem_post(&passengers_done);
	
	return NULL;
}

static void *user_thread(void *unused)
{
	int current_passenger_id = 0;
	char message[SI_UI_MAX_MESSAGE_SIZE];

	si_ui_set_size(1024, 720);
	prctl(PR_SET_NAME,"User Thread",0,0,0); // Sets the name shown in debuggers for this thread

	pthread_t passenger_thread_handle[MAX_N_PERSONS];
	
	while(1){
		// Read a message from the GUI
		si_ui_receive(message);

		/* create a new passenger */
		if(!strcmp(message, "new"))
		{
			// create a new passenger if possible, else
			// use si_ui_show_error() to show an error
			// message if too many passengers have been
			// created. Make sure that each passenger gets
			// a unique ID between 0 and MAX_N_PERSONS-1.

			// current_passenger_id = first_free_id(Lift);
			
			if(current_passenger_id < MAX_N_PERSONS)
			{
				pthread_create(&passenger_thread_handle[current_passenger_id], NULL, passenger_thread, (void *) &current_passenger_id);

				sem_wait(&passenger_semaphore);
				
				current_passenger_id++;
			}
			else
			{
				si_ui_show_error("Can't add a new person!");
			}
		}
		/* pause using the debug module */
		else if(!strcmp(message, "pause"))
		{
			debug_pause();
		}
		/* unpause using the debug module */
		else if(!strcmp(message, "unpause"))
		{
			debug_unpause();
		}
		/* test using the debug module */
		else if(!strncmp(message, "test", 4))
		{
			for (int j = 10;j <= 90;j+=10)
			{
				// printf("\nN:%i\n", j);
				total_time = 0;
				for (int i = 0;i < j;i++)
				{
					pthread_create(&passenger_thread_handle[i], NULL, passenger_thread, (void *) &i);

					sem_wait(&passenger_semaphore);
				}
				for (int i = 0;i < j;i++)
				{
					sem_wait(&passengers_done);
				}
				printf("\nN: %i, Average time: %i ms\n", j, total_time / j);
			}
		}
		/* exit the program */
		else if(!strcmp(message, "exit"))
		{
			lift_delete(Lift);
			exit(0);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	si_ui_init();
	init_random();
	Lift = lift_create();
	debug_init();

	sem_init(&passenger_semaphore, 0, 0);
	sem_init(&passengers_done, 0, 0);

	// Create tasks as appropriate here
	pthread_t lift_thread_handle;
	pthread_t user_thread_handle;

	pthread_create(&lift_thread_handle, NULL, lift_thread, 0);
	pthread_create(&user_thread_handle, NULL, user_thread, 0);
	
	pthread_join(lift_thread_handle, NULL);
	pthread_join(user_thread_handle, NULL);

	//pthread_t passenger_thread_handle[MAX_N_PERSONS];
	


	return 0;
}
