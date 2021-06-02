//
// Created by jhonny on 30/5/21.
//

#ifndef TRYHILOS_UNIT_H
#define TRYHILOS_UNIT_H

#include <stdio.h>
#include <stdlib.h>

int MAX_POBLATION;

typedef struct Unit{
    int hp; //salud de la unidad
    int atk; //fuerza fisica de la unidad
    int level; //nivel que tiene la unidad

} *unit;

typedef struct UnitList{
    unit unit;
    int fitnessValue;
    struct UnitList *next;
};

//Funciones
unit createUnit(int hp, int atk);
struct UnitList createUnitList(int numberUnit);
struct UnitList *insertUnitList(struct UnitList *unitList, unit newUnit);
struct UnitList *createPoblation(struct UnitList *unitList, int numPoblation, unit typeUnit);

#endif //TRYHILOS_UNIT_H
