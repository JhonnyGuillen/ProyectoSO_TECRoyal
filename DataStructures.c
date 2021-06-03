#include "DataStructures.h"
//variables globales

//Threads

static long actThreadID = 0;



//Mutex

MutexQueue mutexQueue = NULL;



//Funciones del sistema de threads


static int getThreadID()
{
    
    
    return ++actThreadID;
}


ThreadNode getHeadThreadNode(ThreadNodeQueue pQueue)
{
    if(pQueue == NULL)
    {
        return NULL;
    }
    else
    {
    	return pQueue->head;
    }
}


ThreadNode getNextThreadNode(ThreadNodeQueue pQueue)
{
    if(pQueue != NULL && pQueue->head != NULL)
    {
        return pQueue->head->nextThread;
    }
    else
    {
    	return NULL;
    }
}


ThreadNode createNewThreadNode(){
	
	ThreadNode newThreadNode = (ThreadNode)malloc(sizeof(struct ThreadNodes));
	if(newThreadNode == NULL){
		free(newThreadNode);
		return NULL;
	}
	else{
		
		newThreadNode->threadContext.uc_stack.ss_sp = malloc(STACKSIZE);
		if(newThreadNode->threadContext.uc_stack.ss_sp == NULL){
			
			free(newThreadNode);
			return NULL;
			
		}
		else{
			
			newThreadNode->threadContext.uc_stack.ss_size = STACKSIZE;
			newThreadNode->threadContext.uc_stack.ss_flags = 0;
			newThreadNode->nextThread = NULL;
			newThreadNode->threadContext.uc_link = 0;
			newThreadNode->ThreadsCompleted = 0;
			newThreadNode->threadsWait = NULL;
			newThreadNode->ThreadsBlocked = 0;
			newThreadNode->ThreadWaiting = 0;
			newThreadNode->roundRobin = 0;
			newThreadNode->lotery = 0;
			newThreadNode->initialTicket = 0;
			newThreadNode->finalTicket = 0;
			newThreadNode->warningLevel = 0;
			newThreadNode->ultimateWarningLevel = 0;
			newThreadNode->limitTime = 0;
			newThreadNode->detach = 0;
			newThreadNode->startQuantum = 0;
			newThreadNode->threadID = getThreadID();
			return newThreadNode;
		}
	}
}


ThreadNode searchThread(thread pThreadID, ThreadNodeQueue pQueue){
	
	ThreadNode headThread = getHeadThreadNode(pQueue);
	
	if(headThread == NULL){
		return NULL;
	}
	
	else if(headThread->threadID == pThreadID){
		return headThread;
	}
	
	else{
		ThreadNode temporalThread = headThread->nextThread;
		
		while(headThread != temporalThread){
			if(temporalThread->threadID == pThreadID){
				return temporalThread;
			}
			
			else{
				temporalThread = temporalThread->nextThread;
			}
		}
		return NULL;
	}
}


ThreadNode searchThreadTicket(int pTicket, ThreadNodeQueue pQueue){
	
	ThreadNode headThread = getHeadThreadNode(pQueue);
	
	if(headThread == NULL){
		return NULL;
	}
	else if(headThread->initialTicket <= pTicket && headThread->finalTicket >= pTicket){
		return headThread;
	}
	else{


		ThreadNode temporalThread = headThread->nextThread;
		while(headThread != temporalThread){
			if(temporalThread->initialTicket <= pTicket && temporalThread->finalTicket >= pTicket){
				return temporalThread;
			}
			else{
				temporalThread = temporalThread->nextThread;
			}
		}
		return NULL;  
	}
}


ThreadNodeQueue createThreadNodeQueue(){
	ThreadNodeQueue newQueue = (ThreadNodeQueue)malloc(sizeof(struct ThreadNodeQueues));
	if(newQueue == NULL){
		return NULL;
	}
	else{
		newQueue->quantum = 0;
		newQueue->quantums = 0;
		newQueue->count = 0;
		newQueue->countRoundRobin = 0;
		newQueue->countLotery = 0;
		newQueue->head = NULL;
		newQueue->headParent = NULL;
		newQueue->currentThread = NULL;
		newQueue->currentThreadCopy = NULL;
		return newQueue;
	}
}


ThreadsWait createWaitingThreadsList()
{
	ThreadsWait newList = (ThreadsWait)malloc(sizeof(struct ListThreadsWait));
	if(newList != NULL)
	{
		newList->waitingThreadNode = NULL;
		newList->nextThreadWait = NULL;
	}
	return newList;
}

ThreadDeadNode searchDeadThread(ThreadsDeadNodesQueue pQueue, thread pThreadID)
{
	if(pQueue != NULL)
	{
		ThreadDeadNode deadThread = pQueue->head;
		while((deadThread != NULL) && (deadThread->threadID != pThreadID))
		{
			deadThread = deadThread->nextThreadDeadNode;
		}
		return deadThread;
	}
	else
	{
		return NULL;
	}
}


