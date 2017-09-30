#include <string>
#include <stdlib.h>
#include <cstdlib>
#include <pthread.h>
#include <iostream>
using namespace std;

/* Represents a task to be scheduled */
class Task {
public:
		string name;
		int total_exec_time;
		int rem_exec_time;
		int deadline;
		int period;
		pthread_t* thread;
		bool completed; // completed during this period

public:

/* Create a task */
Task(string t_name, int t_exec_time, int t_deadline, int t_period, pthread_t* t_thread) {
	name = t_name;
	total_exec_time = t_exec_time;
	rem_exec_time = t_exec_time;
	deadline = t_deadline;
	period = t_period;
	thread = t_thread;
	completed = false;
}

/* Set the task's priority */
void setPriority(int new_priority) {
	pthread_setschedprio(*thread, new_priority);
	//For testing purposes
	struct sched_param* param;
	pthread_getschedparam(*thread, NULL, param);
	printf("%s is at priority %d", name.c_str(), param->sched_priority);
}
};


		
	
