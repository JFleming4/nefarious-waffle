#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "scheduler.h"

void *producer_function(void *arg);
void *consumer_function(void *arg);
consumer_t cpus[NUM_OF_CPU];
int thread_finished = 0;
int main (void) {
    pthread_t producer;
    
    
    srand(time(NULL));
    create_thread(&producer, producer_function,NULL);
    
    for(int i = 0; i < NUM_OF_CPU; i++) {
        consumer_t cpu;
        pthread_mutex_init(&cpu.mutex, NULL);
        cpu.tail = 0;
        cpu.head = 0;
        cpu.size = 0;
        cpu.capacity = Q_CAPACITY;
        create_thread(&cpu.thread, consumer_function, i);
        cpus[i] = cpu;
    }
    while(!thread_finished){
        printf("Waiting for thread to say it's finished...\n");
        sleep(1);
    }
}

void *producer_function(void *arg){
    int cpu_num = 0;
    process_info_t process;
    printf("Hi i'm the producer\n");
    
    while (1) {
        process.priority =  rand() % 98 + 1;
        process.pid = rand() % 1000 + 1000;
        if(cpu_num >= NUM_OF_CPU){
            cpu_num = 0;
        }
        //CRITICAL SECTION
        pthread_mutex_lock(&cpus[cpu_num].mutex);
        if(cpus[cpu_num].size == cpus[cpu_num].capacity) {
            pthread_mutex_unlock(&cpus[cpu_num].mutex);
            printf("CPU %d full\n", cpu_num);
            cpu_num++;
            break;
            //continue;
        }
        cpus[cpu_num].queue[cpus[cpu_num].tail] = process;
        cpus[cpu_num].tail = (cpus[cpu_num].tail + 1) % Q_CAPACITY;
        cpus[cpu_num].size++;
        printf("Process with id: %d and priority: %d, put in cpu: %d\n", 
               process.pid,
               process.priority,
               cpu_num);
        pthread_mutex_unlock(&cpus[cpu_num].mutex);
        cpu_num++;
        
    }
    
    thread_finished = 1;
    pthread_exit(NULL);
}

void *consumer_function(void *arg){
    int cpu_num = (int) arg;
    printf("Hi i'm the consumer %d\n", cpu_num);
    pthread_exit(NULL);
}