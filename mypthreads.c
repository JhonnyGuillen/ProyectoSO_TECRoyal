#include "Schedulers.h"
#include "DataStructures.h"
#include "mypthreads.h"


ThreadNodeQueue threadsQueue = NULL;
struct itimerval timeQuantum;
long timeInterval;
sigset_t sigProcMask;
ThreadsDeadNodesQueue deadThreadsQueue = NULL;
struct sigaction schedulerHandle;
ucontext_t exitContext;
time_t randomTimeSeed;



extern MutexQueue mutexQueue;
extern int roundRobinControl;
extern int loteryControl;

static void *wrapperFunction(void *(*pStartRoutine)(void *), void *pArgument);
static void clearBlockedThreads(ThreadNode pTargetThread);
static void setExitContext();
static void executeExitContext();
static void setSchedulerType(ThreadNode pThread, char *pSchedulerType);


//Threads functions
void my_thread_init(long pTimeInterval) 
{

	if (threadsQueue == NULL && deadThreadsQueue == NULL) 
	{
		sigemptyset(&sigProcMask);
		sigaddset(&sigProcMask, SIGPROF);
		deadThreadsQueue = createDeadTheadsNodesQueue();
		threadsQueue = createThreadNodeQueue();
		if (deadThreadsQueue == NULL || threadsQueue == NULL) 
		{
			return;
		}
		else
		{
			srand((unsigned) time(&randomTimeSeed));
			timeInterval = pTimeInterval * 1000;
			threadsQueue->quantum = pTimeInterval;
			ThreadNode ThreadNodeMain = createNewThreadNode();
			getcontext(&(ThreadNodeMain->threadContext));
			setExitContext();
			ThreadNodeMain->threadContext.uc_link = &exitContext;
			//La linea de abajo indica que el thread principal es administrado por el scheduler RoundRobin.
			// TCBMain->roundRobin = 1;
			// roundRobinControl = 1;
			// Descomentar las lineas comentadas de abajo y comentar la linea de arriba si se quiere que el thread principal sea administrado por el scheduler Lotery.
			ThreadNodeMain->lotery = 1;
			int nextTicket = searchEndTicket(threadsQueue);
			ThreadNodeMain->initialTicket = nextTicket;
			ThreadNodeMain->finalTicket = nextTicket;
			loteryControl = 1;
			//
			threadsQueue->currentThread = ThreadNodeMain;
			insertThread(threadsQueue, ThreadNodeMain);
			memset(&schedulerHandle, 0, sizeof (schedulerHandle));
			schedulerHandle.sa_handler = &realTime;
			sigaction(SIGPROF, &schedulerHandle, NULL);
			//printf("\nMyThread: Biblioteca MyThread Inicializada...\n");
			timeQuantum.it_value.tv_sec = 0;
			timeQuantum.it_value.tv_usec = timeInterval;
			timeQuantum.it_interval.tv_sec = 0;
			timeQuantum.it_interval.tv_usec = timeInterval;
			setitimer(ITIMER_PROF, &timeQuantum, NULL);
		}
	}
}

int my_thread_create(thread *pThread, void *(*pStartRoutine)(void *), void *pArgument, int pLimitTime, char *pSchedulerType) 
{
	if (threadsQueue != NULL) 
	{
		sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
		ThreadNode newThreadNode = createNewThreadNode();
		getcontext(&(newThreadNode->threadContext));
		if (newThreadNode == NULL)
		{
			freeThread(newThreadNode);
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			return -3;
		}
		else
		{
			if(pLimitTime > 0)
			{
				newThreadNode->limitTime = pLimitTime;
			}
			newThreadNode->threadContext.uc_link = &exitContext;
			newThreadNode->startQuantum = threadsQueue->quantums;
			setSchedulerType(newThreadNode, pSchedulerType);
			makecontext(&(newThreadNode->threadContext), (void (*)()) wrapperFunction, 2, pStartRoutine, pArgument);
			//setcontext(&(newThreadNode->threadContext));
			*pThread = newThreadNode->threadID;
			printf("MyThread: Nuevo thread creado: %ld\n", newThreadNode->threadID);
			
			insertThread(threadsQueue, newThreadNode);
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			return 0;
		}
	}
	else
	{
		return -2;
	}
}


