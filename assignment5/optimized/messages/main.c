#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include "lift.h"
#include "si_ui.h"
#include "messages.h"

#include "draw.h"

#define QUEUE_UI 0
#define QUEUE_LIFT 1
#define QUEUE_FIRSTPERSON 10
#define QUEUE_PERSONDONE 100

// These variables keeps track of the process IDs of all processes
// involved in the application so that they can be killed when the
// exit command is received.
static pid_t lift_pid;
static pid_t uidraw_pid;
static pid_t liftmove_pid;
static pid_t person_pid[MAX_N_PERSONS];

static int total_time;

typedef enum {LIFT_TRAVEL, // A travel message is sent to the list process when a person would
			  // like to make a lift travel
			  LIFT_TRAVEL_DONE, // A travel done message is sent to a person process when a
			  // lift travel is finished
			  LIFT_MOVE         // A move message is sent to the lift task when the lift shall move
			  // to the next floor
} lift_msg_type; 

struct lift_msg{
	lift_msg_type type;  // Type of message
	int person_id;       // Specifies the person
	int from_floor[100];      // Specify source and destion for the LIFT_TRAVEL message.
	int to_floor[100];
};

// Since we use processes now and not 
static int get_random_value(int person_id, int maximum_value)
{
	return rand() % (maximum_value + 1);
}


// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
	srand(getpid()); // The pid should be a good enough initialization for
	// this case at least.
}


static void liftmove_process(void)
{
	struct lift_msg m;
	m.type = LIFT_MOVE;
  
	while(1){
#ifdef NEWGUI
		//usleep(4000000); // sleep 4 seconds
#else
		//usleep(2000000); // sleep 2 seconds
#endif
		// TODO:
		//    Send a message to the lift process to move the lift.
		message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);
	}
}

static void lift_process(void)
{
	lift_type Lift;
	Lift = lift_create();
	int change_direction = 0, next_floor;
    
	char msgbuf[4096];

	int journey[MAX_N_PERSONS];
	struct lift_msg journeys[MAX_N_PERSONS];
	
	while(1) {
		int i;
		struct lift_msg reply;
		struct lift_msg *m;
		//message_send((char *) Lift, sizeof(*Lift), QUEUE_UI,0); // Draw the lift
		int len = message_receive(msgbuf, 4096, QUEUE_LIFT); // Wait for a message
		if (len < sizeof(struct lift_msg)) {
			fprintf(stderr, "Message too short\n");
			continue;
		}

		m = (struct lift_msg *) msgbuf;

		switch(m->type) {

		case LIFT_MOVE:
#ifdef NEWGUI
			Lift->moving = 1; // only used in draw.c
			//message_send((char *) Lift, sizeof(*Lift), QUEUE_UI,0); // Draw the lift
			//usleep(1000000);  // "closing doors"
#endif			
			// Move lift
			lift_next_floor(Lift, &next_floor, &change_direction);
			lift_move(Lift, next_floor, change_direction);
			change_direction = 0;

			//message_send((char *) Lift, sizeof(*Lift), QUEUE_UI,0); // Draw the lift
#ifdef NEWGUI
			//usleep(1000000); // "opening doors"
			Lift->moving = 0; // only used in draw.c
			//message_send((char *) Lift, sizeof(*Lift), QUEUE_UI,0); // Draw the lift
#endif
			//    Check if passengers want to leave elevator
			for (i = 0;i < MAX_N_PASSENGERS;i++)
			{
				if (Lift->passengers_in_lift[i].to_floor == Lift->floor)
				{
					//        Send a LIFT_TRAVEL_DONE for each passenger that leaves
					//        the elevator if they've finished all their journeys
					if (journey[Lift->passengers_in_lift[i].id] == 99)
					{
						reply.type = LIFT_TRAVEL_DONE;
						message_send((char *) &reply, sizeof(reply), QUEUE_FIRSTPERSON + Lift->passengers_in_lift[i].id, 0);

						//        Remove the passenger from the elevator
						Lift->passengers_in_lift[i].id = NO_ID;
						Lift->passengers_in_lift[i].to_floor = NO_FLOOR;
					}
					else
					{

						int id = journeys[Lift->passengers_in_lift[i].id].person_id;
						int to_floor = journeys[id].to_floor[journey[id]];
						int from_floor = journeys[id].from_floor[journey[id]];

						// Place on new starting floor for next journey
						int found = 0;
						for (int j = 0;j < MAX_N_PERSONS && !found;j++)
						{
							if (Lift->persons_to_enter[from_floor][j].id == NO_ID)
							{
								Lift->persons_to_enter[from_floor][j].id = id;
								Lift->persons_to_enter[from_floor][j].to_floor = to_floor;
								found = 1;
							}
						}
						
						journey[Lift->passengers_in_lift[i].id]++;
										  
						//        Remove the passenger from the elevator
						Lift->passengers_in_lift[i].id = NO_ID;
						Lift->passengers_in_lift[i].to_floor = NO_FLOOR;

					}
				}
			}

			//    Check if passengers want to enter elevator
			for (i = 0;i < MAX_N_PERSONS;i++)
			{
				//        Remove the passenger from the floor and into the elevator
				int id = Lift->persons_to_enter[Lift->floor][i].id;
				if (id != NO_ID && (n_passengers_in_lift(Lift) < MAX_N_PASSENGERS))
				{
					int to_floor = Lift->persons_to_enter[Lift->floor][i].to_floor;
					
					leave_floor(Lift, id, Lift->floor);
					enter_lift(Lift, id, to_floor);
				}
			}
			break;
			
		case LIFT_TRAVEL:
			journeys[m->person_id] = *m;
			journey[m->person_id] = 0;
			for (i = 0;i < MAX_N_PERSONS;i++)
			{
				if (Lift->persons_to_enter[m->from_floor[0]][i].id == NO_ID)
				{
					Lift->persons_to_enter[m->from_floor[0]][i].id = m->person_id;
					Lift->persons_to_enter[m->from_floor[0]][i].to_floor = m->to_floor[0];
					journey[m->person_id]++;
					break;
				}
			}
			
			break;
		case LIFT_TRAVEL_DONE:
			break;
		}
	}
	return;
}
 

