#include <Task.h>
using namespace std;

// Initialize static class variables
bool Task::running = false;
pthread_mutex_t Task::runMutex;

/* Create a task */
Task::Task(string t_name, int t_exec_time, int t_deadline, int t_period, pthread_t* t_thread) {
	name = t_name;
	total_exec_time = t_exec_time;
	rem_exec_time = t_exec_time;
	deadline = t_deadline;
	period = t_period;
	thread = t_thread;
	completed = false;
	pthread_mutex_init(&activeMutex, NULL);
	pthread_mutex_init(&runMutex, NULL);
}

/* Start or stop this task */
void Task::setActive(bool status){
	pthread_mutex_lock(&activeMutex);
	active = status;
	pthread_mutex_lock(&activeMutex);
}

/* Checks master task switch and instance-specific task switch */
bool Task::canRun() {
	bool status = true;
	pthread_mutex_lock(&runMutex);
	status = running;
	pthread_mutex_unlock(&runMutex);
	pthread_mutex_lock(&activeMutex);
	if (!active) status = false;
	pthread_mutex_lock(&activeMutex);
	return status;
}


		
	