void deleteDeadThread(ThreadsDeadNodesQueue pQueue, thread pThreadID)
{
	ThreadDeadNode  deadThread = pQueue->head;
	ThreadDeadNode  previousDeadTread = NULL;
	while((deadThread != NULL) && (deadThread->threadID != pThreadID))
	{
		previousDeadTread = deadThread;
		deadThread = deadThread->nextThreadDeadNode;
	}
	if(deadThread != NULL)
	{
		if(previousDeadTread == NULL)
		{
			pQueue->head = deadThread->nextThreadDeadNode;
		}
		else
		{
			previousDeadTread->nextThreadDeadNode = deadThread->nextThreadDeadNode;
		}
		free(deadThread);
	}
}


ThreadDeadNode createNewDeadThreadNode()
{
	ThreadDeadNode newDeadThread = (ThreadDeadNode)malloc(sizeof(struct ThreadsDeadNodes));
	if(newDeadThread == NULL)
	{
		return NULL;
	}
	else
	{
		newDeadThread->returnValue = (void**)malloc(sizeof(void*));
		if(newDeadThread->returnValue == NULL)
		{
			free(newDeadThread);
			return NULL;
		}
		else
		{
			newDeadThread->threadID = -4;
			*(newDeadThread->returnValue) = NULL;
			newDeadThread->ThreadsWaiting = 0;
			newDeadThread->nextThreadDeadNode = NULL;
			return newDeadThread;
		}
	}
}

ThreadsDeadNodesQueue createDeadTheadsNodesQueue()
{
	ThreadsDeadNodesQueue newQueue = (ThreadsDeadNodesQueue)malloc(sizeof(struct ThreadDeadNodeQueue));
	if(newQueue == NULL)
	{
		return NULL;
	}
	newQueue->count = 0;
	newQueue->head = NULL;
	return newQueue;
}

int moveForward(ThreadNodeQueue pQueue)
{
	int result = -1;
	if(pQueue != NULL)
	{
		ThreadNode headThread = pQueue->head;
		if(headThread != NULL)
		{
			pQueue->head = headThread->nextThread;
			pQueue->headParent = headThread;
			result = 0;
		}
	}
	return result;
}

int getThreadNodeCount(ThreadNodeQueue pQueue)
{
	if(pQueue == NULL)
	{
		return 0;
	}
	else
	{
		return pQueue->count;
	}
}




int getThreadNodeRoundRobinCount(ThreadNodeQueue pQueue)
{
	if(pQueue == NULL)
	{
		return 0;
	}
	else
	{
		return pQueue->countRoundRobin;
	}
}

int getThreadNodeLoteryCount(ThreadNodeQueue pQueue)
{
	if(pQueue == NULL)
	{
		return 0;
	}
	else
	{
		return pQueue->countLotery;
	}
}

void freeThread(ThreadNode pThread)
{
	if(pThread != NULL)
	{
		ThreadsWait waitingThreadNode = pThread->threadsWait;
		free(pThread->threadContext.uc_stack.ss_sp);
		while(waitingThreadNode != NULL)
		{
			ThreadsWait nextWaitingThreadNode = waitingThreadNode->nextThreadWait;
			free(waitingThreadNode);
			waitingThreadNode = nextWaitingThreadNode;
		}
		free(pThread);
	}
}

