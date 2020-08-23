#include <stdio.h>
#include <stdlib.h>
#include "libGameCard.h"

int main() {

	necesitaContinuar = true;
	listaPokemonMutex = list_create();
	pthread_mutex_init(&m_bitarray, NULL);
	pthread_mutex_init(&m_crearPokemon, NULL);
	sem_init(&salidaSubscripcion, 0, 0);
	// Con ctrl-c se llama a esta funcion para finalizar el Programa
	signal(SIGINT, finalizarPrograma);

	creacion_logger();
	read_and_log_config();

	// conexion para NP
	pthread_create(&hiloNP, NULL, (void*) &suscribir_proceso_GC, "NEW_POKEMON");
	pthread_detach(hiloNP);
	// conexion para GET POKEMON
	pthread_create(&hiloGP, NULL, (void*) &suscribir_proceso_GC, "GET_POKEMON");
	pthread_detach(hiloGP);
	// conexion para CATCH_POKEMON
	pthread_create(&hiloCP, NULL, (void*) &suscribir_proceso_GC, "CATCH_POKEMON");
	pthread_detach(hiloCP);

	crearPuntoDeMontajeGC();
	establecerLaMetadataDeGC();
	setearMetadataBloques();
	crearBitMapGC();
	crearDirectorioParaPokemones();
	crearArchivosBinTotalesFS();

	iniciar_servidor_GC();

	return 0;
}
