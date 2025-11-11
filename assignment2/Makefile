all: set_clock

set_clock: set_clock.c clock.c display.c si_comm.c si_ui.c clock.h display.h si_ui.h si_comm.h
	gcc -std=gnu11 set_clock.c clock.c display.c si_ui.c si_comm.c -g  -o set_clock -Wall -lpthread -lrt

clean:
	rm -f set_clock *~ *.o
