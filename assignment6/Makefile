all:	rt_task disturb create_saw

rt_task: rt_task.c samples.c samples.h
	gcc -std=gnu11 rt_task.c samples.c  -g -o rt_task -lpthread -lrt -Wall

disturb: disturb.c
	gcc -std=gnu11 disturb.c -o disturb

create_saw: create_saw.c
	gcc -std=gnu11 create_saw.c -o create_saw

clean:
	rm -f rt_task *.o *~
	rm -f sample_times.txt a.out output.txt
	rm -f disturb create_saw data_saw.raw
