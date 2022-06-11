//
// Created by student on 6/12/21.
//

#ifndef HW3_ASEEL_PCQUEUE_H
#define HW3_ASEEL_PCQUEUE_H

#include "segel.h"
#include "Queue.h"

// Single Producer - Multiple Consumer queue

typedef struct pcQueue{
    Queue* queue;
    int num_producers;
    pthread_cond_t wait;
    pthread_mutex_t lock;
}PCQueue;

PCQueue* createPCQueue(int size, char* schedalg){
    PCQueue* q = (PCQueue*)malloc(sizeof(PCQueue));
    q->queue = createQueue(size);
    q->num_producers = 0;
    pthread_mutex_init(&(q->lock), NULL);
    pthread_cond_init(&(q->wait), NULL);
    return q;
}

TaskData* pop(PCQueue* q){
    pthread_mutex_lock(&(q->lock));
    while(q->num_producers>0 || getSize(q->queue)==0){ //?????
        pthread_cond_wait(&(q->wait),&(q->lock));
    }
    TaskData* element = dequeue(q->queue);
    //dispatch interval updating:
    struct timeval pop_time;
    gettimeofday(&pop_time, NULL);
    element->dispatch_interval.tv_sec = pop_time.tv_sec - element->arrival_time.tv_sec;
    element->dispatch_interval.tv_usec = pop_time.tv_usec - element->arrival_time.tv_usec;
    pthread_mutex_unlock(&(q->lock));
    return element;
}

int getPCSize(PCQueue* q){
    return getSize(q->queue);
}

int my_ceil(float num) {
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

void pop_quarter_elemets_randomly(PCQueue* q){
    pthread_mutex_lock(&(q->lock));
    float float_num=0.30 * (float)(getMaxSize(q->queue));
    int quarter = my_ceil(float_num);
    for(int i=0; i<quarter; i++){
        int random_place = rand() % (getMaxSize(q->queue));
        TaskData* dropped = dequeueByRandom(&q->queue, random_place);
        Close(dropped->args);
    }
    pthread_mutex_unlock(&(q->lock));
}

void push(PCQueue* q, TaskData* element){
    pthread_mutex_lock(&(q->lock));
    q->num_producers++;
    enqueue(q->queue, element);
    q->num_producers--;
    pthread_cond_broadcast(&(q->wait));
    pthread_mutex_unlock(&(q->lock));
}

int isPCFull(PCQueue* q, int handling){
    return (getSize(q->queue)+handling) == getMaxSize(q->queue); //q->curr_size==q->max_size;
}

void destroyPCQueue(PCQueue* q){
    destroyQueue(q->queue);
    pthread_mutex_destroy(&(q->lock));
    pthread_cond_destroy(&(q->wait));
    free(q);
}

#endif //HW3_ASEEL_PCQUEUE_H
