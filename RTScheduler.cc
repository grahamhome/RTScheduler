#include <cstdlib>
#include <iostream>
#include <Task.cc>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
using namespace std;

#define NUM_TASKS 10
vector<Task*> allTasks;
vector<Task*> activeTasks; // Ordered by priority, low to high
pthread_mutex_t activeTasksMutex;
int algorithm = -1;
bool tasksRunning = false;
pthread_mutex_t runMutex;

/* Method run by a task during execution */
void* execute(void* task) {
	return NULL;
}

/* Sets up a new thread */
void createThread(pthread_t* thread, Task* task) {
	pthread_attr_t attributes;
	pthread_attr_init(&attributes);
	int priority = 1; // Default starting priority for all tasks
	struct sched_param param;
	pthread_attr_getschedparam(&attributes, &param);
	param.sched_priority = priority;
	pthread_attr_setschedparam(&attributes, &param);
	pthread_create(thread, &attributes, execute, (void*)task);
}

/* Assign priorities to all tasks based on EDF algorithm */
void EDF(){
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&activeTasksMutex);
	activeTasks.clear();
	for (int i=0;i<NUM_TASKS;i++){
		Task* task = allTasks.at(i);
		if(!task->completed){
			int t;
			for (t=0;t<activeTasks.size();t++) {
				if (activeTasks.at(t)->deadline > task->deadline) {
					break;
				}
			}
			activeTasks.insert(activeTasks.begin()+t, task);
		}
	}
	pthread_mutex_unlock(&activeTasksMutex);
}

void* SCT(){
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&activeTasksMutex);
	activeTasks.clear();
	for (int i=0;i<NUM_TASKS;i++){
		Task* task = allTasks.at(i);
		if(!task->completed){
			int t;
			for (t=0;t<activeTasks.size();t++) {
				if (activeTasks.at(t)->rem_exec_time > task->rem_exec_time) {
					break;
				}
			}
			activeTasks.insert(activeTasks.begin()+t, task);
		}
	}
	pthread_mutex_unlock(&activeTasksMutex);
}

void LST(){
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&activeTasksMutex);
	activeTasks.clear();
	for (int i=0;i<NUM_TASKS;i++){
		Task* task = allTasks.at(i);
		if(!task->completed){
			int t;
			for (t=0;t<activeTasks.size();t++) {
				if (activeTasks.at(t)->deadline-activeTasks.at(t)->rem_exec_time > task->deadline-task->rem_exec_time) {
					break;
				}
			}
			activeTasks.insert(activeTasks.begin()+t, task);
		}
	}
	pthread_mutex_unlock(&activeTasksMutex);
}

void scheduleTasks() {
	switch(algorithm) {
	case 0:
		EDF();
		break;
	case 1:
		SCT();
		break;
	case 2:
		LST();
		break;
	}
}

void startAllTasks() {
	pthread_mutex_lock(&runMutex);
	tasksRunning = true;
	pthread_mutex_unlock(&runMutex);
}

bool checkTasksRunning() {
	bool running = false;
	pthread_mutex_lock(&runMutex);
	running = tasksRunning;
	pthread_mutex_unlock(&runMutex);
	return running;

}

int main(int argc, char *argv[]) {
	pthread_mutex_init(&activeTasksMutex, NULL);
	pthread_mutex_init(&runMutex, NULL);
	printf("Welcome to the Real-Time Scheduler\n");
	printf("Scheduling algorithm choices:\n(0) Earliest Deadline First\n(1) Shortest Completion Time\n(2) Least Slack Time\n");
	printf("Choose a scheduling algorithm (enter the number of your choice): ");
	while (!(algorithm>=0 && algorithm <=2)) {
		cin >> algorithm;
	}
	printf("You selected ");
	switch(algorithm) {
	case 0:
			printf("Earliest Deadline First\n");
			break;
	case 1:
			printf("Shortest Completion Time\n");
			break;
	case 2:
			printf("Least Slack Time\n");
			break;
	}
	printf("Enter tasks in the format \'<name> <execution time> <deadline> <period>\' and follow each with a newline.\nTo run tasks, enter \'start\' followed by a newline.\n");
	string input;
	getline(cin, input); // Ignore newline from previous println
	bool done = false;
	while (!done) {
		getline(cin, input);
		if (input == "start") {
			done = true;
			break;
		}
		string name;
		int c;
		int p;
		int d;
		int tokenCount = 0;
		size_t pos = 0;
		while (((pos = input.find(" ")) <= string::npos) && tokenCount < 4) {
			switch(tokenCount) {
			case 0:
				name = input.substr(0, pos);
				tokenCount++;
				break;
			case 1:
				c = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			case 2:
				p = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			case 3:
				d = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			default:
				tokenCount++;
				break;
			}
			input.erase(0, pos+1);
		}
		if (tokenCount != 4) {
			printf("Invalid task format.\n");
		} else {
			pthread_t* taskThread = NULL;
			Task* task = new Task(name, c, p, d, taskThread);
			createThread(taskThread, task);
			allTasks.push_back(task);
		}
	}

	//Scheduler section
	int runTime = 0;
	int elapsedTime = 0;

	printf("How long would you like to run the simulation for? ");
	cin >> runTime;
	scheduleTasks();
	startAllTasks();

	while(elapsedTime < runTime){
		sleep(1);
		scheduleTasks();
	}

	return EXIT_SUCCESS;
}
