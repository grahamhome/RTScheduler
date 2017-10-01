/*
 * Task.h
 *
 *  Created on: Oct 1, 2017
 *      Author: gmh5970
 */

#ifndef TASK_H_
#define TASK_H_

#include <string>
#include <stdlib.h>
#include <cstdlib>
#include <pthread.h>
#include <iostream>
using namespace std;

/* Represents a task to be scheduled */
class Task {
private:
	static bool running;
	static pthread_mutex_t runMutex;
public:
	string name;
	int total_exec_time;
	int rem_exec_time;
	int deadline;
	int period;
	pthread_t* thread;
	bool completed; // completed during this period
	bool active; // indicates whether or not this particular task is active
	pthread_mutex_t activeMutex;

	Task(string t_name, int t_exec_time, int t_deadline, int t_period, pthread_t* t_thread);

	/* Start or stop all tasks */
	static void toggleAll(bool run) {
		pthread_mutex_lock(&runMutex);
		running = run;
		pthread_mutex_unlock(&runMutex);
	}

	/* Start or stop this task */
	void setActive(bool status);

	/* Checks master task switch and instance-specific task switch */
	bool canRun();
};

#endif /* TASK_H_ */
