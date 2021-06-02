//
// Created by jhonny on 29/5/21.
//

#include "genetic_algorithm.h"
#include "unit.h"

struct UnitList *selectionUnits(struct UnitList *poblation){
    struct UnitList *unitSelection = (struct UnitList*) malloc(sizeof (struct UnitList));
    int halfPoblation = MAX_POBLATION/2;
    int currentNewPoblation=0;
    int fitness = poblation->fitnessValue;
    while(poblation != NULL){
        if(poblation->unit->level > fitness){
            unitSelection = insertUnitList(unitSelection, poblation->unit);
            fitness = poblation->unit->level;
            currentNewPoblation++;
        }
        if(currentNewPoblation >= halfPoblation){
            break;
        }
        poblation= poblation->next;
    }
    return unitSelection;
}

unit *crossover(unit male, unit female){
    int hpPoint = rand()%(3-1)+2;
    int atkPoint = rand()%(3-1)+2;
    int hpInherited;
    int atkInherited;
    if(hpPoint==1){
        hpInherited=male->hp;
    }
    else{
        hpInherited=female->hp;
    }
    if(atkPoint==1){
        atkInherited=male->atk;
    }
    else{
        atkInherited=female->atk;
    }
    return createUnit(hpInherited, atkInherited);
}

unit *mutation(unit newUnit){
    double hpMutationRate = (double)rand()/((double)1+1);
    double atkMutationRate = (double)rand()/((double)1+1);
    int hpMutated;
    int atkMutated;
    if(hpMutationRate>MUTATION_RATE){
        hpMutated=newUnit->hp+1;
    }
    if(atkMutationRate>MUTATION_RATE){
        atkMutated=newUnit->atk+1;
    }
    return createUnit(hpMutated, atkMutated);
}

struct UnitList *mating(struct UnitList *poblacion){
    struct UnitList *newGeneration = (struct UnitList*) malloc(sizeof (struct UnitList));
    unit *newUnit = (unit)malloc(sizeof (struct Unit));

    while(poblacion != NULL){
        unit male = poblacion->unit;
        poblacion = poblacion->next;
        unit female = poblacion->unit;
        newUnit = mutation(crossover(male, female));
        newGeneration = insertUnitList(newGeneration, newUnit);
        newGeneration = insertUnitList(newGeneration, newUnit);
        poblacion=poblacion->next;
    }
    return newGeneration;
}

void printPoblation(struct UnitList *poblacion){
    struct UnitList *p = (struct UnitList*) malloc(sizeof (struct UnitList));
    printf("Poblacion:\n");
    while (p !=NULL){
        printf("Nivel: %d \tSalud: %d \tAtaque: %d \tValor Fitness: %d\n",p->unit->level, p->unit->hp, p->unit->atk, p->fitnessValue);
        p=p->next;
    }
}
