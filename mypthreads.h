#ifndef MYPTHREADS_H
#define	MYPTHREADS_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "DataStructures.h"


//FUNCIONES DE THREADS

void my_thread_init(long pTimeInterval);
int my_thread_create(thread *pThread, void *(*pStartRoutine)(void *), void *pArgument, int pLimitTime, char *pSchedulerType);
thread my_thread_self(void);
void my_thread_yield(void);
void my_thread_exit(void *pReturnValue);
void my_thread_chsched(ThreadNode pThread);
void my_thread_sleep(long pSleepTime);
int my_thread_end(thread pThread);
int my_thread_join(thread pThread, void **pStatus);
int my_thread_detach(thread pThread);
long my_threadGetTimeExecution(thread pThread);

//FUNCIONES DE MUTEX

int my_mutex_init(ThreadMutex *pMutex);
int my_mutex_lock(ThreadMutex *pMutex);
int my_mutex_unlock(ThreadMutex *pMutex);


#endif
