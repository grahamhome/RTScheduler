#include <cstdlib>
#include <iostream>
#include <Task.cc>
#include <vector>
using namespace std;

#define NUM_TASKS 10
vector<Task*> tasks;

Task* (*algorithm)();

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
	while (!(selection>=0 && selection <3)) {
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
	return EXIT_SUCCESS;
}
