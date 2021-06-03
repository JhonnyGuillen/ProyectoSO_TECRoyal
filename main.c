#include <stdio.h>
#include "mypthreads.h"
#include "unit.h"
#include "genetic_algorithm.h"

 void *imprimir(void * numero1);
 void *imprimir2(void * numero1);
 void *imprimir3(void * numero1);

	thread *threadsArray;
	ThreadMutex generalMutex;
	ThreadMutex useSocket;


int main(int argc, char **argv)
{
	
	


        char* l = "Lotery";
	char* rr = "RoundRobin";
	threadsArray = (thread*)malloc(sizeof(thread) * 3);
	
	my_thread_init(10);
	my_mutex_init(&generalMutex);
	my_mutex_init(&useSocket);
	
	my_thread_create(&threadsArray[0],imprimir,(void*)10,(int)150,rr);
	
	
	my_thread_create(&threadsArray[1],imprimir2,(void*)10,(int)150,rr);
	
	my_thread_create(&threadsArray[2],imprimir3,(void*)10,(int)150,rr);
	//printf("\n aca join \n");
	
	
	
	
	
	my_thread_join(threadsArray[0],NULL);
	
	my_thread_join(threadsArray[1],NULL);
	
	my_thread_join(threadsArray[2],NULL);
	
	free(threadsArray);
	
	
	return 0;
}

 void *imprimir(void * numero1){
	
	
	my_mutex_lock(&generalMutex);
	int maxP = 5;
		unit unidad = (unit)malloc(sizeof (struct Unit));
		unidad = createUnit(10, 2);
		struct UnitList *temp = (struct UnitList*) malloc(sizeof (struct UnitList));
		temp = createPoblation(temp, maxP, unidad);
		printf("%d\n",temp);
	my_mutex_unlock(&generalMutex);
	
	
	return 0;
	
}

 void *imprimir2(void * numero1){
	
	my_mutex_lock(&generalMutex);
	int maxP = 3;
	unit unidad = (unit)malloc(sizeof (struct Unit));
	unidad = createUnit(20, 10);
	struct UnitList *temp = (struct UnitList*) malloc(sizeof (struct UnitList));
	temp = createPoblation(temp, maxP, unidad);

	printf("%d\n",temp);
	my_mutex_unlock(&generalMutex);
	return 0;
	
}

void *imprimir3(void * numero1){
	
	my_mutex_lock(&generalMutex);
	int maxP = 2;
		unit unidad = (unit)malloc(sizeof (struct Unit));
		unidad = createUnit(3, 1);
		struct UnitList *temp = (struct UnitList*) malloc(sizeof (struct UnitList));
		temp = createPoblation(temp, maxP, unidad);
		printf("%d\n",temp);
	my_mutex_unlock(&generalMutex);
	
	return 0;
	
}

