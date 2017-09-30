#include <cstdlib>
#include <iostream>
#include <Task.cc>
#include <vector>
#include <string>
#include <pthread.h>
using namespace std;

#define NUM_TASKS 10
vector<Task*> tasks;

Task* (*algorithm)();

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

Task* EDF(){
	Task* earliestTask = NULL;
	int i;
	for (i=0;i<NUM_TASKS;i++){
		Task* task = tasks.at(i);
		if(!task->completed){
			if (earliestTask == NULL || (earliestTask->deadline > task->deadline)) {
				earliestTask = task;
			}
		}
	}
	return earliestTask;
}

Task* SCT(){
	Task* earliestTask = NULL;
	int i;
	for (i=0;i<NUM_TASKS;i++){
		Task* task = tasks.at(i);
		if(!task->completed){
			if (earliestTask == NULL || (earliestTask->rem_exec_time > task->rem_exec_time)) {
				earliestTask = task;
			}
		}
	}
	return earliestTask;
}

Task* LST(){
	Task* earliestTask = NULL;
	int i;
	for (i=0;i<NUM_TASKS;i++){
		Task* task = tasks.at(i);
		if(!task->completed){
			if (earliestTask == NULL || (earliestTask->deadline - earliestTask->rem_exec_time > task->deadline - task->rem_exec_time)) {
				earliestTask = task;
			}
		}
	}
	return earliestTask;
}

int main(int argc, char *argv[]) {
	printf("Welcome to the Real-Time Scheduler\n");
	printf("Scheduling algorithm choices:\n(0) Earliest Deadline First\n(1) Shortest Completion Time\n(2) Least Slack Time\n");
	printf("Choose a scheduling algorithm (enter the number of your choice): ");
	int selection = -1;
	while (!(selection>=0 && selection <=2)) {
		cin >> selection;
	}
	printf("You selected ");
	switch(selection) {
	case 0: algorithm = &EDF;
			printf("Earliest Deadline First\n");
			break;
	case 1: algorithm = &SCT;
			printf("Shortest Completion Time\n");
			break;
	case 2: algorithm = &LST;
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
			tasks.push_back(task);
		}
	}

	//Scheduler section
	int runTime = 0;
	int elapsedTime = 0;
	Task* currentTask;
	Task* previousTask = NULL;


	printf("How long would you like to run the simulation for? ");
	cin >> runTime;
	currentTask = algorithm();

	while(elapsedTime < runTime){

		if (currentTask != previousTask && previousTask != NULL){
			currentTask.runSet(true);
			previousTask.runSet(false);
		}else{

		}
		//
		currentTask->total_exec_time ++;
		currentTask = algorithm();
	}
	printf("%d", tasks.size());


	return EXIT_SUCCESS;
}
