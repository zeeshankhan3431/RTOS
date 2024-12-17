#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 20

// Structure to store thread arguments
typedef struct {
    int core_num;
    const char* task_name;
} thread_args_t;

// Task functions
void* add_task(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        clock_t start_time = clock();
        int result = a + b;
        clock_t end_time = clock();
        double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Core %d - Task: ADD - Result: %d + %d = %d\n", args->core_num, a, b, result);
        printf("Core %d - Iteration %d: Execution Time = %.6f seconds\n", args->core_num, i + 1, execution_time);
    }
    return NULL;
}

void* subtract_task(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        clock_t start_time = clock();
        int result = a - b;
        clock_t end_time = clock();
        double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Core %d - Task: SUBTRACT - Result: %d - %d = %d\n", args->core_num, a, b, result);
        printf("Core %d - Iteration %d: Execution Time = %.6f seconds\n", args->core_num, i + 1, execution_time);
    }
    return NULL;
}

void* multiply_task(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        clock_t start_time = clock();
        int result = a * b;
        clock_t end_time = clock();
        double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Core %d - Task: MULTIPLY - Result: %d * %d = %d\n", args->core_num, a, b, result);
        printf("Core %d - Iteration %d: Execution Time = %.6f seconds\n", args->core_num, i + 1, execution_time);
    }
    return NULL;
}

void* divide_task(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int a = rand() % 100;
        int b = (rand() % 99) + 1; // Ensure b is not 0
        clock_t start_time = clock();
        double result = (double)a / b;
        clock_t end_time = clock();
        double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Core %d - Task: DIVIDE - Result: %d / %d = %.2f\n", args->core_num, a, b, result);
        printf("Core %d - Iteration %d: Execution Time = %.6f seconds\n", args->core_num, i + 1, execution_time);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    thread_args_t args[NUM_THREADS];

    // Set the CPU affinity and assign tasks to cores
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].core_num = i + 1;
        if (i == 0) {
            args[i].task_name = "ADD";
            pthread_create(&threads[i], NULL, add_task, &args[i]);
        } else if (i == 1) {
            args[i].task_name = "SUBTRACT";
            pthread_create(&threads[i], NULL, subtract_task, &args[i]);
        } else if (i == 2) {
            args[i].task_name = "MULTIPLY";
            pthread_create(&threads[i], NULL, multiply_task, &args[i]);
        } else if (i == 3) {
            args[i].task_name = "DIVIDE";
            pthread_create(&threads[i], NULL, divide_task, &args[i]);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

