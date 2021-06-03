#include "DataStructures.h"
#include "mypthreads.h"
#include "Schedulers.h"

extern sigset_t sigProcMask;
extern ThreadNodeQueue threadsQueue;
extern struct itimerval timeQuantum;
extern long timeInterval;

int roundRobinControl = 0;
int loteryControl = 0;
int tiquete = 0;

void printQueue(ThreadNodeQueue pQueue);

//--------------------------------------RoundRobin-----------------------------------------

//Funcion que retorna el primer thread de la lista que rea del scheduler RoundRobin
static ThreadNode obtenerPrimeroRoundRobin(ThreadNodeQueue threadsQueue)
{
	//Moviendo al siguiente en la lista
	//printf("\n id %ld \n",getHeadThreadNode(threadsQueue)->threadID);
	if(getHeadThreadNode(threadsQueue)->ThreadsCompleted == 0)
	{
		moveForward(threadsQueue);
	}

	ThreadNode threadResultado = NULL;
	int i = 0;
	int totalThreads = getThreadNodeCount(threadsQueue);
	int threadsRoundRobin = getThreadNodeRoundRobinCount(threadsQueue);
	ThreadNode threadHeader = getHeadThreadNode(threadsQueue);

	while(i < totalThreads && threadsRoundRobin > 0)
	{
		//Revisa si el thread es administrado por RoundRobin y si tiene que ejecutar algo
		//Sino se elimana de paso
		if (threadHeader->ThreadsCompleted && threadHeader->roundRobin)
		{
			if (threadsQueue->currentThread == threadHeader)
			{
				threadsQueue->currentThread = NULL;
			}
			deleteHeadThread(threadsQueue);
			threadHeader = getHeadThreadNode(threadsQueue);
		//Revisa si el thread no se puede administrar por el Scheduler por una u otra razón
		}else if (threadHeader->ThreadsBlocked || threadHeader->lotery)
		{
			moveForward(threadsQueue);
			threadHeader = getHeadThreadNode(threadsQueue);
			i++;
		//No hay problema con el thread revisado
		}else{
			threadResultado = threadHeader;
			i = totalThreads;
		}
	}
	return threadResultado;
}

void roundRobinScheduler()
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	ThreadNode threadActual = obtenerPrimeroRoundRobin(threadsQueue);
	ThreadNode threadAnterior = threadsQueue->currentThread;
	//DeadLock
	if (threadAnterior == NULL && threadActual == NULL)
	{
		exit(1);
	//No hay RoundRobin para ser usado
	}else if (threadActual == NULL)
	{
		threadsQueue->currentThreadCopy = threadAnterior;
		threadsQueue->currentThread = NULL;
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		my_thread_yield();
	//En la selección anterior no se escogio ningún thread
	}else if(threadAnterior == NULL && threadActual != threadsQueue->currentThreadCopy)
	{
		threadsQueue->currentThread = threadActual;
		timeQuantum.it_value.tv_usec = timeInterval;
		if (threadsQueue->currentThreadCopy != NULL)
		{
			ThreadNode aux = threadsQueue->currentThreadCopy;
			threadsQueue->currentThreadCopy = NULL;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			swapcontext(&(aux->threadContext), &(threadActual->threadContext));
		}else
		{
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			setcontext(&(threadActual->threadContext));
		}
	//Todo bien
	}else
	{
		if (threadAnterior != NULL && threadAnterior != threadActual)
		{
			threadsQueue->currentThread = threadActual;
			threadsQueue->currentThreadCopy = NULL;
			timeQuantum.it_value.tv_usec = timeInterval;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			swapcontext(&(threadAnterior->threadContext), &(threadActual->threadContext));
		}else
		{
			threadsQueue->currentThread = threadActual;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		}
	}
}


//---------------------------------------------Lotery--------------------------------------

