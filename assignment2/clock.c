/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "clock.h"

/* data structure for clock information */
typedef struct
{
    /* time */
    int hours, minutes, seconds;
    
} time_data_type;

/* data structure for clock */
typedef struct
{
    /* the current time */
    time_data_type time;

    /* alarm time */
    time_data_type alarm_time;

    /* alarm enabled flag */
    int alarm_enabled;

    /* semaphore for mutual exclusion */
    pthread_mutex_t mutex;

    /* semaphore for clock activation */
    sem_t start_alarm;
} clock_data_type;

/* the actual clock */
static clock_data_type Clock;

/* functions operating on the clock */ 

/* clock_init: initializes the clock variables */ 
void clock_init(void)
{
    /* initialize starting time */ 
    Clock.time.hours = 0;
    Clock.time.minutes = 0;
    Clock.time.seconds = 0;

    /* initialize alarm time */ 
    Clock.alarm_time.hours = 0;
    Clock.alarm_time.minutes = 0;
    Clock.alarm_time.seconds = 0;

    /* initialize alarm enable flag */
    Clock.alarm_enabled = 0;

    /* initialize semaphores */ 
    pthread_mutex_init(&Clock.mutex, NULL);
    sem_init(&Clock.start_alarm, 0, 0);
}

/* increment_time: increments the current time with 
   one second */ 
void increment_time(void)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Clock.mutex);

    /* increment time */ 
    Clock.time.seconds++; 
    if (Clock.time.seconds > 59)
    {
        Clock.time.seconds = 0; 
        Clock.time.minutes++; 
        if (Clock.time.minutes > 59)
        {
            Clock.time.minutes = 0; 
            Clock.time.hours++; 
            if (Clock.time.hours > 23)
            {
                Clock.time.hours = 0; 
            }
        }
    }

    /* release clock variables */ 
    pthread_mutex_unlock(&Clock.mutex);
}

/* set_time: set time to hours, minutes and seconds */ 
void set_time(int hours, int minutes, int seconds)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Clock.mutex);

    /* assign values */
    Clock.time.hours = hours; 
    Clock.time.minutes = minutes;
    Clock.time.seconds = seconds; 
    
    /* release clock variables */ 
    pthread_mutex_unlock(&Clock.mutex);
}

/* get_time: read time from clock variables */ 
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.time.hours;
    *minutes = Clock.time.minutes;
    *seconds = Clock.time.seconds;
        
    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* set_alarm: set alarm hours, minutes and seconds, also enable alarm */
void set_alarm_time(int hours, int minutes, int seconds)
{
	/* reserve clock variables */
	pthread_mutex_lock(&Clock.mutex);

	/* set alarm values */
	Clock.alarm_time.hours = hours;
	Clock.alarm_time.minutes = minutes;
	Clock.alarm_time.seconds = seconds;

	/* enable alarm */
	Clock.alarm_enabled = 1;

	/* release clock variables */
	pthread_mutex_unlock(&Clock.mutex);
}

/* alarm_enabled: read state of alarm */
int alarm_enabled(void)
{
	/* read alarm state */
	return(Clock.alarm_enabled);
}

/* reset_alarm: reset alarm */
void reset_alarm(void)
{
	/* reserve clock variables */
	pthread_mutex_lock(&Clock.mutex);

	/* reset alarm */
	Clock.alarm_enabled = 0;
	
	/* release clock variables */
	pthread_mutex_unlock(&Clock.mutex);
}

/* get_alarm: read alarm time from clock variables */
void get_alarm_time(int *hours, int *minutes, int *seconds)
{
	/* release clock variables */
	pthread_mutex_lock(&Clock.mutex);

	/* read values */
	*hours = Clock.alarm_time.hours;
	*minutes = Clock.alarm_time.minutes;
	*seconds = Clock.alarm_time.seconds;
	
	/* release clock variables */
	pthread_mutex_unlock(&Clock.mutex);
}

/* time_from_set_message: extract time from set message from user interface */ 
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds)
{
	sscanf(message,"set %d %d %d", hours, minutes, seconds);
}

/* time_from_alarm_message: extract alarm time from alarm message from user interface */
void time_from_alarm_message(char message[], int *hours, int *minutes, int *seconds)
{
	sscanf(message,"alarm %d %d %d", hours, minutes, seconds);
}

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */ 
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 && 
        seconds >= 0 && seconds <= 59; 
}

/* trigger_alarm: trigger the alarm semaphore */
void trigger_alarm(void)
{
	sem_post(&Clock.start_alarm);
}

/* wait_for_alarm: wait for the alarm semaphore to trigger */
void wait_for_alarm(void)
{
	sem_wait(&Clock.start_alarm);
}
