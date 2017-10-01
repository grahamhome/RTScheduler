#include <string>
#include <pthread.h>
#include <RTScheduler.h>
using namespace std;

class Task {
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

	Task(string t_name, int t_exec_time, int t_deadline, int t_period, pthread_t* t_thread) {
		name = t_name;
		total_exec_time = t_exec_time;
		rem_exec_time = t_exec_time;
		deadline = t_deadline;
		period = t_period;
		thread = t_thread;
		completed = false;
		pthread_mutex_init(&activeMutex, NULL);
	}

	/* Start or stop this task */
	void setActive(bool status){
		pthread_mutex_lock(&activeMutex);
		active = status;
		pthread_mutex_lock(&activeMutex);
	}

	/* Checks master task switch and instance-specific task switch */
	bool canRun() {
		bool status = true;
		pthread_mutex_lock(&activeMutex);
		status = active;
		pthread_mutex_lock(&activeMutex);
		return status;
	}
};


		
	
