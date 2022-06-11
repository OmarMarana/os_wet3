//
// Created by student on 6/12/21.
//

#ifndef HW3_ASEEL_QUEUE_H
#define HW3_ASEEL_QUEUE_H

#include "segel.h"

typedef struct task_data{
    void (*doFunction)(int, struct timeval, struct timeval, void*);
    int args;
    struct timeval arrival_time;
    struct timeval dispatch_interval;
}TaskData;

typedef struct my_queue{
    int max_size;
    int curr_size;
    TaskData** array;
    int head, tail;
}Queue;

TaskData* createTaskData(void (*tfunction)(int, struct timeval, struct timeval, void*),
                            int args, struct timeval arrival_time){
    TaskData* task = (TaskData*)malloc(sizeof(TaskData));
    task->doFunction = tfunction;
    task->args = args;
    task->arrival_time = arrival_time;
    return task;
}

void destroyTaskData(TaskData* t){
    free(t);
}

Queue* createQueue(int size){
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->max_size = size;
    queue->curr_size = 0;
    queue->head = 0;
    queue->tail = size-1;
    queue->array = (TaskData**)malloc(sizeof(TaskData*)*size);
    return queue;
}

int isEmpty(Queue* queue){
    return queue->curr_size == 0;
}

int isFull(Queue* queue){
    return queue->curr_size==queue->max_size;
}

void enqueue(Queue* queue, TaskData* elem){
    if(isFull(queue)){
        return;
    }
    queue->tail = (queue->tail+1) % (queue->max_size);
    queue->array[queue->tail] = elem;
    queue->curr_size++;
}

int getSize(Queue* q){
    return q->curr_size;
}

int getMaxSize(Queue* q){
    return q->max_size;
}

TaskData* dequeue(Queue* queue){
    if(isEmpty(queue)){
        return NULL;
    }
    TaskData* elem = queue->array[queue->head];
    queue->head = (queue->head+1) % (queue->max_size);
    queue->curr_size--;
    return elem;
}

Queue* copyExceptOfRandIndex(Queue* old, int rand_index, TaskData** t){
    Queue* new_queue = createQueue(old->max_size);
    for(int i=0; i<old->curr_size; i++){
        int index = (old->head+i)%old->max_size;
        if(index != rand_index) {
            enqueue(new_queue, old->array[index]);
        }else{
            *t = old->array[index];
        }
    }
    return new_queue;
}

void destroyQueue(Queue* q){
    for(int i=0; i<q->curr_size; i++){
        destroyTaskData(q->array[(q->head+i)%q->max_size]);
    }
    free(q->array);
    free(q);
}

void printQueue(Queue* q){
    for(int i=0; i<q->curr_size; i++){
        printf("{index= %d, element= %d}, ",(q->head+i)%(q->max_size), (q->array)[(q->head+i)%(q->max_size)]->args);
    }
    printf("\n");

}

TaskData* dequeueByRandom(Queue** queue, int rand_place){
    if(isEmpty(*queue)){ // not supposed to be empty ????
        return NULL;
    }
    TaskData* elem ;//= (*queue)->array[((*queue)->head+rand_place)%(*queue)->max_size]; // (old->head+i)%old->max_size
    Queue* new_queue = copyExceptOfRandIndex(*queue, rand_place, &elem);
    *queue = new_queue;
//    destroyQueue(*tmp); // TODO: check if free(tmp) in destroy don't make problems
    return elem;
}

TaskData* getHead(Queue* queue){
    if(isEmpty(queue)){
        return NULL;
    }
    return queue->array[queue->head];
}

TaskData* getTail(Queue* queue){
    if(isEmpty(queue)){
        return NULL;
    }
    return queue->array[queue->tail];
}


#endif //HW3_ASEEL_QUEUE_H