int my_thread_join(thread pThread, void **pStatus) 
{
	
	//printf("\nestado del thread: %d",pThread->ThreadsBlocked);
	
	//printf("\n threadActual: %ld \n",pThread);
	
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	
	ThreadNode currentThread = threadsQueue->currentThread;
	ThreadNode targetThread = searchThread(pThread, threadsQueue);
	
	if (currentThread == targetThread || currentThread == NULL || (targetThread != NULL && targetThread->detach)) 
	{
		
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		return -1;
	}
	else
	{
		//printf("\naca2\n");
		
		if (targetThread == NULL || targetThread->ThreadsCompleted)
		{
			
			
			ThreadDeadNode deadThreadNode = searchDeadThread(deadThreadsQueue, pThread);
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			if (deadThreadNode != NULL) 
			{
				if (pStatus != NULL) 
				{
					*pStatus = *(deadThreadNode->returnValue);
				}
				deleteDeadThread(deadThreadsQueue, pThread);
				return 0;
			} 
			else 
			{
				return -1;
			}
		}
		else
		{
			
			insertWaitingThread(targetThread, currentThread);
			
			int isBlocked = currentThread->ThreadsBlocked;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			//printf("\naca\n");
			while (isBlocked) 
			{
				
				isBlocked = currentThread->ThreadsBlocked;
			}
			
			sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
			ThreadDeadNode deadThreadNode = searchDeadThread(deadThreadsQueue, pThread);
			if(deadThreadNode != NULL)
			{
				
				if (pStatus != NULL) 
				{   
					*pStatus = *(deadThreadNode->returnValue);
				}
				if(((deadThreadNode->ThreadsWaiting) - 1) == 0)
				{   
					deleteDeadThread(deadThreadsQueue, pThread);
				}
				else
				{
					deadThreadNode->ThreadsWaiting--;
				}
				sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
				return 0;
			}
			else
			{
				//printf("MyThread: Un thread anterior a este ha realizado el join primero, intente realizando el join para ambos threads antes que el thread al cual desea hacer el join haya finalizado\n");
				sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
				return 0;
			}
		}
	}
}



thread my_thread_self(void) 
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	ThreadNode currentThread = threadsQueue->currentThread;
	if (currentThread == NULL) 
	{
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		return -4;
	}
	else
	{
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		return currentThread->threadID;
	}
}

void my_thread_yield(void) 
{
	raise(SIGPROF);
}

int my_thread_end(thread pThread)
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	ThreadNode currentThread = threadsQueue->currentThread;
	if ((currentThread != NULL) && (currentThread->threadID != pThread))
	{
		ThreadNode targetThread = searchThread(pThread, threadsQueue);
		if (targetThread != NULL) 
		{
			clearBlockedThreads(targetThread);
			//printf("MyThread: Thread %ld end\n", targetThread->threadID);
			targetThread->ThreadsCompleted = 1;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			return 0;
		}
		else
		{
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			return -1;
		}
	}
	else
	{
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		return -1;
	}
}



void my_thread_exit(void *pReturnValue) 
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	ThreadNode currentThreadNode = threadsQueue->currentThread;
	ThreadDeadNode deadThreadNode = createNewDeadThreadNode();
	if (deadThreadNode != NULL && currentThreadNode != NULL)
	{
		*(deadThreadNode->returnValue) = pReturnValue;
		deadThreadNode->threadID = currentThreadNode->threadID;
		deadThreadNode->ThreadsWaiting = currentThreadNode->ThreadWaiting;
		insertDeadThread(deadThreadsQueue, deadThreadNode);
	}
	executeExitContext();
	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	raise(SIGPROF);
}

int my_thread_detach(thread pThread)
{
	ThreadNode targetThread = searchThread(pThread, threadsQueue);
	if(targetThread != NULL)
	{
		targetThread->detach = 1;
		return 0;
	}
	else
	{
		return -1;
	}
}

void my_thread_chsched(ThreadNode pThread)
{
	if(pThread->ThreadsCompleted == 0)
	{
		if(pThread->lotery)
		{
			sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
			pThread->lotery = 0;
			pThread->roundRobin = 1;
			restructureTickets(threadsQueue, pThread);
			pThread->initialTicket = 0;
			pThread->finalTicket = 0;
			if(threadsQueue->countRoundRobin == 0)
			{
				setNewHead(threadsQueue, pThread);
			}
			threadsQueue->countRoundRobin++;
			threadsQueue->countLotery--;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		}
		else
		{
			sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
			pThread->roundRobin = 0;
			setSchedulerType(pThread, "Lotery");
			moveHeadToNextRoundRobin(threadsQueue, pThread);
			threadsQueue->countRoundRobin--;
			threadsQueue->countLotery++;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		}
	}
	else
	{
		//printf("MyThread: No puede cambiar de scheduler un thread que ya ha sido completado.\n");
	}
}



void my_thread_sleep(long pSleepTime)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    long actualQuantums = threadsQueue->quantums;
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(pSleepTime >= threadsQueue->quantum)
    {
        while(((threadsQueue->quantums - actualQuantums) * threadsQueue->quantum) <= pSleepTime);
    }
}

