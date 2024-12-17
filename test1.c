#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define NUM_CORES 4   // Number of cores/tasks
#define NUM_RUNS 20   // Each task runs 20 times

// Function to get current time in seconds (decimal)
double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Task 1: Add two numbers
void task_add(int core_id) {
    int a = rand() % 100, b = rand() % 100;  // Generate two random numbers
    printf("Core %d - Task: ADD - Result: %d + %d = %d\n", core_id, a, b, a + b);
}

// Task 2: Subtract two numbers
void task_subtract(int core_id) {
    int a = rand() % 100, b = rand() % 100;
    printf("Core %d - Task: SUBTRACT - Result: %d - %d = %d\n", core_id, a, b, a - b);
}

// Task 3: Multiply two numbers
void task_multiply(int core_id) {
    int a = rand() % 100, b = rand() % 10;
    printf("Core %d - Task: MULTIPLY - Result: %d * %d = %d\n", core_id, a, b, a * b);
}

// Task 4: Divide two numbers
void task_divide(int core_id) {
    int a = rand() % 100 + 1, b = rand() % 10 + 1;  // Avoid zero division
    printf("Core %d - Task: DIVIDE - Result: %d / %d = %.2f\n", core_id, a, b, (double)a / b);
}

// Function to execute task and print execution time
void* run_task(void* arg) {
    int core_id = *(int*)arg;  // Get the core ID
    double start_time, end_time;

    // Set CPU affinity to bind thread to specific core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    srand(time(NULL) + core_id);  // Seed random number generator uniquely for each core

    // Execute the task 20 times
    for (int i = 0; i < NUM_RUNS; i++) {
        start_time = get_time_in_seconds();

        // Perform task based on core ID
        switch (core_id) {
            case 0: task_add(core_id); break;
            case 1: task_subtract(core_id); break;
            case 2: task_multiply(core_id); break;
            case 3: task_divide(core_id); break;
        }

        end_time = get_time_in_seconds();
        printf("Core %d - Iteration %d: Execution Time = %.6f seconds\n", core_id, i + 1, end_time - start_time);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_CORES];  // Array to store thread IDs
    int core_ids[NUM_CORES];       // Core numbers

    // Create 4 threads for 4 tasks (each on a separate core)
    for (int i = 0; i < NUM_CORES; i++) {
        core_ids[i] = i;  // Core ID (0, 1, 2, 3)
        pthread_create(&threads[i], NULL, run_task, &core_ids[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_CORES; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All tasks completed successfully.\n");
    return 0;
}

