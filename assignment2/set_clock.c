/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "clock.h"
#include "display.h"
#include "si_ui.h"
#include "si_comm.h"

void *clock_thread(void *unused)
{
	/* time for next update */
	struct timespec ts;
	
    /* local copies of the current time and alarm state */ 
    int hours, minutes, seconds, alarm_hours, alarm_minutes, alarm_seconds, alarm_enabled;

	/* initialize time for next update */
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
    /* infinite loop */ 
    while (1)
    {
        /* read and display current time */ 
        get_time(&hours, &minutes, &seconds); 
        display_time(hours, minutes, seconds);

		/* check if alarm is enabled */
		get_alarm_enabled(&alarm_enabled);
		if(alarm_enabled)
		{
			/* read and display alarm time */
			get_alarm_time(&alarm_hours, &alarm_minutes, &alarm_seconds);
			display_alarm_time(alarm_hours, alarm_minutes, alarm_seconds);

			/* check if current time is equal to the alarm time */
			if(hours == alarm_hours &&
			   minutes == alarm_minutes &&
			   seconds == alarm_seconds)
			{				
				trigger_alarm();
			}
		}

        /* increment time */ 
        increment_time(); 

        /* wait one second */ 

		/* compute time for next update */
		ts.tv_nsec += 1000000000;
		while(ts.tv_nsec >= 1000000000)
		{
			ts.tv_nsec -= 1000000000;
			ts.tv_sec++;
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
    }
}

void *alarm_thread(void *unused)
{
	/* time for next update */
	struct timespec ts;
	
	while(1)
	{
		wait_for_alarm();

		/* initialize time for next update */
		clock_gettime(CLOCK_MONOTONIC, &ts);

		int alarm_enabled;

		get_alarm_enabled(&alarm_enabled);

		while(alarm_enabled)
		{
			display_alarm_text();
			
			/* wait one and a half seconds */

			/* compute time for next update */
			ts.tv_nsec += 1500000000;
			while(ts.tv_nsec >= 1000000000)
			{
				ts.tv_nsec -= 1000000000;
				ts.tv_sec++;
			}
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);

			get_alarm_enabled(&alarm_enabled);
		}
	}
}

void *set_thread(void *unused)
{
	/* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    /* time read from user interface */ 
    int hours, minutes, seconds; 

    /* set GUI size */ 
    si_ui_set_size(400, 200); 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is a set message */
		
		/* set clock time */
		if(strncmp(message, "set", 3) == 0)
		{
			time_from_set_message(message, &hours, &minutes, &seconds);
			if(time_ok(hours, minutes, seconds))
			{
				set_time(hours, minutes, seconds);
			}
			else
			{
				si_ui_show_error("Illegal value for hours, minutes or seconds");
			}
		}
		/* set alarm time */
		else if(strncmp(message, "alarm", 5) == 0)
		{
			time_from_alarm_message(message, &hours, &minutes, &seconds);
			if(time_ok(hours, minutes, seconds))
			{
				/* reset alarm, if you set the alarm while there is an alarm configured */
				reset_alarm();
				erase_alarm_text();

				/* set alarm */
				set_alarm_time(hours, minutes, seconds);
			}
			else
			{
				si_ui_show_error("Illegal value for hours, minutes or seconds");
			}
		}
		/* reset alarm */
		else if(strcmp(message, "reset") == 0)
		{
			reset_alarm();
			erase_alarm_text();
			erase_alarm_time();
		}
		/* exit program */
		else if(strcmp(message, "exit") == 0)
		{
			exit(0);
		}
		/* not a legal message */
		else
		{
			si_ui_show_error("unexpected message type"); 
		}
	}
}

/* main */ 
int main(void)
{
	/* initialize UI channel */
	si_ui_init();

	/* initialize variables */
	clock_init();
	display_init();

	/* create tasks */
	pthread_t clock_thread_handle;
	pthread_t set_thread_handle;
	pthread_t alarm_thread_handle;

	pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
	pthread_create(&set_thread_handle, NULL, set_thread, 0);
	pthread_create(&alarm_thread_handle, NULL, alarm_thread, 0);

	pthread_join(clock_thread_handle, NULL);
	pthread_join(set_thread_handle, NULL);
	pthread_join(alarm_thread_handle, NULL);
	
    /* will never be here! */ 
    return 0; 
}
