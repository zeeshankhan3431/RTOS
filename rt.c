#define _GNU_SOURCE  // Required for CPU affinity
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <float.h>  // For FLT_MAX, FLT_MIN

#define NUM_CORES 4   // Total cores (4 tasks)
#define NUM_RUNS 20   // Each task runs 20 times

// Function to get current time in decimal seconds
double get_time_in_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;  // Convert seconds + nanoseconds to decimal
}

// Store execution times for latency calculation
double latencies[NUM_CORES * NUM_RUNS];  // Array to store latencies

// Function to perform task assigned to each thread
void* run_task(void* arg) {
    int core_id = *(int*)arg;  // Get core number
    double start_time, end_time;

    // Set CPU affinity: Bind thread to specific core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);          // Clear CPU set
    CPU_SET(core_id, &cpuset);  // Add core_id to CPU set
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Execute the task 20 times
    for (int i = 0; i < NUM_RUNS; i++) {
        start_time = get_time_in_seconds();  // Start time

        // Simulate a task (e.g., some computation or workload)
        usleep(10000 * (core_id + 1));  // Simulated workload: Adjust for each core

        end_time = get_time_in_seconds();  // End time
        double latency = end_time - start_time;
        
        // Store latency in the array
        latencies[core_id * NUM_RUNS + i] = latency;

        // Print core number and execution time in seconds
        printf("Core %d: Execution Time = %.6f seconds\n", core_id, latency);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_CORES];  // Array to store thread IDs
    int core_ids[NUM_CORES];       // Core numbers

    // Create 4 threads for 4 cores
    for (int i = 0; i < NUM_CORES; i++) {
        core_ids[i] = i;  // Assign core ID (0, 1, 2, 3)
        pthread_create(&threads[i], NULL, run_task, &core_ids[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_CORES; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calculate max, min, and average latency
    double max_latency = -FLT_MAX;  // Initialize to the lowest possible value
    double min_latency = FLT_MAX;   // Initialize to the highest possible value
    double total_latency = 0.0;

    for (int i = 0; i < NUM_CORES * NUM_RUNS; i++) {
        if (latencies[i] > max_latency) {
            max_latency = latencies[i];
        }
        if (latencies[i] < min_latency) {
            min_latency = latencies[i];
        }
        total_latency += latencies[i];
    }

    double average_latency = total_latency / (NUM_CORES * NUM_RUNS);

    // Print the conclusion: max, min, and average latency
    printf("\nConclusion:\n");
    printf("Max Latency: %.6f seconds\n", max_latency);
    printf("Min Latency: %.6f seconds\n", min_latency);
    printf("Average Latency: %.6f seconds\n", average_latency);

    return 0;
}

