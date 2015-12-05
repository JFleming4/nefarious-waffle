#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "scheduler.h"

void *producer_function(void *arg);
void *consumer_function(void *arg);
void *balancer_function(void *arg);
consumer_t cpus[NUM_OF_CPU];
int thread_finished = 0;
int main (int argc, char *argv[]) {
    int num_processes;

    if (argc < 2)
        num_processes = 20;
    else
        sscanf(argv[1], "%d", &num_processes);

    pthread_t producer, balancer;

    srand(time(NULL));
    create_thread(&producer, producer_function, num_processes);

    for(int i = 0; i < NUM_OF_CPU; i++) {
        consumer_t cpu;
        pthread_mutex_init(&cpu.mutex, NULL);
        for(int j = 0; j < NUM_QUEUE; j++){
            cpu.ready_queue[j].tail = 0;
            cpu.ready_queue[j].head = 0;
            cpu.ready_queue[j].size = 0;
            cpu.ready_queue[j].capacity = Q_CAPACITY;
        }
        cpu.size = 0;
        create_thread(&cpu.thread, consumer_function, i);
        cpus[i] = cpu;
    }

    create_thread(&balancer, balancer_function, NULL);

    while(thread_finished < 6){
        printf("Waiting for %d threads to say they finished...\n", 5 - thread_finished);
        sleep(5);
    }
}

int get_priority(void) {
    int hold = rand() %5;
    if(hold == 0) {
        return rand() % 100;
    }
    else {
        return rand() %40 + 100;
    }
}

long time_in_millis(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000) + (now.tv_usec / 1000);
}

process_info_t init_process(void) {



    process_info_t process;

    process.pid = rand() % 1000 + 1000;
    process.start_time = time_in_millis();
    process.static_priority = get_priority();
    process.sleep_avg = 0;
    process.last_execution = 0;
    process.dynamic_priority = process.static_priority;
    process.expected_execution_time = 200 + rand() % 2000;
    process.accumulated_execution_time = 0;
    return process;
}

void transfer_process(int from_index, int to_index) {
    process_info_t task;
    consumer_t *from_cpu = &cpus[from_index];
    consumer_t *to_cpu = &cpus[to_index];
    cpu_queue_t *from_queue, *to_queue;

    pthread_mutex_lock(&from_cpu->mutex);
    if (from_cpu->ready_queue[2].size > 0)
        from_queue = &from_cpu->ready_queue[2];
    else if (from_cpu->ready_queue[1].size > 0)
        from_queue = &from_cpu->ready_queue[1];
    else if (from_cpu->ready_queue[0].size > 0)
        from_queue = &from_cpu->ready_queue[0];
    else {
        pthread_mutex_unlock(&from_cpu->mutex);
        return;
    }

    task = from_queue->queue[from_queue->tail];
    from_queue->tail--;
    from_queue->size--;
    from_cpu->size--;

    pthread_mutex_unlock(&from_cpu->mutex);

    pthread_mutex_lock(&to_cpu->mutex);
    if (task.dynamic_priority < 100)
        to_queue = &to_cpu->ready_queue[0];
    else if (task.dynamic_priority < 130)
        to_queue = &to_cpu->ready_queue[1];
    else
        to_queue = &to_cpu->ready_queue[2];

    to_queue->queue[to_queue->tail] = task;
    to_queue->tail = (to_queue->tail + 1) % Q_CAPACITY;
    to_queue->size++;
    to_cpu->size++;

    pthread_mutex_unlock(&to_cpu->mutex);
}

void *balancer_function(void *arg) {
    int total_size, max_index, min_index;
    printf("Hi i'm the balancer\n");

    do {
        total_size = 0;
        max_index = 0;
        min_index = 0;
        for (int i = 0; i < NUM_OF_CPU; i++) {
            total_size += cpus[i].size;
            if (i == 0) continue;

            if (cpus[max_index].size < cpus[i].size)
                max_index = i;
            else if (cpus[min_index].size > cpus[i].size)
                min_index = i;
        }

        while (cpus[max_index].size - cpus[min_index].size > 2) {
            printf("Transferring from cpu %d to cpu %d\n", max_index, min_index);
            transfer_process(max_index, min_index);
        }

    } while (total_size > 0);
    thread_finished += 1;
    pthread_exit(NULL);
}

