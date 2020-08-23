#ifndef PLANIFICADORES_H
#define PLANIFICADORES_H

#include "libSAFA.h"
#include <stdlib.h>
#include <stdio.h>

void planificadorLargoPlazo();
void planificarSegunRR(CPU* cpu);
void planificarSegunVRR(CPU* cpu);
void planificarSegunIOBF(CPU* cpu);

DTB* obtenerDTBAEjecutarSegunRR();

DTB* obtenerDTBAEjecutarSegunVRR();
bool closureSortVRR(DTB* p1, DTB* p2);

DTB* obtenerDTBAEjecutarSegunIOBF();
bool closureSortIOBF(DTB* p1, DTB* p2);

void  identificarProceso(socket_connection * connection ,char** args);


#endif
