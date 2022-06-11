//
// Created by student on 6/12/21.
//

#ifndef HW3_ASEEL_THREADPOOL_H
#define HW3_ASEEL_THREADPOOL_H

#include "segel.h"
#include "PCQueue.h"

ThreadStatistics* createStatistic(int i_thread){
    ThreadStatistics* stat = (ThreadStatistics*)malloc(sizeof(ThreadStatistics));
    stat->thread_id = i_thread;
    stat->thread_cnt = 0;
    stat->static_requests_num =0;
    stat->dynamic_requests_num =0;
    return stat;
}

typedef struct thread_pool{
    int threads_num;
    pthread_t* worker_threads;
    ThreadStatistics** statistics;
    PCQueue* waiting_tasks_queue;
    int curr_handling_tasks;
    char* schedalg;
    pthread_mutex_t count_lock;
    pthread_mutex_t full_lock;
    pthread_cond_t full_cond;

}ThreadPool;

typedef struct thread_args{
    ThreadPool* pool;
    int i_thread;
}ThreadArgs;

void* thread_function(void* arg){
    while(1){
        ThreadArgs* args = arg;
        ThreadPool* pool = args->pool;
        int i_thread = args->i_thread;
        TaskData* task = pop(pool->waiting_tasks_queue); // the dispatch time calculated here when we dequeue
        pthread_mutex_lock(&(pool->count_lock));
        pool->curr_handling_tasks++;
        pthread_mutex_unlock(&(pool->count_lock));
        //run task
        task->doFunction(task->args, task->arrival_time, task->dispatch_interval, pool->statistics[i_thread]); // TODO?????
        pthread_mutex_lock(&(pool->count_lock));
        pool->curr_handling_tasks--;
        pthread_mutex_unlock(&(pool->count_lock));
        Close(task->args); //???? TODO check if closing should be here ?
        // finish of task so there's empty place in queue
        pthread_cond_broadcast(&(pool->full_cond)); //???
    }
    return NULL;
}

ThreadPool* createThreadPool(int threads_num, int queue_size, char* schedalg){
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool)); //????
    pool->threads_num = threads_num;
    pool->waiting_tasks_queue = createPCQueue(queue_size, schedalg);
    pool->curr_handling_tasks = 0;
    pool->schedalg = (char*)malloc(sizeof(char)*strlen(schedalg));
    strcpy(pool->schedalg,schedalg);
    pthread_mutex_init(&(pool->count_lock), NULL);
    pthread_mutex_init(&(pool->full_lock), NULL);
    pthread_cond_init(&(pool->full_cond),NULL);
    //creating the statistics array, each cell is pointer for a thread statistics
    pool->statistics = (ThreadStatistics**)malloc(sizeof(ThreadStatistics*)*threads_num);
    for(int i=0; i<pool->threads_num; i++){
        pool->statistics[i] = createStatistic(i);
    }
    //creating the threads array
    pool->worker_threads = (pthread_t*)malloc(sizeof(pthread_t)*threads_num);
    for(int i=0; i<pool->threads_num; i++){
        //create args
        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
        args->pool = pool;
        args->i_thread = i;
        // TODO: don't forget to free(args)
        int res = pthread_create(&(pool->worker_threads[i]), NULL, thread_function, args);
        if(res != 0){
            return NULL;
        }
    }
    return pool;
}

void destroyThreadPool(ThreadPool* pool){
    for(int i=0; i<pool->threads_num; i++){
        pthread_join(pool->worker_threads[i],NULL);
        free(pool->statistics[i]);
    }
    free(pool->worker_threads);
    free(pool->statistics);
    destroyPCQueue(pool->waiting_tasks_queue);
    free(pool->schedalg);
//    free(pool->)
    pthread_mutex_destroy(&(pool->count_lock));
    pthread_mutex_destroy(&(pool->full_lock));
    pthread_cond_destroy(&(pool->full_cond));
    free(pool);
}

void addTaskToPoolQueue(ThreadPool* pool, void (*task_function)(int, struct timeval, struct timeval, void*), int args,
        struct timeval arrival_time){
    TaskData* task = createTaskData(task_function, args, arrival_time);
    //TODO: if queue full we should ......
    pthread_mutex_lock(&pool->full_lock);
    if(isPCFull(pool->waiting_tasks_queue, pool->curr_handling_tasks)){
        if(strcmp(pool->schedalg, "block") == 0){
            while(isPCFull(pool->waiting_tasks_queue, pool->curr_handling_tasks)){
                //wait
                pthread_cond_wait(&(pool->full_cond),&(pool->full_lock));
            }
            push(pool->waiting_tasks_queue ,task); // it has lock , it's ok??
        }
        if(strcmp(pool->schedalg, "dt") == 0){
            Close(args); // task->args
        }
        if(strcmp(pool->schedalg, "dh") == 0){
            if(getPCSize(pool->waiting_tasks_queue)==0){
                Close(args); // task->args
                pthread_mutex_unlock(&pool->full_lock);
                return;
            }
            pop(pool->waiting_tasks_queue);
            push(pool->waiting_tasks_queue, task);
        }
        if(strcmp(pool->schedalg, "random") == 0){
            if(getPCSize(pool->waiting_tasks_queue)==0){
                Close(args); // task->args
                pthread_mutex_unlock(&pool->full_lock);
                return;
            }
            pop_quarter_elemets_randomly(pool->waiting_tasks_queue);
            push(pool->waiting_tasks_queue, task);
        }
    }else{
        push(pool->waiting_tasks_queue ,task);
    }
    pthread_mutex_unlock(&pool->full_lock);
}

#endif //HW3_ASEEL_THREADPOOL_H