static void insertarValorThreadsGenerados(int* threadsGenerados, int idThread, int cantLotery);
static void borrarThreadsGenerados(int* threadsGenerados, int idThread, int cantLotery);
static int conteoThreadsGenerados(int* threadsGenerados, int cantLotery);
static int threadsBloqueados(ThreadNodeQueue queue);
static int existeEnThreads(int* threadsGenerados, int valorEncontradoThreads, int cantLotery);
static int generarTiquete(ThreadNodeQueue queue, int* generarThreads, int cantLotery);
static ThreadNode obtenerPrimeroLotery(ThreadNodeQueue queue);



void loteryScheduler()
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	ThreadNode threadActual = obtenerPrimeroLotery(threadsQueue);
	ThreadNode threadAnterior = threadsQueue->currentThread;

	//Si no hay threads para poner en funcionamiento desde hace un tiempo
	if(threadAnterior == NULL && threadActual == NULL)
	{
		//printf("Ocurrio un posible Deadlock en el schaduler lotery.\n");
		exit(1);
	//No hay uno disponible para actuar
	}else if (threadActual == NULL)
	{
		//printf("El siguiente thread a funcionar esta en NULL.\n");
		threadsQueue->currentThreadCopy = threadsQueue->currentThread;
		threadsQueue->currentThread = NULL;
		sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		my_thread_yield();
	//RoundRobin se quedo sin threads para funcionar por algún motivo o el anterior era del tipo lotery y termino
	}else if (threadAnterior == NULL && threadActual != threadsQueue->currentThreadCopy)
	{
		//printf("\naqui\n");
		threadsQueue->currentThread = threadActual;
		timeQuantum.it_value.tv_usec = timeInterval;
		if(threadsQueue->currentThreadCopy != NULL)
		{
			//printf("El thread anterior estaba en NULL, cambiandolo a otro posible\n");
			ThreadNode aux = threadsQueue->currentThreadCopy;
			threadsQueue->currentThreadCopy = NULL;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			swapcontext(&(aux->threadContext), &(threadActual->threadContext));
			
		}else
		{
			//printf("El thread anterior estaba en NULL, cambiandolo a otro posible\n");
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			setcontext(&(threadActual->threadContext));
		}
	}else{
		if (threadAnterior != NULL && threadAnterior != threadActual)
		{
			//printf("Cambiando thread %ld \n",threadsQueue->currentThread->threadID);
			threadsQueue->currentThread = threadActual;
			threadsQueue->currentThreadCopy = NULL;
			timeQuantum.it_value.tv_usec = timeInterval;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
			//printf("\naqui2\n");
			swapcontext(&(threadAnterior->threadContext), &(threadActual->threadContext));
			
		}else{
			threadsQueue->currentThread = threadActual;
			sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
		}
	}
}

//Funcion que busca el primer thread valido para el schedule de lotery
//De paso se borran los que ya estan completos
static ThreadNode obtenerPrimeroLotery(ThreadNodeQueue queue)
{
	ThreadNode threadResultado = NULL;
	int cantLotery = getThreadNodeLoteryCount(queue);
	int* generarThreads = (int*)calloc(cantLotery, sizeof(int));
	tiquete = generarTiquete(queue, generarThreads, cantLotery);
	//printf("El tiquete inicial para el thread fue: %i\n", tiquete);
	ThreadNode threadHeader = searchThreadTicket(tiquete, queue);

	if (threadsBloqueados(queue))
	{
		//printf("Todos los threads de lotery bloqueados\n");
		tiquete = 0;
		threadResultado = NULL;
	}else
	{
		int totalThreads = getThreadNodeLoteryCount(queue);
		while(conteoThreadsGenerados(generarThreads, cantLotery) <= totalThreads)
		{
			if (threadHeader->ThreadsCompleted && threadHeader->lotery)
			{
				if (queue->currentThread == threadHeader)
				{
					queue->currentThread = NULL;
				}
				borrarThreadsGenerados(generarThreads, (int)threadHeader->threadID, cantLotery);
				deleteLoteryThread(threadHeader->threadID, queue);
				if (getThreadNodeLoteryCount(queue) == 0 || threadsBloqueados(queue))
				{
					//printf("Imposible seguir administrando threads lotery en este momento\n");
					tiquete = 0;
					threadResultado = NULL;
					break;
				}else
				{
					tiquete = generarTiquete(queue, generarThreads, cantLotery);
					threadHeader = searchThreadTicket(tiquete, queue);
					insertarValorThreadsGenerados(generarThreads, (int)threadHeader->threadID, cantLotery);

				}
			}else if (threadHeader->ThreadsBlocked || threadHeader->roundRobin)
			{
				//printf("Thread bloqueado o de scheduler incompatible para lotery.\n");
				tiquete = generarTiquete(queue, generarThreads, cantLotery);
				threadHeader = searchThreadTicket(tiquete, queue);
				insertarValorThreadsGenerados(generarThreads, (int)threadHeader->threadID, cantLotery);
			}else
			{
				//printf("Se encontro thread optimo.\n");
				threadResultado = threadHeader;
				break;
			}
		}
	}
	free(generarThreads);
	return threadResultado;
}

