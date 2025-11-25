/* clock_init: initializes the clock variables */ 
void clock_init(void);

/* increment_time: increments the current time with 
   one second */ 
void increment_time(void);

/* set_time: set time to hours, minutes and seconds */ 
void set_time(int hours, int minutes, int seconds);

/* get_time: read time from clock variables */ 
void get_time(int *hours, int *minutes, int *seconds);

/* set_alarm: set alarm hours, minutes and seconds, also enable alarm */
void set_alarm_time(int hours, int minutes, int seconds);

/* unsafe, use get_alarm_enabled() instead */
/* /\* alarm_enabled: read state of alarm *\/ */
/* int alarm_enabled(void); */

void get_alarm_enabled(int *alarm_enabled);

/* reset_alarm: reset alarm */
void reset_alarm(void);

/* get_alarm: read alarm time from clock variables */
void get_alarm_time(int *hours, int *minutes, int *seconds);

/* time_from_set_message: extract time from set message from user interface */ 
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds);

/* time_from_alarm_message: extract alarm time from alarm message from user interface */
void time_from_alarm_message(char message[], int *hours, int *minutes, int *seconds);

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */ 
int time_ok(int hours, int minutes, int seconds);

/* trigger_alarm: trigger the alarm semaphore */
void trigger_alarm(void);

/* wait_for_alarm: wait for the alarm semaphore to trigger */
void wait_for_alarm(void);
