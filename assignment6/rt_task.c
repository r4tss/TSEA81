#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#define MODIFIED 1

// This file contains the definition of MAX_ITERATIONS, among other things.
#include "samples.h"


// Delay in nanoseconds (1 millisecond)
#define DELAY 1000000

// Number of samples that do_work() handles
#define PROCESSING_INTERVAL  256 

// Could be a local variable, but you may need it as a static variable
// here when you modify this file according to the lab instructions.
static int sample_buffer[2][PROCESSING_INTERVAL];

// Mutex for buffers
static pthread_mutex_t sample_mutex[2];

// Semaphore for synchronizing when to do work
static sem_t do_work_sem;

// Integer for telling when things are done
static int done = 0;

// Mutex for done variable
static pthread_mutex_t done_mutex;

void do_work(int *samples)
{
	int i;

	//  A busy loop. (In a real system this would do something
	//  more interesting such as an FFT or a particle filter,
	//  etc...)
	volatile int dummy; // A compiler warning is ok here
	for(i=0; i < 20000000;i++){
		dummy=i;
	}

	// Write out the samples.
	for(i=0; i < PROCESSING_INTERVAL; i++){
		write_sample(0,samples[i]);
	}

}

struct timespec firsttime;
void *maintask(void *arg)
{
	int channel = 0;
	struct timespec current;
	int i;
	int samplectr = 0;
	current = firsttime;


	for(i=0; i < MAX_ITERATIONS; i++){
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);


		sample_buffer[0][samplectr] = read_sample(channel);
		samplectr++;
		if(samplectr == PROCESSING_INTERVAL){
			samplectr = 0;
			do_work(sample_buffer[0]);
		}

		// Increment current time point
		current.tv_nsec +=  DELAY;
		if(current.tv_nsec >= 1000000000){
			current.tv_nsec -= 1000000000;
			current.tv_sec++;
		}
	}
	return NULL;
}

// Split maintask() into sampletask() and doworktask()
void *sampletask(void *arg)
{
	int channel = 0;
	struct timespec current;
	int i;
	int samplectr = 0;
	int buffersel = 0;
	current = firsttime;

	// Set thread priority
	struct sched_param sp;
	sp.sched_priority = 10;

	if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
	{
		fprintf(stderr, "WARNING: Failed to set sample thread to real-time priority\n");
	}

	pthread_mutex_lock(&sample_mutex[buffersel]);
	
	for (i=0;i < MAX_ITERATIONS;i++)
	{
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);

		sample_buffer[buffersel][samplectr] = read_sample(channel);
		samplectr++;

		if (samplectr == PROCESSING_INTERVAL)
		{
			pthread_mutex_unlock(&sample_mutex[buffersel]);

			buffersel = !buffersel;
			samplectr = 0;

			pthread_mutex_lock(&sample_mutex[buffersel]);

			sem_post(&do_work_sem);
		}

		current.tv_nsec += DELAY;
		if (current.tv_nsec >= 1000000000)
		{
			current.tv_nsec -= 1000000000;
			current.tv_sec++;
		}
	}
	
	pthread_mutex_unlock(&sample_mutex[buffersel]);

	pthread_mutex_lock(&done_mutex);
	done = 1;
	pthread_mutex_unlock(&done_mutex);

	sem_post(&do_work_sem);
	return NULL;
}

void *doworktask(void *arg)
{
	int buffersel = 0;
	
	// Set thread priority
	struct sched_param sp;
	sp.sched_priority = 5;

	if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
	{
		fprintf(stderr, "WARNING: Failed to set sample thread to real-time priority\n");
	}

	pthread_mutex_lock(&done_mutex);
	while (!done)
	{
		pthread_mutex_unlock(&done_mutex);
		
		sem_wait(&do_work_sem);

		pthread_mutex_lock(&sample_mutex[buffersel]);
		do_work(sample_buffer[buffersel]);
		pthread_mutex_unlock(&sample_mutex[buffersel]);

		buffersel = !buffersel;

		pthread_mutex_lock(&done_mutex);
	}
	pthread_mutex_unlock(&done_mutex);
	
	return NULL;
}

int main(int argc,char **argv)
{
#if MODIFIED
	// Init semaphore and mutexes
	sem_init(&do_work_sem, 0, 0);

	pthread_mutex_init(&sample_mutex[0], NULL);
	pthread_mutex_init(&sample_mutex[1], NULL);
	pthread_mutex_init(&done_mutex, NULL);

	pthread_t sample_thread_handle;
	pthread_t dowork_thread_handle;
#else
	pthread_t main_thread_handle;
#endif
	pthread_attr_t attr;

	clock_gettime(CLOCK_MONOTONIC, &firsttime);

	// Start the sampling at an even multiple of a second (to make
	// the sample times easy to analyze by hand if necessary)
	firsttime.tv_sec+=2;
	firsttime.tv_nsec = 0;
	printf("Starting sampling at about t+2 seconds\n");
        
	samples_init(&firsttime);

	if(pthread_attr_init(&attr)){
		perror("pthread_attr_init");
	}
	// Set default stacksize to 64 KiB (should be plenty)
	if(pthread_attr_setstacksize(&attr, 65536)){
		perror("pthread_attr_setstacksize()");
	}

#if MODIFIED
	// Lock memory to ensure no swapping is done
	if (mlockall(MCL_FUTURE | MCL_CURRENT))
	{
		fprintf(stderr, "WARNING: Failed to lock memory\n");
	}
     
	pthread_create(&sample_thread_handle, &attr, sampletask, NULL);
	pthread_create(&dowork_thread_handle, &attr, doworktask, NULL);
	
	pthread_join(sample_thread_handle, NULL);
	pthread_join(dowork_thread_handle, NULL);
#else
	pthread_create(&main_thread_handle, &attr, maintask, NULL);

	pthread_join(main_thread_handle, NULL);
#endif

	// Dump output data which will be used by the analyze.m script
	dump_outdata();
	dump_sample_times();
	return 0;
}
