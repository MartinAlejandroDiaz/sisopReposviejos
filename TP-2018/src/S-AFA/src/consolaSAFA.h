#ifndef CONSOLA_SAFA_H
#define CONSOLA_SAFA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <semaphore.h>
#include "libSAFA.h"
#include "planificadores.h"

int generadorDeIds;

//definicion de funciones
void consolaSAFA();
void ejecutar(char* rutaScript);
void status(int idGDT);
void finalizar(int idGDT);
void metricas(int idGDT);
void pausarPlanificacion();
void iteracionPausarCPUs(CPU* cpu);
void continuarPlanificacion();
void iteracionContinuarCPUs(CPU* cpu);


//  A medida que avanzamos puede que necesitemos cambiarles los
//  parametros a las funciones o no.

#endif
