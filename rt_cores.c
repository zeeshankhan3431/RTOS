#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NANOSECONDS_PER_SECOND 1000000000
#define ITERATIONS 20
#define PERIOD_NS 1000000 // Task period in nanoseconds (1 ms)

typedef struct {
    int task_no;      // Task number
    int core_id;      // Core to run on
    long latencies[ITERATIONS]; // Latency values in nanoseconds
} TaskData;

// Calculate the difference in nanoseconds between two timespecs
long calculate_latency(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * NANOSECONDS_PER_SECOND + (end.tv_nsec - start.tv_nsec);
}

// Function executed by each task
void *real_time_task(void *arg) {
    TaskData *data = (TaskData *)arg;
    struct timespec next_activation, current_time;

    // Pin task to a specific core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(data->core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Get the current time
    clock_gettime(CLOCK_MONOTONIC, &next_activation);

    for (int i = 0; i < ITERATIONS; i++) {
        // Wait until the next activation time
        next_activation.tv_nsec += PERIOD_NS;
        if (next_activation.tv_nsec >= NANOSECONDS_PER_SECOND) {
            next_activation.tv_nsec -= NANOSECONDS_PER_SECOND;
            next_activation.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_activation, NULL);

        // Get the current time and calculate latency
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        data->latencies[i] = calculate_latency(next_activation, current_time);

        // Print execution info (convert latency to seconds)
        printf("Task %d\tCore %d\t%.9f s\n", data->task_no, data->core_id, (double)data->latencies[i] / NANOSECONDS_PER_SECOND);
    }

    return NULL;
}

int main() {
    const int num_tasks = 4;
    pthread_t threads[num_tasks];
    TaskData task_data[num_tasks];

    // Initialize tasks
    for (int i = 0; i < num_tasks; i++) {
        task_data[i].task_no = i + 1;
        task_data[i].core_id = i; // Assign each task to a different core
    }

    // Create tasks
    for (int i = 0; i < num_tasks; i++) {
        if (pthread_create(&threads[i], NULL, real_time_task, &task_data[i]) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    // Wait for all tasks to finish
    for (int i = 0; i < num_tasks; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calculate and display statistics
    printf("\nCore\tMin Latency (s)\tMax Latency (s)\tAvg Latency (s)\n");
    for (int i = 0; i < num_tasks; i++) {
        long min_latency = task_data[i].latencies[0];
        long max_latency = task_data[i].latencies[0];
        long total_latency = 0;

        for (int j = 0; j < ITERATIONS; j++) {
            if (task_data[i].latencies[j] < min_latency) {
                min_latency = task_data[i].latencies[j];
            }
            if (task_data[i].latencies[j] > max_latency) {
                max_latency = task_data[i].latencies[j];
            }
            total_latency += task_data[i].latencies[j];
        }

        // Display latencies in seconds (s)
        printf("%d\t%.9f\t%.9f\t%.9f\n", i,
               (double)min_latency / NANOSECONDS_PER_SECOND,
               (double)max_latency / NANOSECONDS_PER_SECOND,
               (double)(total_latency / ITERATIONS) / NANOSECONDS_PER_SECOND);
    }

    return EXIT_SUCCESS;
}
