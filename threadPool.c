//
// Created by tomer Grady on 5/30/18
// 205660863
//


#include "threadPool.h"

void handleError() {
    write(STDERR, ERROR, ERROR_SIZE);
    exit(-1);
}

ThreadPool* tpCreate(int numOfThreads) {
    printf("tpCreate\n");
    int i;
    ThreadPool* thread_pool;
    pthread_t* threads;
    if ((thread_pool = (ThreadPool *) calloc(1, sizeof(ThreadPool))) == NULL) {
        handleError();
    }
    if ((threads = (pthread_t *) calloc((size_t) numOfThreads, sizeof(pthread_t))) == NULL) {
        handleError();
    }
    thread_pool->tasksQueue = osCreateQueue();
    thread_pool->threadsNum = numOfThreads;
    thread_pool->threads = threads;
    thread_pool->shouldWaitForTasks = true ;
    if (pthread_mutex_init(&thread_pool->mutex, NULL)) {
        handleError();
    }
    if(pthread_cond_init(&thread_pool->cond , NULL)) {
        handleError();
    }
    // create threads for the pool
    for (i = 0; i< numOfThreads ; i++){
        pthread_t tid;
        int ret = pthread_create(&tid, NULL, startNewThread, thread_pool);
        if (ret != 0) {
            handleError();
        }
        thread_pool->threads[i] = tid;
    }
    return thread_pool;

}
/**
 *
 * @param threadPool
 * @param shouldWaitForTasks
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    printf("tpDestroy\n");

    int i;
    int exitLoop = TRUE;
    //wait until all the Tasks that running to finish.
    if(shouldWaitForTasks == 0) {
        threadPool->shouldWaitForTasks = false;
        printf("tpDestroy NOT WAIT\n");
    }
        //wait until all the tasks are over
    else {
        while (exitLoop) {
            if (osIsQueueEmpty(threadPool->tasksQueue)) {
                printf("tpDestroy FINISH WAIT\n");
                threadPool->shouldWaitForTasks = false;
                exitLoop = FALSE;
            }
        }
    }
    if(pthread_cond_broadcast(&threadPool->cond)) {
        handleError();
    }
    while(!osIsQueueEmpty(threadPool->tasksQueue)) {
        free(osDequeue(threadPool->tasksQueue));
    }

    for (i = 0; i < threadPool->threadsNum; i++) {
        pthread_join(threadPool->threads[i], NULL);
    }
    pthread_cond_destroy(&threadPool->cond);
    pthread_mutex_destroy(&threadPool->mutex);
    osDestroyQueue(threadPool->tasksQueue);
    free(threadPool->threads);
    free(threadPool);

}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if(threadPool->shouldWaitForTasks) {
        printf("tpInsertTask\n");
        Task *task = (Task *) calloc(1, sizeof(Task));
        if (task == NULL) {
            handleError();
        }
        task->arg = param;
        task->func = computeFunc;
        if(osIsQueueEmpty(threadPool->tasksQueue)) {
            if (pthread_cond_broadcast(&threadPool->cond)) {
                handleError();
            }
        }
        osEnqueue(threadPool->tasksQueue, task);
        return SUCCESS;
    } else {
        return FAIL;
    }

}
/**
 * function the wraps the function that create a new thread
 * @param arg thread pool
 * @return null
 */
static void* startNewThread(void *arg)
{
    ThreadPool* tp = (ThreadPool*) arg;
    execute_thread(tp);
    return NULL;
}
/**
 * exectue a thread when the thread pool is ready
 * @param tp
 */
void execute_thread(ThreadPool* tp) {
    printf("execute_thread\n");
    Task *task = NULL;
    while (tp->shouldWaitForTasks == TRUE) {

        if (pthread_mutex_lock(&tp->mutex)) {
            handleError();
        }
        if (osIsQueueEmpty(tp->tasksQueue)) {
            if (pthread_cond_wait(&tp->cond, &tp->mutex)) {
                handleError();
            } //instead of busy waiting!
            if (!tp->shouldWaitForTasks) {
                if (pthread_mutex_unlock(&tp->mutex)) {
                    handleError();
                }
                return;
            }
        }
        task = (Task *) osDequeue(tp->tasksQueue);
        if (pthread_mutex_unlock(&tp->mutex)) {
            handleError();
        }
        (*task->func)(task->arg);
        free(task);
    }
}