void *producer_function(void *arg){
    int num_processes = (int) arg;
    int cpu_num = 0;
    process_info_t process;
    cpu_queue_t * process_queue;
    printf("Hi i'm the producer\n");

    for(int i = 0; i < num_processes; i ++) {
        process = init_process();

        if(cpu_num >= NUM_OF_CPU)
            cpu_num = 0;

        //CRITICAL SECTION
        pthread_mutex_lock(&cpus[cpu_num].mutex);

        if (process.dynamic_priority < 100)
            process_queue = &cpus[cpu_num].ready_queue[0];
        else
            process_queue = &cpus[cpu_num].ready_queue[1];

        if(process_queue->size == process_queue->capacity) {
            pthread_mutex_unlock(&cpus[cpu_num].mutex);
            printf("CPU %d full\n", cpu_num);
            cpu_num++;
            i--;
            continue;
        }
        process_queue->queue[process_queue->tail] = process;
        process_queue->tail = (process_queue->tail + 1) % Q_CAPACITY;
        cpus[cpu_num].size++;
        process_queue->size++;

        printf("Process with id: %d and static priority: %d, put in cpu: %d\n",
               process.pid,
               process.static_priority,
               cpu_num);
        pthread_mutex_unlock(&cpus[cpu_num].mutex);
        cpu_num++;
    }

    thread_finished = 1;
    pthread_exit(NULL);
}

int min(int a, int b) {
    return (a > b) ? b : a;
}

int max(int a, int b) {
    return (a >= b) ? a : b;
}

int quantum(int priority) {
    if (priority < 120)
        return (140 - priority) * 20;
    return (140 - priority) * 5;
}

void *consumer_function(void *arg){
    int cpu_num = (int) arg;
    consumer_t *cpu = &cpus[cpu_num];
    cpu_queue_t *cpu_queue;
    process_info_t task;
    long start_time, end_time;
    int execution_time;
    int sleep_avg;


    printf("Hi i'm the consumer %d\n", cpu_num);
    while(!thread_finished || cpu->size !=0){
        //CRITICAL SECTION
        pthread_mutex_lock(&cpu->mutex);
        if (cpu->ready_queue[0].size > 0)
            cpu_queue = &cpu->ready_queue[0];
        else if (cpu->ready_queue[1].size > 0)
            cpu_queue = &cpu->ready_queue[1];
        else if (cpu->ready_queue[2].size > 0)
            cpu_queue = &cpu->ready_queue[2];
        else {
            pthread_mutex_unlock(&cpu->mutex);
            sleep(1);
            continue;
        }
        task = cpu_queue->queue[cpu_queue->head];
        cpu_queue->head = (cpu_queue->head + 1) % Q_CAPACITY;
        cpu_queue-> size--;
        cpu->size--;
        pthread_mutex_unlock(&cpu->mutex);
        start_time = time_in_millis();
        sleep_avg = (start_time - task.last_execution) / 200;
        task.sleep_avg = min(10, sleep_avg + task.sleep_avg);

        execution_time = min(quantum(task.static_priority), task.expected_execution_time - task.accumulated_execution_time);

        start_time = time_in_millis();
        usleep(execution_time * 1000);
        end_time = time_in_millis();

        task.accumulated_execution_time += end_time - start_time;

        if (task.accumulated_execution_time < task.expected_execution_time) {
            sleep_avg = (end_time - start_time) / 200;
            task.sleep_avg = max(0, task.sleep_avg - sleep_avg);

            task.dynamic_priority = max(100, min(139, task.dynamic_priority - task.sleep_avg + 5));

            //CRITICAL SECTION
            pthread_mutex_lock(&cpu->mutex);

            if (task.dynamic_priority < 100)
                cpu_queue = &cpu->ready_queue[0];
            else if (task.dynamic_priority < 130)
                cpu_queue = &cpu->ready_queue[1];
            else
                cpu_queue = &cpu->ready_queue[2];

            cpu_queue->queue[cpu_queue->tail] = task;
            cpu_queue->tail = (cpu_queue->tail + 1) % Q_CAPACITY;
            cpu_queue-> size++;
            cpu->size++;

            pthread_mutex_unlock(&cpu->mutex);
            continue;
        }
        printf("CPU %d finished process %d in %d ms\n",
               cpu_num,
               task.pid,
               task.accumulated_execution_time);
    }

    thread_finished += 1;
    pthread_exit(NULL);
}
