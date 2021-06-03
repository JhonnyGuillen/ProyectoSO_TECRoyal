#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define STACKSIZE 1024*8

typedef long thread;

typedef struct ThreadNodes
{
    thread threadID;
    ucontext_t threadContext;
    struct ThreadNodes *nextThread;
    struct ListThreadsWait *threadsWait;
    long startQuantum;
    int ThreadsCompleted;
    int ThreadsBlocked;
    int ThreadWaiting;
    int roundRobin;
    int lotery;
    int initialTicket;
    int finalTicket;
    int warningLevel;
    int ultimateWarningLevel;
    int limitTime;
    int detach;
}*ThreadNode;

typedef struct ListThreadsWait
{
    ThreadNode waitingThreadNode;
    struct ListThreadsWait *nextThreadWait;
}*ThreadsWait;

typedef struct ThreadNodeQueues
{
    ThreadNode head;
    ThreadNode headParent;
    ThreadNode currentThread;
    ThreadNode currentThreadCopy;
    long countRoundRobin;
    long countLotery;
    long count;
    long quantum;
    long quantums;
}*ThreadNodeQueue;

typedef struct ThreadsDeadNodes
{
    thread threadID;
    void **returnValue;
    int ThreadsWaiting;
    struct ThreadsDeadNodes *nextThreadDeadNode;
}*ThreadDeadNode;


typedef struct ThreadDeadNodeQueue
{
    ThreadDeadNode head;
    long count;
}*ThreadsDeadNodesQueue;

typedef struct MutexNode
{
   long lockNumber;
} ThreadMutex;

typedef struct MutexQueueNodes
{
    ThreadMutex *mutex;
    struct MutexQueueNodes *nextMutex;
    ThreadsWait threadNodeList;
} *MutexQueue;


//Threads

ThreadNode getHeadThreadNode(ThreadNodeQueue pQueue); //Obtiene el thread principal de la queue, el thread que se encuentra en la cabeza de la queue.
ThreadNode getNextThreadNode(ThreadNodeQueue pQueue); //Obtiene el thread que le sigue a la cabeza.
ThreadNode createNewThreadNode(); //Crea un nuevo nodo para un thread.
ThreadNode searchThread(thread pThreadID,ThreadNodeQueue pQueue); //Retorna un thread buscando por su ID en la queue.
ThreadNode searchThreadTicket(int pTicket, ThreadNodeQueue pQueue); //Retorna un thread buscando por su ID de tickete.
ThreadNodeQueue createThreadNodeQueue(); //Instancia una nueva queue donde se guardaran los nodos que contienen los threads.
ThreadsWait createWaitingThreadsList(); //Inicializa una nueva lista donde se guardaran los threds en espera de que otro finalice.
ThreadDeadNode searchDeadThread(ThreadsDeadNodesQueue pQueue, thread pThreadID); //Obtiene un thread dado su threadID de la lista de threads muertos.
ThreadDeadNode createNewDeadThreadNode(); //Instancia un nuevo nodo donde se guardara un thread que ya haya finalizado.
ThreadsDeadNodesQueue createDeadTheadsNodesQueue(); //Instancia una nueva queue donde se guardaran los threads que ya han terminado.
int moveForward(ThreadNodeQueue pQueue); //Avanza en la estructura, es decir, cambia la cabeza al siguiente y pone en el padre de la cabeza el thread anterior a la cabeza actual en la queue.
int getThreadNodeCount(ThreadNodeQueue pQueue); //Retorna la cantidad de threads que hay en la queue.
int getThreadNodeRoundRobinCount(ThreadNodeQueue pQueue); //Retorna la cantidad de threads adaministrados por el scheduler RoundRobin que hay en la queue.
int getThreadNodeSortCount(ThreadNodeQueue pQueue); //Retorna la cantidad de threads administrador por el scheduler Sort que hay en la queue.
int deleteHeadThread(ThreadNodeQueue pQueue); //Elimina el thread cabeza en la queue.
int deleteSortThread(thread pThreadID, ThreadNodeQueue pQueue); //Elimina el thread dado de la queue.
int insertThread(ThreadNodeQueue pQueue, ThreadNode pThread); //Agrega un nuevo thread al final de la queue (Que a su vez es una lista circular).
int insertWaitingThread(ThreadNode pTargetThread,ThreadNode pThreadAdded); //Agrega un nuevo thread a la lista de threads en espera (threads que hicieron un join para esperar la finalizacion de otro thread).
int insertDeadThred(ThreadsDeadNodesQueue pQueue, ThreadDeadNode pThread);// Agrega un nuevo thread al inicio de la queue y setea la caebeza al nuevo thread agregado.
int searchEndTicket(ThreadNodeQueue pQueue); //Busca el ultimo ticket que fue asignado a un proceso y lo devuelve sumandole uno.
void deleteDeadThread(ThreadsDeadNodesQueue pQueue, thread pThreadID); //Borra un deadThread de la lista.
void freeThread(ThreadNode pThread); //Libera el espacio asociado a un thread en la queue.
void giveTickets(ThreadNodeQueue pQueue, ThreadNode pThread, int pTickets); //Da mas ticketes al thread dado con la cantidad de tiquetes dados.
void deleteTickets(ThreadNodeQueue pQueue, ThreadNode pThread, int pTickets); //Borra ticketes al thread dado la cantidad enviada.
void setNewHead(ThreadNodeQueue pQueue, ThreadNode pThread); //Setea la cabeza al nuevo thread dado.
void restructureTickets(ThreadNodeQueue pQueue, ThreadNode pErasedThread); //Reestructura por completo los ticketes de los threads administrados por el scheduler sort.
void moveHeadToNextRoundRobin(ThreadNodeQueue pQueue, ThreadNode pThread); //Mueve la cabeza al siguiente thread administrado por un scheduler RoundRobin.

//Mutex

MutexQueue searchMutexQueue(ThreadMutex *pMutex);
MutexQueue createNewMutexQueue();
ThreadMutex *createNewMutexNode();
void insertMutexQueue(MutexQueue pMutex);
int isMutexInQueue(ThreadMutex *pMutex);

#endif