//Función que genera los tiquetes para los threads
static int generarTiquete(ThreadNodeQueue queue, int* generarThreads, int cantLotery)
{
	int random = ((rand() % (searchEndTicket(queue) -1)) + 1);

	while(existeEnThreads(generarThreads, (int)searchThreadTicket(random, queue)->threadID, cantLotery)){
		random = ((rand() % (searchEndTicket(queue) -1)) + 1);
	}
	return random;
}



//Función de validación para tiquetes únicos
static int existeEnThreads(int* threadsGenerados, int valorEncontradoThreads, int cantLotery)
{
	int i = 0;
	int res = 0;
	while(i < cantLotery && res == 0)
	{
		if(threadsGenerados[i] == valorEncontradoThreads)
		{
			res = 1;
		}else
		{
			i++;
		}
	}
	return res;
}

static int threadsBloqueados(ThreadNodeQueue queue)
{
	int res = 1;
	ThreadNode threadHeader = getHeadThreadNode(queue);
	ThreadNode threadSiguiente = threadHeader->nextThread;
	if (threadHeader->lotery && !threadHeader->ThreadsBlocked)
	{
		res = 0;
	}else
	{
		while(threadHeader != threadSiguiente && res == 1)
		{
			if (threadSiguiente->lotery && !threadSiguiente->ThreadsBlocked)
			{
				res = 0;
			}else
			{
				threadSiguiente = threadSiguiente->nextThread;
			}
		}
	}
	return res;
}

//Cuenta cuantos threads han sido asignados hasta el momento
static int conteoThreadsGenerados(int* threadsGenerados, int cantLotery)
{
	int i = 0;
	int threads = 0;

	while(i < cantLotery)
	{
		if (threadsGenerados[i] != 0)
		{
			threads++;
		}
		i++;
	}
	return threads;
}

//Funcion que descarta el thread del arreglo de threads generados
static void borrarThreadsGenerados(int* threadsGenerados, int idThread, int cantLotery)
{
	int i = 0;
	int encontrado = 0;
	while(i < cantLotery && encontrado == 0)
	{
		if(threadsGenerados[i] == idThread)
		{
			threadsGenerados[i] = 0;
		}
		i++;
	}
}

//Funcio que inserta
static void insertarValorThreadsGenerados(int* threadsGenerados, int idThread, int cantLotery)
{
	int i = 0;
	while(i < cantLotery)
	{
		if (threadsGenerados[i] == 0 && !existeEnThreads(threadsGenerados, idThread, cantLotery))
		{
			threadsGenerados[i] = idThread;
			i = cantLotery;
		}
		i++;
	}
}

//--------------------------------------------------RealTime---------------------------------------------

static void revisarThreads();
static void comprobarThread(ThreadNode thread);
static void aumentarPrioridad(ThreadNode thread);
static void cambiarScheduler();

//Función que ejecuta el realTime Schaduler
void realTime()
{
	//printf("entre al scheduler\n");
	//printQueue(threadsQueue);
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	threadsQueue->quantums++;
	revisarThreads();
	cambiarScheduler();
	//printf("\n sali\n");
	//sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
}

