/* Scheduler program created by Graham Home and Alberto Castro */
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

/* Assign priorities to all tasks based on EDF algorithm */
void EDF(){
	vector<Task*> orderedTasks;
	for (int i=0;i<tasks.size();i++){
		Task* task = tasks.at(i);
		int t;
		for (t=0;t<orderedTasks.size();t++) {
			if (orderedTasks.at(t)->deadline > task->deadline) {
				break;
			}
		}
		orderedTasks.insert(orderedTasks.begin()+t, task);
	}
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&taskListMutex);
	tasks = orderedTasks;
	pthread_mutex_unlock(&taskListMutex);
}

/* Assign priorities to all tasks based on SCT algorithm */
void SCT(){
	vector<Task*> orderedTasks;
	for (int i=0;i<NUM_TASKS;i++){
		Task* task = tasks.at(i);
		int t;
		for (t=0;t<orderedTasks.size();t++) {
			if (orderedTasks.at(t)->rem_exec_time > task->rem_exec_time) {
				break;
			}
		}
		orderedTasks.insert(orderedTasks.begin()+t, task);
	}
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&taskListMutex);
	tasks = orderedTasks;
	pthread_mutex_unlock(&taskListMutex);
}

/* Assign priorities to all tasks based on LST algorithm */
void LST(){
	vector<Task*> orderedTasks;
	for (int i=0;i<NUM_TASKS;i++){
		Task* task = tasks.at(i);
		int t;
		for (t=0;t<orderedTasks.size();t++) {
			if (orderedTasks.at(t)->deadline-orderedTasks.at(t)->rem_exec_time > task->deadline-task->rem_exec_time) {
				break;
			}
		}
		orderedTasks.insert(orderedTasks.begin()+t, task);
	}
	/* Lock the active tasks list mutex
	 * so no task threads will try to select
	 * the next task from the list while we are rebuilding it
	 */
	pthread_mutex_lock(&taskListMutex);
	tasks = orderedTasks;
	pthread_mutex_unlock(&taskListMutex);
}

/* Schedule tasks using user-specified algorithm */
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

/* Return elapsed time since start of scheduler loop in seconds */
int elapsedTime() {
	return (int)(difftime(time(NULL), startTime) + 0.5); // Ensure value is properly rounded
}

/* Method run by a task during execution */
void* execute(void* t) {
	Task* task = (Task*)t;
	int periodCount = 0;
	while(elapsedTime() < runTime) {
		// Has the task already completed during this period?
		int lastRunTime = 0;
		while (!(task->completed)) {
			int currentTime = elapsedTime();
			if (currentTime != lastRunTime) {
				lastRunTime = currentTime;
				// Suspend until allowed to run
				pthread_mutex_lock(&(task->activeMutex));
				while (!(task->active)) pthread_cond_wait(&(task->activeCondition), &(task->activeMutex));
				pthread_mutex_unlock(&(task->activeMutex));
				// Did we miss the deadline?
				if (currentTime % task->period > task->deadline) { //TODO: Not detecting missed deadlines if period == deadline
					printf("Task %d missed a deadline\n", (task->name).c_str());
					if (find(missedTasks.begin(), missedTasks.end(), task) == missedTasks.end()) { // If the task is not already in the list of missed-deadline tasks, add it.
						missedTasks.push_back(task);
					}
				}
				printf("task %s counting down 1\n", (task->name).c_str());
				task->rem_exec_time -= 1;
				if (task->rem_exec_time == 0) {
					printf("Task %s completed\n", (task->name).c_str());
					task->completed = true;
					task->rem_exec_time = task->total_exec_time;
				}
			}
		}
		while (task->completed) {
			if (elapsedTime()/task->period > periodCount) {
				task->completed = false;
			}
		}
	}
	return NULL;
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

	// Get tasks
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
			tasks.push_back(task);
			tokenCount = 0;
		}
	}

	//Scheduler section

	// Get run duration
	printf("How many seconds would you like to run the simulation for? ");
	cin >> runTime;
	printf("Running tasks...\n");
	scheduleTasks();
	// Get start time
	time(&startTime);

	// Set highest priority task active
	Task* runningTask = tasks.at(0);
	runningTask->setActive(true);
	// Start all task threads (inactive by default)
	for (int i=0;i<tasks.size();i++) {
		Task* t = tasks.at(i);
		createThread(t->thread, t);
	}
	int lastCheckedTime = elapsedTime();
	int r_count = 0;
	while(elapsedTime() < runTime){
		// Scheduler runs periodically
		int currentTime = elapsedTime();
		if ((currentTime != lastCheckedTime) && (currentTime % SCHEDULER_FREQUENCY == 0) ) {
			r_count++;
			scheduleTasks();
			if (tasks.at(0) != runningTask) {
				// Stop running task
				runningTask->setActive(false);
				// Start highest priority task
				runningTask = tasks.at(0);
				runningTask->setActive(true);
			}
		}
		lastCheckedTime = currentTime;
	}
	printf("Rescheduled %d times\n", r_count);
	if (missedTasks.size() > 0) {
		for (int i=0;i<missedTasks.size();i++) {
			printf("Task %s missed its deadline.\n", (missedTasks.at(i)->name).c_str());
		}
	} else {
		printf("All tasks ran for %d seconds without missing any deadlines.\n", runTime);
	}

	return EXIT_SUCCESS;
}
