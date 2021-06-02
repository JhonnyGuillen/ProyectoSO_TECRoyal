//
// Created by jhonny on 30/5/21.
//

#include "unit.h"

unit createUnit(int hp, int atk){
    unit newUnit = (unit) malloc(sizeof(struct Unit));
    if(newUnit == NULL){
        free(newUnit);
        return NULL;
    }
    else{
        newUnit->hp = hp;
        newUnit->atk = atk;
        newUnit->level = (hp + atk)/2;

        return newUnit;
    }
}

struct UnitList *insertUnitList(struct UnitList *unitList, unit newUnit){
    struct UnitList *newUList = (struct UnitList*) malloc(sizeof (struct UnitList));

    newUList->unit = newUnit;
    newUList->fitnessValue = newUList->unit->level - ((999/10)/2); //calculo fitness, para ahorrar una funcion

    newUList->next = unitList;

    return newUList;
}

struct UnitList *createPoblation(struct UnitList *unitList, int numPoblation, unit typeUnit){
    struct UnitList *newPoblation = (struct UnitList*) malloc(sizeof (struct UnitList));
    for(int i=0; i<numPoblation; i++){
        newPoblation = insertUnitList(newPoblation, typeUnit);
    }
    MAX_POBLATION = numPoblation;
    return newPoblation;
}