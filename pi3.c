#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define NSEC_PER_SEC 1000000000ULL   // Nanoseconds per second
#define RT_PRIORITY 99               // Real-time priority
#define NUM_ITERATIONS 100           // Number of task executions

void set_realtime_priority() {
    struct sched_param param;
    param.sched_priority = RT_PRIORITY;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("Failed to set real-time scheduling policy");
        exit(EXIT_FAILURE);
    }
    printf("Real-time priority set to %d using SCHED_FIFO policy.\n", RT_PRIORITY);
}

unsigned long long timespec_to_ns(struct timespec *ts) {
    return (unsigned long long)ts->tv_sec * NSEC_PER_SEC + ts->tv_nsec;
}

void print_task_results(int iteration, unsigned long long exec_time, unsigned long long min, unsigned long long max) {
    printf("Iteration: %d, Execution Time: %llu ns, Min Latency: %llu ns, Max Latency: %llu ns\n", 
           iteration, exec_time, min, max);
}

void rt_task(int iterations) {
    struct timespec start, end, task_start, task_end;
    unsigned long long min_latency = NSEC_PER_SEC, max_latency = 0;
    unsigned long long exec_time, jitter;

    printf("\nStarting real-time task for %d iterations...\n", iterations);

    clock_gettime(CLOCK_MONOTONIC, &start); // Initial time marker for the program

    for (int i = 1; i <= iterations; i++) {
        clock_gettime(CLOCK_MONOTONIC, &task_start);

        // Simulate task workload
        usleep(1000); // Sleep for 1 ms to simulate real-time task processing

        clock_gettime(CLOCK_MONOTONIC, &task_end);

        // Calculate execution time
        exec_time = timespec_to_ns(&task_end) - timespec_to_ns(&task_start);

        // Update min and max latency
        if (exec_time < min_latency) min_latency = exec_time;
        if (exec_time > max_latency) max_latency = exec_time;

        // Print results for each iteration
        print_task_results(i, exec_time, min_latency, max_latency);

        // Small delay between iterations
        struct timespec delay = {0, 5000000}; // 5 ms delay
        nanosleep(&delay, NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // Final time marker

    unsigned long long total_time = timespec_to_ns(&end) - timespec_to_ns(&start);
    printf("\nReal-time task completed.\n");
    printf("Total Execution Time: %llu ns\n", total_time);
    printf("Final Min Latency: %llu ns\n", min_latency);
    printf("Final Max Latency: %llu ns\n", max_latency);
    printf("Final Jitter: %llu ns\n", max_latency - min_latency);
}

int main() {
    printf("Real-Time Task Verification Program\n");

    // Step 1: Set real-time priority
    set_realtime_priority();

    // Step 2: Bind the task to a specific CPU core (optional)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset); // Bind task to CPU core 1

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        perror("Failed to set CPU affinity");
        exit(EXIT_FAILURE);
    }
    printf("Task bound to CPU core 1.\n");

    // Step 3: Run the real-time task
    rt_task(NUM_ITERATIONS);

    return 0;
}

