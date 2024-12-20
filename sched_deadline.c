#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/sched.h>
#include <sys/syscall.h>

#define SIMULATION_TIME 50  // Simulation duration in time units

// Task structure
typedef struct {
    char name[10];      // Task name
    int capacity;       // Execution time (Ci)
    int deadline;       // Relative deadline (Di)
    int period;         // Period (Ti)
    int remaining_time; // Remaining execution time in the current period
    int next_deadline;  // Absolute deadline
} Task;

// sched_attr structure (if not defined in headers)
struct sched_attr {
    unsigned int size;
    unsigned int sched_policy;
    unsigned long sched_flags;
    unsigned long sched_nice;
    unsigned long sched_priority;
    unsigned long sched_runtime;
    unsigned long sched_deadline;
    unsigned long sched_period;
};

// sched_setattr syscall wrapper
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
    return syscall(314, pid, attr, flags);  // syscall number 314 for sched_setattr
}

// Initialize tasks for simulation
void initialize_tasks(Task tasks[], int n) {
    for (int i = 0; i < n; i++) {
        tasks[i].remaining_time = tasks[i].capacity;
        tasks[i].next_deadline = tasks[i].deadline;
    }
}

// Find the next task to run based on EDF
int find_next_task(Task tasks[], int n, int time) {
    int earliest_deadline = 1e9, selected_task = -1;
    for (int i = 0; i < n; i++) {
        if (tasks[i].remaining_time > 0 && tasks[i].next_deadline < earliest_deadline) {
            earliest_deadline = tasks[i].next_deadline;
            selected_task = i;
        }
    }
    return selected_task;
}

// Simulate EDF scheduling
void edf_schedule(Task tasks[], int n) {
    printf("Time\tRunning Task\n");
    printf("-------------------\n");

    for (int time = 0; time < SIMULATION_TIME; time++) {
        // Check for task arrivals
        for (int i = 0; i < n; i++) {
            if (time % tasks[i].period == 0) {  // New period
                tasks[i].remaining_time = tasks[i].capacity;
                tasks[i].next_deadline = time + tasks[i].deadline;
            }
        }

        // Select the next task to execute
        int running_task = find_next_task(tasks, n, time);
        if (running_task != -1) {
            tasks[running_task].remaining_time--;  // Execute task
            printf("%2d\t%s\n", time, tasks[running_task].name);
        } else {
            printf("%2d\tIdle\n", time);  // No task to execute
        }
    }

    // Final summary
    printf("-------------------\n");
    printf("Task Summary:\n");
    printf("Name\tCapacity\tDeadline\tPeriod\n");
    for (int i = 0; i < n; i++) {
        printf("%s\t%d\t\t%d\t\t%d\n", tasks[i].name, tasks[i].capacity, tasks[i].deadline, tasks[i].period);
    }
}

// Set SCHED_DEADLINE policy for a task
int set_task_deadline_policy(Task *task, int runtime, int deadline, int period) {
    struct sched_attr attr = {0};

    // Set the scheduling parameters
    attr.size = sizeof(attr);
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_runtime = runtime * 1e6;   // Convert to nanoseconds
    attr.sched_deadline = deadline * 1e6; // Convert to nanoseconds
    attr.sched_period = period * 1e6;    // Convert to nanoseconds

    // Apply the SCHED_DEADLINE policy
    if (sched_setattr(0, &attr, 0) == -1) {
        perror("Error setting SCHED_DEADLINE policy");
        return -1;
    }

    return 0;
}

int main() {
    int n;

    // Get the number of tasks
    printf("Enter the number of tasks: ");
    scanf("%d", &n);

    // Dynamically allocate memory for tasks
    Task *tasks = (Task *)malloc(n * sizeof(Task));

    // Get task details from the user
    for (int i = 0; i < n; i++) {
        printf("\nEnter details for Task %d:\n", i + 1);
        printf("Name: ");
        scanf("%s", tasks[i].name);
        printf("Execution Time (Capacity): ");
        scanf("%d", &tasks[i].capacity);
        printf("Deadline: ");
        scanf("%d", &tasks[i].deadline);
        printf("Period: ");
        scanf("%d", &tasks[i].period);
    }

    // Set scheduling policy for each task using SCHED_DEADLINE
    for (int i = 0; i < n; i++) {
        if (set_task_deadline_policy(&tasks[i], tasks[i].capacity, tasks[i].deadline, tasks[i].period) == -1) {
            printf("Failed to set scheduling policy for task %s\n", tasks[i].name);
        }
    }

    // Initialize and run EDF scheduling
    initialize_tasks(tasks, n);
    edf_schedule(tasks, n);

    // Free dynamically allocated memory
    free(tasks);

    return 0;
}
