#ifndef TEAM_PLANIFICADOR_H_
#define TEAM_PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include "libTeam.h"

void crearHiloSobreAlgoritmoDePlanificacion();
void planificadorSeleccionEntrenador();
void encolarEntrenador(t_queue*, t_entrenador*, pthread_mutex_t);
t_entrenador* desencolarEntrenadorCercano(t_queue*, t_entrenador*, pthread_mutex_t);
t_entrenador* desencolarEntrenadorSobreCola(t_entrenador*);
bool seEncuentraOcupado(t_entrenador*);
void establecerEntrenadorAlPokemon(t_list*, t_entrenador**, t_pokemon*);
t_entrenador* seleccionarQuienEstaMasCerca(t_entrenador**, t_entrenador*, t_pokemon*);
t_entrenador* despertarAlEntrenadorMasCercano(t_pokemon*);
t_entrenador* elDeMenorQuantum();
bool esMayorQue(double, double);
bool seEncuentraEnLaMismaPosicion(t_entrenador*);
bool estaParaIntercambiar(t_entrenador*);
void planificarPorFifo();
void planificarPorRR();
void planificarPorSJFSinDesalojo();
void planificarPorSRT();
void planificarEntrenador(void*);
void algoritmoDeteccionDeadlock();
bool necesitaUnPokemonDelOtro(t_entrenador*, t_entrenador*);
void iniciarIntercambioEntreEntrenadores(t_entrenador*);
void asignarYMoverEntrenadorAIntercambiar(t_entrenador*, t_entrenador*);
void intercambiarSinRestriccion(t_entrenador*);
void intercambiarConRestriccion(t_entrenador*);
void transicionarEntrenadorAlFinalizarIntercambio(t_entrenador*, char*);
void ejecutarIntercambioDePokemones(t_entrenador*);
int cantidadEntrenadoresDisponibles();
t_pokemon* asignarHaciaElMasCercano(t_entrenador*);
void elegirPokemonMasCercano(t_pokemon**, t_entrenador*, t_pokemon*);

#endif /* TEAM_PLANIFICADOR_H_ */
