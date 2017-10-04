/* Scheduler program created by Graham Home and Alberto Santos-Castro */
#include <RTScheduler.h>
#include <algorithm>
using namespace std;

/* Set up a new thread */
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

/* Return elapsed time since start of scheduler loop in seconds */
int elapsedTime() {
	return (int)(difftime(time(NULL), startTime) + 0.5); // Ensure value is properly rounded
}

/* Method run by a task during execution */
void* execute(void* t) {
	Task* task = (Task*)t;
	int lastRunTime = 0;
	int currentTime = elapsedTime();
	while(elapsedTime() <= runTime) {
		// Has the task already completed during this period?
		while (!(task->completed)) {
			currentTime = elapsedTime();

			while(currentTime == lastRunTime)
				currentTime = elapsedTime();
			if(elapsedTime() > runTime)
				break;
				// Suspend until allowed to run
				pthread_mutex_lock(&(task->activeMutex));
				while (!(task->active)) pthread_cond_wait(&(task->activeCondition), &(task->activeMutex));
				pthread_mutex_unlock(&(task->activeMutex));
				currentTime = elapsedTime();
				lastRunTime = currentTime;

				task->rem_exec_time -= 1;
				// Did we miss the deadline?
				if (((currentTime % task->period == 0 )||(currentTime % task->period >= task->deadline))&&(task->rem_exec_time != 0)) {
					if(PRINT_ACTIVE) printf("\n*** %s Missed the deadline! after %d seconds", task->name.c_str(), currentTime);
					if (find(missedTasks.begin(), missedTasks.end(), task) == missedTasks.end()) { // If the task is not already in the list of missed-deadline tasks, add it.
						missedTasks.push_back(task);
					}
				}

				if (task->rem_exec_time == 0) {
					task->completed = true;
					//if(PRINT_ACTIVE)printf("\n-->%s Completed after %d seconds", task->name.c_str(), currentTime);
				}

				if (elapsedTime() % task->period == 0 ){
					task->rem_exec_time = task->total_exec_time;
				}
		}
		while (task->completed) {
			if(elapsedTime() > runTime)
				break;

			if (elapsedTime()% task->period == 0) {
				task->completed = false;
				task->rem_exec_time = task->total_exec_time;
			}
			currentTime = elapsedTime();
			lastRunTime = currentTime;
		}
	}
	return NULL;
}

/* Schedule tasks using user-specified algorithm */
void scheduleTasks(Task* active_task){
	vector<Task*> orderedTasks;
	Task* task;
	int elapsed_time = elapsedTime();

	switch(algorithm){
	case EDF:
		for (unsigned int i=0;i<tasks.size();i++){
			task = tasks.at(i);
			unsigned int t;
			// Insert completed tasks at the end of the list
			if(task->completed){
				orderedTasks.push_back(task);
			} else {
				for (t=0;t<orderedTasks.size();t++) {
					if ((orderedTasks.at(t)->completed)||((orderedTasks.at(t)->deadline - (elapsed_time % orderedTasks.at(t)->period)) > (task->deadline-(elapsed_time % task->period)))) {
						break;
					}
				}
				orderedTasks.insert(orderedTasks.begin()+t, task);
			}
		}
		break;
	case SCT:
		for (unsigned int i=0;i<tasks.size();i++){
			Task* task = tasks.at(i);
			unsigned int t;
			// Insert completed tasks at the end of the list
			if(task->completed){
				orderedTasks.push_back(task);
			} else {
				for (t=0;t<orderedTasks.size();t++) {
					if ((orderedTasks.at(t)->completed)||(orderedTasks.at(t)->rem_exec_time > task->rem_exec_time)) {
						break;
					}
				}
				orderedTasks.insert(orderedTasks.begin()+t, task);
			}
		}
		break;
	case LST:
		for (unsigned int i=0;i<tasks.size();i++){
			Task* task = tasks.at(i);
			unsigned int t;
			// Insert completed tasks at the end of the list
			if(task->completed){
				orderedTasks.push_back(task);
			} else {
				for (t=0;t<orderedTasks.size();t++) {
					if ((orderedTasks.at(t)->completed)||(orderedTasks.at(t)->deadline-orderedTasks.at(t)->rem_exec_time > task->deadline-task->rem_exec_time)) {
						break;
					}
				}
				orderedTasks.insert(orderedTasks.begin()+t, task);
			}
		}
		break;
	}

	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&taskListMutex);
	tasks = orderedTasks;
	pthread_mutex_unlock(&taskListMutex);
}

