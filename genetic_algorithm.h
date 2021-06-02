//
// Created by jhonny on 29/5/21.
//

#ifndef TRYHILOS_GENETIC_ALGORITHM_H
#define TRYHILOS_GENETIC_ALGORITHM_H

#include <stdio.h>
#include <stdlib.h>

#include "unit.h"

#define TRUE 1
#define FALSE 0
#define MUTATION_RATE 0.05

//funciones
int evaluateOrganisms(struct UnitList *poblation);
struct UnitList *selectionUnits(struct UnitList *poblation);
unit *crossover(unit male, unit female);
unit *mutation(unit newUnit);
struct UnitList * mating(struct UnitList *poblacion);
void printPoblation(struct UnitList *poblacion);
struct UnitList *genetics(struct UnitList *currentPoblation);

#endif //TRYHILOS_GENETIC_ALGORITHM_H
