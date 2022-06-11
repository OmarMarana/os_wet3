#include "segel.h"
#include "request.h"
#include "ThreadPool.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port,int *threads_num, int *queue_size, char** schedalg, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = (char*)malloc(sizeof(char)*strlen(argv[4]));
    strcpy(*schedalg, argv[4]);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, queue_size;
    struct sockaddr_in clientaddr;

    char* schedalg; // block/dt/dh/random
    getargs(&port,&threads_num, &queue_size, &schedalg, argc, argv);

    // 
    // HW3: Create some threads...
    //
    ThreadPool* pool = createThreadPool(threads_num, queue_size, schedalg);
    free(schedalg);
    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
//	gettimeofday()
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	//
	struct timeval arrival_time;
	gettimeofday(&arrival_time, NULL);
    addTaskToPoolQueue(pool, requestHandle, connfd, arrival_time);
//	Close(connfd);
    }
    destroyThreadPool(pool);
    free(schedalg);
}


    


 