//Revisa todos los threads uno por uno por si ubiera que aumentar la prioridad de alguno
static void revisarThreads()
{
	ThreadNode threadHeader = getHeadThreadNode(threadsQueue);
	ThreadNode threadSiguiente = threadHeader->nextThread;
	if (threadHeader != NULL)
	{
		comprobarThread(threadHeader);
		while(threadHeader != threadSiguiente)
		{
			comprobarThread(threadSiguiente);
			threadSiguiente = threadSiguiente->nextThread;
		}
	}
}

//Función que comprueba si es requerido cambiar la prioridad de un thread
//Si estaba en RoundRobin lo cambia a Lotery
static void comprobarThread(ThreadNode thread)
{
	if (thread->limitTime > 0 && thread->warningLevel > 0 && thread->ThreadsCompleted == 0)
	{
		//De roundRobin a lotery
		if (thread->roundRobin)
		{
			my_thread_chsched(thread);
		}
		aumentarPrioridad(thread);
	}
	//printf("estoy dentro\n");
}

//Da una cantidad de threads acorde al warning solicitado
static void aumentarPrioridad(ThreadNode thread)
{
	if(thread->warningLevel == 1)
	{
		giveTickets(threadsQueue, thread, 10);
	}
	else if(thread->warningLevel == 2)
	{
		giveTickets(threadsQueue, thread, 20);
	}
	else if(thread->warningLevel == 3)
	{
		giveTickets(threadsQueue, thread, 30);
	}
	thread->ultimateWarningLevel = thread->warningLevel;
	thread->warningLevel = 0;
}

//Funcion que cambia el scheduler 
static void cambiarScheduler()
{
	//printf("Estoy en el cambio\n");
	int cantLotery = getThreadNodeLoteryCount(threadsQueue);
	int cantRoundRobin = getThreadNodeRoundRobinCount(threadsQueue);
	if (loteryControl && cantLotery > 0 && cantRoundRobin == 0)
	{
		//printf("Primer caso de Lotery\n");
		roundRobinControl = 0;
		loteryScheduler();
	}else if (roundRobinControl && cantRoundRobin > 0 && cantLotery == 0)
	{
		//printf("Primer caso de roundRobin\n");
		loteryControl = 0;
		roundRobinScheduler();
	}else if (roundRobinControl && cantLotery > 0)
	{
		//printf("segundo caso de lotery\n");
		roundRobinControl = 0;
		loteryControl = 1;
		loteryScheduler();
	}else if (loteryControl && cantRoundRobin > 0)
	{
		//printf("segundo caso de roundRobin\n");
		roundRobinControl = 1;
		loteryControl = 0;
		roundRobinScheduler();
	}else
	{
		//printf("No hay threads en ninguno de los schedules\n");
		exit(0);
	}
}

void printQueue(ThreadNodeQueue pQueue)
{
    ThreadNode headThread = getHeadThreadNode(pQueue);
    ThreadNode nextThread = headThread->nextThread;
    if(headThread != NULL)
    {
        printf("Imprimiendo queue\n");
        printf("Head: %d\n", (int)headThread->threadID);
        printf("Parent: %d\n", (int)pQueue->headParent->threadID);
        printf("Thread: %d\n", (int)headThread->threadID);
        
        while(headThread != nextThread)
        {
            printf("nextThread: %d\n", (int)nextThread->threadID);
            nextThread = nextThread->nextThread;
        }
        printf("Termino la impresion de la queue\n");
        
        printf("nextThread: %d\n", (int)nextThread->threadID);
    }
}




//Con lo que probaba yo, descomentar si la necesita
/*
int main(int argc, char const *argv[])
{
	threadsQueue = createThreadNodeQueue();
	ThreadNode ThreadNodeMain = createNewThreadNode();
	insertThread(threadsQueue, ThreadNodeMain);
	realTime();
	freeThread(ThreadNodeMain);
	free(threadsQueue);
	return 0;
}
*/