int deleteHeadThread(ThreadNodeQueue pQueue)
{
	int result = 0;
	ThreadNode previousThread, headThread;
	if(pQueue == NULL)
	{
		result = -1;
	}
	else
	{
		headThread = pQueue->head;
		previousThread = pQueue->headParent;
		if(headThread != NULL)
		{
			if(pQueue->count == 1)
			{
				pQueue->head = NULL;
				pQueue->headParent = NULL;
			}
			else
			{
				pQueue->head = headThread->nextThread;
				previousThread->nextThread = pQueue->head;
			}
			headThread->roundRobin == 1 ? pQueue->countRoundRobin-- : pQueue->countLotery--;
			freeThread(headThread);
			pQueue->count--;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}


int deleteLoteryThread(thread pThreadID, ThreadNodeQueue pQueue)
{
	int result = 0;
	ThreadNode headThread;
	if(pQueue == NULL)
	{
		result = -1;
	}
	else
	{
		headThread = pQueue->head;
		if(headThread != NULL)
		{
			if(pQueue->count == 1)
			{
				pQueue->head = NULL;
				pQueue->headParent = NULL;
			}
			else if(headThread->threadID == pThreadID)
			{
				pQueue->headParent->nextThread= headThread->nextThread;
				pQueue->head = pQueue->headParent->nextThread;
				if((pQueue->countLotery - 1) != 0)
				{
					restructureTickets(pQueue, headThread);
				}
				free(headThread);
			}
			else
			{
				while(headThread->nextThread->threadID != pThreadID)
				{
					headThread = headThread->nextThread;
				}
				ThreadNode auxThread = headThread->nextThread;
				headThread->nextThread = auxThread->nextThread;
				if(headThread->nextThread== pQueue->head)
				{
					pQueue->headParent = headThread;
				}
				if((pQueue->countLotery - 1) != 0)
				{
					restructureTickets(pQueue, auxThread);
				}
				free(auxThread);
			}
			pQueue->count--;
			pQueue->countLotery--;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}



int insertThread(ThreadNodeQueue pQueue, ThreadNode pThread)
{
	int result = 0;
	if(pQueue == NULL || pThread == NULL)
	{
		result = -1;
	}
	else
	{
		if(pQueue->head == NULL)
		{
			
			printf("\n id a meter :%ld \n",pThread->threadID);
			pThread->nextThread = pThread;
			pQueue->headParent = pThread;
			pQueue->head = pThread;
		}
		else if(pQueue->countRoundRobin == 0)
		{
			pThread->nextThread= pQueue->head;
			pQueue->headParent->nextThread = pThread;
			pQueue->head = pThread;
		}
		else
		{
			pThread->nextThread = pQueue->head;
			pQueue->headParent->nextThread= pThread;
			pQueue->headParent = pThread;
		}
		pQueue->count++;
		pThread->roundRobin == 1 ? pQueue->countRoundRobin++ : pQueue->countLotery++;
	}
	return result;
}

int insertWaitingThread(ThreadNode pTargetThread, ThreadNode pThreadAdded)
{
	ThreadsWait newWaitingThread = createWaitingThreadsList();
	if(newWaitingThread != NULL)
	{
		pTargetThread->ThreadWaiting++;
		newWaitingThread->waitingThreadNode = pThreadAdded;
		newWaitingThread->nextThreadWait = pTargetThread->threadsWait;
		pTargetThread->threadsWait = newWaitingThread;
		pThreadAdded->ThreadsBlocked = 1;
		return 0;
	}
	else
	{
		return -1;
	}
}


int insertDeadThread(ThreadsDeadNodesQueue pQueue, ThreadDeadNode pThread)
{
	if(pQueue != NULL && pThread != NULL)
	{
		pThread->nextThreadDeadNode = pQueue->head;
		pQueue->head = pThread;
		return 0;
	}
	else
	{
		return -1;
	}
}

int searchEndTicket(ThreadNodeQueue pQueue)
{
	ThreadNode headThread = getHeadThreadNode(pQueue);
	if(headThread == NULL)
	{
		return 1;
	}
	else
	{
		int nextTicket = 0;
		ThreadNode nextThread = headThread->nextThread;
		if(headThread->lotery && headThread->finalTicket > nextTicket)
		{
			nextTicket = headThread->finalTicket;
		}
		while(nextThread != headThread)
		{
			if(nextThread->lotery)
			{
				if(nextThread->finalTicket > nextTicket)
				{
					nextTicket = nextThread->finalTicket;
				}
				nextThread = nextThread->nextThread;
			}
			else
			{
				nextThread = nextThread->nextThread;
			}
		}
		return ++nextTicket;
	}
}


void giveTickets(ThreadNodeQueue pQueue, ThreadNode pThread, int pTickets)
{
	ThreadNode headThread = getHeadThreadNode(pQueue);
	if(headThread != NULL)
	{
		ThreadNode nextThread = headThread->nextThread;
		int previousTicket = pThread->finalTicket;
		pThread->finalTicket = pThread->finalTicket + pTickets;
		if(headThread->lotery && headThread != pThread && headThread->initialTicket > previousTicket)
		{
			headThread->initialTicket = headThread->initialTicket + pTickets;
			headThread->finalTicket = headThread->finalTicket + pTickets;
		}
		while(nextThread != headThread)
		{
			if(nextThread->lotery && nextThread != pThread)
			{
				if(nextThread->initialTicket > previousTicket)
				{
					nextThread->initialTicket = nextThread->initialTicket + pTickets;
					nextThread->finalTicket = nextThread->finalTicket + pTickets;
				}
				nextThread = nextThread->nextThread;
			}
			else
			{
				nextThread = nextThread->nextThread;
			}
		}
	}
}

void deleteTickets(ThreadNodeQueue pQueue, ThreadNode pThread, int pTickets)
{
	ThreadNode headThread = getHeadThreadNode(pQueue);
	if(headThread != NULL)
	{
		ThreadNode nextThread = headThread->nextThread;
		int previousTicket = pThread->finalTicket;
		if(pTickets > (pThread->finalTicket - pThread->initialTicket))
		{
			pTickets = pThread->finalTicket - pThread->initialTicket;
			pThread->finalTicket = pThread->initialTicket;
		}
		else
		{
			pThread->finalTicket = pThread->finalTicket - pTickets;
		}
		if(headThread->lotery && headThread != pThread && headThread->initialTicket > previousTicket)
		{
			headThread->initialTicket = headThread->initialTicket - pTickets;
			headThread->finalTicket = headThread->finalTicket - pTickets;
		}
		while(nextThread != headThread)
		{
			if(nextThread->lotery && nextThread != pThread)
			{
				if(nextThread->initialTicket > previousTicket)
				{
					nextThread->initialTicket = nextThread->initialTicket - pTickets;
					nextThread->finalTicket = nextThread->finalTicket - pTickets;
				}
				nextThread = nextThread->nextThread;
			}
			else
			{
				nextThread = nextThread->nextThread;
			}
		}
	}
}


void moveHeadToNextRoundRobin(ThreadNodeQueue pQueue, ThreadNode pThread)
{
	if(getThreadNodeRoundRobinCount(pQueue) > 1 && getHeadThreadNode(pQueue) == pThread)
	{
		ThreadNode auxThread = pThread->nextThread;
		while(auxThread->lotery)
		{
			auxThread = auxThread->nextThread;
		}
		setNewHead(pQueue, auxThread);
	}
}

void setNewHead(ThreadNodeQueue pQueue, ThreadNode pThread)
{
	ThreadNode headThread = getHeadThreadNode(pQueue);
	while(headThread != pThread)
	{
		moveForward(pQueue);
		headThread = getHeadThreadNode(pQueue);
	}
}

void restructureTickets(ThreadNodeQueue pQueue, ThreadNode pErasedThread)
{
	ThreadNode headThread = getHeadThreadNode(pQueue);
	ThreadNode nextThread = headThread->nextThread;
	int erasedTickets = (pErasedThread->finalTicket - pErasedThread->initialTicket) + 1;
	if(headThread != NULL)
	{
		if(headThread->lotery && headThread->initialTicket > pErasedThread->finalTicket)
		{
			headThread->initialTicket = headThread->initialTicket - erasedTickets;
			headThread->finalTicket = headThread->finalTicket - erasedTickets;
		}
		while(nextThread != headThread)
		{
			if(nextThread->lotery && nextThread->initialTicket > pErasedThread->finalTicket)
			{
				nextThread->initialTicket = nextThread->initialTicket - erasedTickets;
				nextThread->finalTicket = nextThread->finalTicket - erasedTickets;
				nextThread = nextThread->nextThread;
			}
			else
			{
				nextThread = nextThread->nextThread;
			}
		}
	}
}







//Funciones del sistema Mutex

int isMutexInQueue(ThreadMutex *pMutex){

	if(mutexQueue == NULL || pMutex == NULL){
		
		return 0;
	}
	else{
		
		MutexQueue auxQueue = mutexQueue;
		
		while((auxQueue != NULL) && (auxQueue->mutex->lockNumber != pMutex->lockNumber)){
			
			
			auxQueue = auxQueue->nextMutex;
			
		}
		if(auxQueue == NULL){
			return 0;
		}
		else{
			
			return 1;
		}
	}
}

MutexQueue createNewMutexQueue(){
	
	MutexQueue auxQueue = (MutexQueue)malloc(sizeof(struct MutexQueueNodes));
	if(auxQueue != NULL){
		
		auxQueue->mutex = NULL;
		auxQueue->nextMutex = NULL;
		auxQueue->threadNodeList = NULL;
		return auxQueue;
	}
	else{
		
		return NULL;
	}
}


ThreadMutex* createNewMutexNode(){
	
	static long currentMutexValue = 0;
	ThreadMutex* newMutexNode = (ThreadMutex*)malloc(sizeof(struct MutexNode));
	newMutexNode->lockNumber = ++currentMutexValue;
	return newMutexNode;
}

void insertMutexQueue(MutexQueue pMutex)
{
	if(pMutex != NULL)
	{
		pMutex->nextMutex = mutexQueue;
		mutexQueue = pMutex;
	}
}

MutexQueue searchMutexQueue(ThreadMutex *pMutex)
{
	if(mutexQueue== NULL || pMutex == NULL)
	{
		return NULL;
	}
	else
	{
		MutexQueue auxQueue = mutexQueue;
		while((auxQueue != NULL)  && (auxQueue->mutex->lockNumber != pMutex->lockNumber))
		{
			auxQueue = auxQueue->nextMutex;
		}
		return auxQueue;
	}
}













