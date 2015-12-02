#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_OF_CPU 4
#define Q_CAPACITY 30
#define NUM_QUEUE 3
#define MAX_SLEEP 10
typedef struct {
    int pid;
    long start_time;
    int sleep_avg;
    long last_execution;
    int static_priority;
    int dynamic_priority;
    int expected_execution_time;
    int accumulated_execution_time;
    int last_cpu;
}process_info_t;
    
typedef struct {
    process_info_t queue[Q_CAPACITY];
    int tail;
    int head;
    int capacity;
    int size;
}cpu_queue_t;

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    cpu_queue_t ready_queue[NUM_QUEUE];
    int size;
    
}consumer_t;



void create_thread(pthread_t *thread, void * func, void * mes) {
    int max_priority;
    int min_priority;
    int res;
    pthread_attr_t thread_attr;
    
    res = pthread_attr_init(&thread_attr);
    if (res != 0) {
        perror("Attribute creation failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached attribute failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_create(thread, &thread_attr, func, (void *)mes);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
}