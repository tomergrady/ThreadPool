//
// Created by tomer Grady on 5/30/18.
// 205660863
//

#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <stdbool.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "osqueue.h"
#include "stdlib.h"
#define ERROR "Error in system call\n"
#define ERROR_SIZE strlen(ERROR)
#define STDERR 2
#define FAIL (-1)
#define SUCCESS 0
#define FALSE 0
#define TRUE 1

typedef struct task
{
    void (*func)(void* arg);
    void* arg;
}Task;

typedef struct thread_pool
{
    int threadsNum;
    int shouldWaitForTasks;
    OSQueue* tasksQueue;
    pthread_t* threads;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

static void* startNewThread(void *arg);

void execute_thread(ThreadPool* tp);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);



#endif