long my_threadGetTimeExecution(thread pThread)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    ThreadNode targetThread = searchThread(pThread, threadsQueue);
    long actualQuantums = threadsQueue->quantums;
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    return (actualQuantums - targetThread->startQuantum) * threadsQueue->quantum;
}

static void setSchedulerType(ThreadNode pThread, char *pSchedulerType)
{
	if(!strcmp("Lotery", pSchedulerType))
	{
		pThread->lotery = 1;
        int nextTicket = searchEndTicket(threadsQueue);
        pThread->initialTicket = nextTicket;
        pThread->finalTicket = nextTicket;
	}
	else
	{
		pThread->roundRobin = 1;
	}
}

static void *wrapperFunction(void *(*pStartRoutine)(void *), void *pArgument) 
{
    void *returnValueFunction;
    ThreadNode currentThreadNode = threadsQueue->currentThread;
    //printf("aca");
    returnValueFunction = (*pStartRoutine)(pArgument);
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    if(!currentThreadNode->detach)
    {
    	ThreadDeadNode deadThreadNode = createNewDeadThreadNode();
	    if (deadThreadNode != NULL) 
	    {
	        *(deadThreadNode->returnValue) = returnValueFunction;
	        deadThreadNode->threadID = currentThreadNode->threadID;
	        deadThreadNode->ThreadsWaiting = currentThreadNode->ThreadWaiting;
	        insertDeadThread(deadThreadsQueue, deadThreadNode);
	    }
    }
    //
    //printDeadQueue(deadThreadsQueue);
    //
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    return returnValueFunction;
}


static void clearBlockedThreads(ThreadNode pTargetThread)
{
    ThreadsWait blockedThread = pTargetThread->threadsWait;
    while(blockedThread != NULL)
    {
        blockedThread->waitingThreadNode->ThreadsBlocked = 0;
        blockedThread = blockedThread->nextThreadWait;
    }
}

static void executeExitContext()
{
    ThreadNode currentThread = threadsQueue->currentThread;
    clearBlockedThreads(currentThread);
    //printf("MyThread: Thread %ld completado\n", currentThread->threadID);
    currentThread->ThreadsCompleted = 1;
    raise(SIGPROF);
}

static void setExitContext()
{
    static int exitContextCreated;
    if(!exitContextCreated)
    {
        getcontext(&exitContext);
        exitContext.uc_link = 0;
        exitContext.uc_stack.ss_sp = malloc(STACKSIZE);
        exitContext.uc_stack.ss_size = STACKSIZE;
        exitContext.uc_stack.ss_flags= 0;
        makecontext(&exitContext, (void (*) (void))&executeExitContext, 0);
        exitContextCreated = 1;
    }
}

//Mutex Functions


int my_mutex_init(ThreadMutex *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    if(pMutex != NULL)
    {
        if(!isMutexInQueue(pMutex))
        {
            MutexQueue newMutexQueue = createNewMutexQueue();
            newMutexQueue->mutex = createNewMutexNode();
            pMutex->lockNumber = newMutexQueue->mutex->lockNumber;
            insertMutexQueue(newMutexQueue);
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return 0;
        }
    }
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    return -1;
}

int my_mutex_lock(ThreadMutex *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    MutexQueue auxQueue = searchMutexQueue(pMutex);
    ThreadNode currentThreadNode = threadsQueue->currentThread;
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(auxQueue == NULL)
    {        
        return -1;
    }
    else
    {
    	if(currentThreadNode != NULL)
	    {
            while(auxQueue->threadNodeList);
            sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
            ThreadsWait waitingThreadNodeNode = createWaitingThreadsList();
            waitingThreadNodeNode->waitingThreadNode = currentThreadNode;
            waitingThreadNodeNode->nextThreadWait = auxQueue->threadNodeList;
            auxQueue->threadNodeList = waitingThreadNodeNode;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return 0;
	    }
	    else
	    {
	    	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	    	return -1;
	    }
    }
}

int my_mutex_unlock(ThreadMutex *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    MutexQueue auxQueue = searchMutexQueue(pMutex);
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(auxQueue == NULL)
    {   
        return -1;
    }
    else if(auxQueue->threadNodeList == NULL)
    {
        return -1;
    }
    else
    {
    	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
        ThreadNode currentThreadNode = threadsQueue->currentThread;
        ThreadsWait waitingThreadNodeNode = auxQueue->threadNodeList;
        if(waitingThreadNodeNode ->waitingThreadNode == currentThreadNode)
        {
            auxQueue->threadNodeList = NULL;
            free(waitingThreadNodeNode );
        }
        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	    return 0;
    }
}
