#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_OF_CPU 4
#define Q_CAPACITY 50
#define NUM_QUEUE 3
#define MAX_SLEEP 10

#define RR 1
#define FIFO 0
#define NORM 2

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
    int scheduling_type;
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

void print_process_info(process_info_t task) {
    const char *p;
    if (task.scheduling_type == RR)
        p = "RR";
    else if (task.scheduling_type == FIFO)
        p = "FIFO";
    else
        p = "NORMAL";
    
    printf(
        "Process %d has scheduling type of %s, static priority %d and dynamic priority %d.\nIt has a total expected execution time of %dms and has been running for %dms.\nIt was last running on CPU %d and started at %ld.\n",
        task.pid,
        p,
        task.static_priority,
        task.dynamic_priority,
        task.expected_execution_time,
        task.accumulated_execution_time,
        task.last_cpu,
        task.start_time
    );
}


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