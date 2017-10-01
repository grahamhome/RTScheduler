/*
 * RTScheduler.h
 * Created by Graham Home and Alberto Castro
 */

#ifndef RTSCHEDULER_H_
#define RTSCHEDULER_H_
#include <cstdlib>
#include <iostream>
#include <Task.cc>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_TASKS 10
#define SCHEDULER_FREQUENCY 5 // Number of seconds between each run of the scheduler

// Program globals
vector<Task*> tasks; // Ordered by priority, high to low
pthread_mutex_t taskListMutex;

vector<Task*> missedTasks;

int algorithm = -1; // Number used to select the scheduling algorithm

time_t startTime;

/* Method run by a task during execution */
void* execute(void* task);

/* Set up a new thread */
void createThread(pthread_t* thread, Task* task);

/* Assign priorities to all tasks based on EDF algorithm */
void EDF();

/* Assign priorities to all tasks based on SCT algorithm */
void* SCT();

/* Assign priorities to all tasks based on LST algorithm */
void LST();

/* Schedule tasks using user-specified algorithm */
void scheduleTasks();

/* Return elapsed time since start of scheduler loop in seconds */
double elapsedTime();

int timeInPeriod(int period);

/* Start or stop all tasks */
void toggleAll(bool run);

/* Scheduler loop  */
int main(int argc, char *argv[]);

#endif /* RTSCHEDULER_H_ */
