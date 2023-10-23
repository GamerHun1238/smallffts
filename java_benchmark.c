#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>

int NUM_THREADS;
int NUM_ITERATIONS;
#define DATA_SIZE 1024

int* workDone;
bool stopThreads = false;
int totalVal = 0;
struct timeval startTime, endTime;

double* generateRandomData(int size) {
    double* data = (double*)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        data[i] = (double)rand() / RAND_MAX;
    }
    return data;
}

void performFFT(double* data) {
    int n = DATA_SIZE;

    for (int m = 0; m < n; m++) {
        double sumReal = 0;
        double sumImag = 0;
        for (int k = 0; k < n; k++) {
            double angle = 2.0 * M_PI * k * m / n;
            sumReal += data[k] * cos(angle);
            sumImag -= data[k] * sin(angle);
        }
    }
}

void* threadFunction(void* arg) {
    int threadIndex = *(int*)arg;
    for (int j = 0; j < NUM_ITERATIONS; j++) {
        double* data = generateRandomData(DATA_SIZE);
        performFFT(data);
        free(data);
        workDone[threadIndex]++;
        if (stopThreads) {
            return NULL; // Stop the thread if the flag is set
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <numThreads> <numIterations>\n", argv[0]);
        return 1;
    }

    NUM_THREADS = atoi(argv[1]);
    NUM_ITERATIONS = atoi(argv[2]);
    
    if (NUM_THREADS <= 0 || NUM_ITERATIONS <= 0) {
        printf("Invalid number of threads or iterations\n");
        return 1;
    }

    workDone = (int*)malloc(NUM_THREADS * sizeof(int));

    pthread_t threads[NUM_THREADS];
    int threadIndices[NUM_THREADS];

    gettimeofday(&startTime, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        threadIndices[i] = i;
        if (pthread_create(&threads[i], NULL, threadFunction, &threadIndices[i]) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    atexit(free); // Cleanup allocated memory on exit

    printf("Startup.\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&endTime, NULL);

    double timeTook = (double)(endTime.tv_sec - startTime.tv_sec) + (double)(endTime.tv_usec - startTime.tv_usec) / 1000000.0;

    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Thread %d work done: %d\n", i, workDone[i]);
        totalVal += workDone[i];
    }

    double itPerSecond = (double)totalVal / timeTook;

    printf("Total iterations done: %d\n", totalVal);
    printf("Time took in seconds: %f\n", timeTook);

    // Format the "Iterations per second" output for better readability
    if (timeTook > 0) {
        double itPerMillisecond = itPerSecond / 1000;

        printf("Iterations per second: %.6f (%.6f per millisecond)\n", itPerSecond, itPerMillisecond);
    } else {
        printf("Iterations per second: N/A\n");
    }

    printf("All threads have exited.\n");

    return 0;
}
