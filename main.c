#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMBER_OF_CPUS 4
#define CPU_QUEUE_CAPACITY 10

char message[] = "Hello World";
int thread_finished = 0;
pthread_mutex_t io_mutex;

typedef struct {
    pid_t pid;
    int static_priority;
    int dynamic_priority;
    int expected_execution_time;
    int accumulated_execution_time;
    int last_cpu;
} task_t;

typedef struct {
    int head;
    int size;
    task_t queue[CPU_QUEUE_CAPACITY];
} thread_cpu_t;

thread_cpu_t cpu[NUMBER_OF_CPUS];

void print_task(task_t task) {
    pthread_mutex_lock(&io_mutex);
    printf("PID:\t\t%d\n", task.pid);
    printf("SPriority:\t%d\n", task.static_priority);
    printf("DPriority:\t%d\n", task.dynamic_priority);
    printf("ETime:\t\t%d\n", task.expected_execution_time);
    printf("ATime:\t\t%d\n", task.accumulated_execution_time);
    printf("CPU:\t\t%d\n", task.last_cpu);
    pthread_mutex_unlock(&io_mutex);
}

void *consumer(void *arg) {
    int cpu_number = (int) arg;
    thread_cpu_t cpu_queue = cpu[cpu_number];
    printf("Hello from consumer %d\n", cpu_number);
    sleep(4);
    while(cpu_queue.size < 1) {
        sleep(1);
    }
    
    task_t task = cpu_queue.queue[0];
    task.last_cpu = cpu_number;
    print_task(task);
    
        
    printf("Bye from consumer %d\n", cpu_number);
    thread_finished += 1;
    pthread_exit(NULL);
}

void create_thread(pthread_t a_thread, int thread_id) {
    int res;
    void *thread_result;
    pthread_attr_t thread_attr;

    int max_priority;
    int min_priority;
    struct sched_param scheduling_value;
    
    res = pthread_attr_init(&thread_attr);
    if (res != 0) {
        perror("Attribute creation failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached attribute failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_create(&a_thread, &thread_attr, consumer, (void *)(long)thread_id);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    (void)pthread_attr_destroy(&thread_attr);
}

int main(int argc, char *argv[]) {
    int res;
    pthread_t threads[4];
    int num_of_threads = 0;
    task_t task;
    
    res = pthread_mutex_init(&io_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    
    task.pid = 600;
    task.static_priority = 120;
    task.dynamic_priority = 100;
    task.expected_execution_time = 40;
    task.accumulated_execution_time = 24; 
    
    for (num_of_threads; num_of_threads < 4; num_of_threads++) {
        cpu[num_of_threads].head = 0;
        cpu[num_of_threads].size = 1;
        cpu[num_of_threads].queue[0] = task;
        create_thread(threads[num_of_threads], num_of_threads);
    }
    
   
    
    while(thread_finished < num_of_threads) {
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}