static void person_process(int id)
{
	init_random();
	char buf[4096];
	struct lift_msg m;
	int i = 0;

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);

	m.person_id = id;
	
	while(i < 100){
		for (int trip = 0;trip < 100;trip++)
		{
			//    Generate a to and from floor
			m.to_floor[trip] = get_random_value(id, N_FLOORS - 1);
			m.from_floor[trip] = get_random_value(id, N_FLOORS - 1);
			while (m.to_floor[trip] == m.from_floor[trip])
			{
				m.from_floor[trip] = get_random_value(id, N_FLOORS - 1);
			}
		}
		
		//    Send a LIFT_TRAVEL message to the lift process
		message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);

		//    Wait for a LIFT_TRAVEL_DONE message
		int len = message_receive(buf, 4096, QUEUE_FIRSTPERSON + id);

		if(len < sizeof(struct lift_msg))
		{
			fprintf(stderr, "Message too short\n");
			continue;
		}

		i++;
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &end);

	uint delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

	//total_time += delta_us / 1000;

	struct lift_msg temp;
	temp.person_id = delta_us / 1000;

	message_send((char *) &temp, sizeof(temp), QUEUE_PERSONDONE + id, 0);
	//printf("%i: Done in %i ms!\n", id, delta_us / 1000);
	
	pause();
}

// This is the final process called by main()
// It is responsible for:
//   * Receiving and executing commands from the java GUI
//   * Killing off all processes when exiting the application
void uicommand_process(void)
{
	int current_person_id = 0;
	char message[SI_UI_MAX_MESSAGE_SIZE]; 
	while(1){
		// Read a message from the GUI
		si_ui_receive(message);
		if(!strcmp(message, "new")){
			// * Check that we don't create too many persons
			// * fork and create a new person process (and
			//   record the new pid in person_pid[])
			if (current_person_id < MAX_N_PERSONS)
			{
				person_pid[current_person_id] = fork();
				
				if (!person_pid[current_person_id])
				{
					person_process(current_person_id);
				}

				current_person_id++;
			}
			else
			{
				si_ui_show_error("Can't add a new person!");
			}
		}
		else if (!strncmp(message, "test", 4))
		{
			total_time = 0;
			char buf[4096];
			struct lift_msg *m;

			for (int j = 10;j <= 90;j+=2)
			{
				printf("\nN:%i\n", j);
				for (int i = 0;i < j;i++)
				{
					person_pid[i] = fork();

					if (!person_pid[i])
					{
						person_process(i);
					}
				}
				for (int i = 0;i < j;i++)
				{
					int len = message_receive(buf, 4096, QUEUE_PERSONDONE + i);
				
					if (len < sizeof(struct lift_msg))
					{
						fprintf(stderr, "Message too short\n");
						continue;
					}

					m = (struct lift_msg *) buf;

					// printf("Person %i done in %i ms\n", i, m->person_id);

					total_time += m->person_id;

					kill(person_pid[i], SIGINT);
				}
				printf("\nN: %i, Average time: %i ms\n", j, total_time / j);
			}
		}
		else if(!strcmp(message, "exit")){
			int i;
			// The code below sends the SIGINT signal to
			// all processes involved in this application
			// except for the uicommand process itself
			// (which is exited by calling exit())
			kill(uidraw_pid, SIGINT);
			kill(lift_pid, SIGINT);
			kill(liftmove_pid, SIGINT);
			for(i=0; i < MAX_N_PERSONS; i++){
				if(person_pid[i] > 0){
					kill(person_pid[i], SIGINT);
				}
			}
			exit(0);
		}
	}
}

// This process is responsible for drawing the lift. Receives lift_type structures
// as messages.
void uidraw_process(void)
{
	char msg[1024];
	si_ui_set_size(670, 700); 
	while(1){
		message_receive(msg, 1024, QUEUE_UI);
		lift_type Lift = (lift_type) &msg[0];
		draw_lift(Lift);
	}
}

int main(int argc, char **argv)
{
	message_init();
	si_ui_init(); // Initialize user interface. (Must be done
	// here!)
	
	lift_pid = fork();
	if(!lift_pid) {
		lift_process();
	}
	uidraw_pid = fork();
	if(!uidraw_pid){
		uidraw_process();
	}
	liftmove_pid = fork();
	if(!liftmove_pid){
		liftmove_process();
	}
	uicommand_process();

	return 0;
}