/* Scheduler loop  */
int main(int argc, char *argv[]) {
	// Set up mutexes
	pthread_mutex_init(&taskListMutex, NULL);

	// Get scheduling algorithm
	printf("Welcome to the Real-Time Scheduler\n");
	printf("Scheduling algorithm choices:\n(0) Earliest Deadline First\n(1) Shortest Completion Time\n(2) Least Slack Time\n");
	printf("Choose a scheduling algorithm (enter the number of your choice): ");
	while (!(algorithm>=0 && algorithm <=2)) {
		cin >> algorithm;
	}

	printf("You selected ");
	switch(algorithm) {
	case EDF:
			printf("Earliest Deadline First\n");
			break;
	case SCT:
			printf("Shortest Completion Time\n");
			break;
	case LST:
			printf("Least Slack Time\n");
			break;
	}

	// Get tasks
	printf("\nEnter tasks in the format \'<name> <execution time> <deadline> <period>\' and follow each with a newline.\nTo run tasks, enter \'start\' followed by a newline.\n");
	string input;
	getline(cin, input); // Ignore newline from previous println
	bool done = false;
	while (!done) {
		getline(cin, input);
		if (input == "start") {
			done = true;
			break;
		}else if(input == "test" || input == "0"){
			//Default values for debugging purposes
			pthread_t* myThread1 = NULL;
			pthread_t* myThread2 = NULL;
			pthread_t* myThread3 = NULL;
			Task* task1 = new Task("Task1", 1, 1, 1, myThread1);
			Task* task2 = new Task("Task2", 2, 2, 2, myThread2);
			Task* task3 = new Task("Task3", 3, 3, 3, myThread3);
			tasks.push_back(task1);
			tasks.push_back(task2);
			tasks.push_back(task3);
			break;
		}
		string name;
		int c;
		int p;
		int d;
		int tokenCount = 0;
		size_t pos = 0;
		while (((pos = input.find(" ")) <= string::npos) && tokenCount < 4) {
			switch(tokenCount++) {
			case 0:
				name = input.substr(0, pos);
				break;
			case 1:
				c = atoi(input.substr(0, pos).c_str());
				break;
			case 2:
				p = atoi(input.substr(0, pos).c_str());
				break;
			case 3:
				d = atoi(input.substr(0, pos).c_str());
				break;
			default:
				break;
			}
			input.erase(0, pos+1);
		}
		if (tokenCount != 4) {
			printf("Invalid task format.\n");
		} else {
			pthread_t* taskThread = NULL;
			Task* task = new Task(name, c, p, d, taskThread);
			tasks.push_back(task);
			tokenCount = 0;
		}
	}

	//Scheduler section
	// Get run duration
	printf("How many seconds would you like to run the simulation for? ");
	cin >> runTime;
	printf("Running tasks...\n");


	// Get start time
	time(&startTime);
	if(PRINT_ACTIVE)printf("\n-------------------------------\n START:");
	// Set highest priority task active
	scheduleTasks(NULL);
	Task* runningTask = tasks.at(0);
	// Start all task threads (inactive by default)
	for (unsigned int i=0;i<tasks.size();i++) {
		Task* t = tasks.at(i);
		createThread(t->thread, t);
		if(PRINT_ACTIVE)printf("\n%s Thread created!", t->name.c_str());
	}
	runningTask->setActive(true);
	if(PRINT_ACTIVE)printf("\n--> %s Active task! at time %d", (runningTask->name).c_str(), elapsedTime());
	int lastCheckedTime = elapsedTime();
	int r_count = 0;
	while(elapsedTime() <= runTime){
		// Scheduler runs periodically
		int currentTime = elapsedTime();
		while(currentTime == lastCheckedTime)
			currentTime = elapsedTime();

			r_count++;
			scheduleTasks(runningTask);

			if (tasks.at(0)->completed){
				runningTask->setActive(false);
				printf("\nAt time %d all tasks completed!", elapsedTime());
			}
			else if (tasks.at(0) != runningTask) {
				// Stop running task
				runningTask->setActive(false);
				//if(PRINT_ACTIVE)printf("\n*** %s No longer running!", (runningTask->name).c_str());
				// Start highest priority task
				runningTask = tasks.at(0);
				runningTask->setActive(true);
				if(PRINT_ACTIVE)printf("\n--> %s Active task at time %d!", (runningTask->name).c_str(), currentTime);
			}

		lastCheckedTime = currentTime;
	}
	// Stop all tasks
	for (unsigned int i=0;i<tasks.size();i++) {
		tasks.at(i)->setActive(false);
	}

	if(PRINT_ACTIVE)printf("\n-------------------------------\n RESULTS:");
	printf("\nRescheduled %d times\n", r_count);
	if (missedTasks.size() > 0) {
		for (unsigned int i=0;i<missedTasks.size();i++) {
			printf("Task %s missed its deadline.\n", (missedTasks.at(i)->name).c_str());
		}
	} else {
		printf("All tasks ran for %d seconds without missing any deadlines.\n", runTime);
	}

	return EXIT_SUCCESS;
}
