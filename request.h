#ifndef __REQUEST_H__

typedef struct statistics{
    int thread_id; // The id of the responding thread
    int thread_cnt; // The total number of http requests this thread has handled
    int static_requests_num; // The total number of static requests this thread has handled
    int dynamic_requests_num; // The total number of dynamic requests this thread has handled
}ThreadStatistics;

void requestHandle(int fd, struct timeval arrival_time, struct timeval dispatch_time, void* threadStat);

#endif
