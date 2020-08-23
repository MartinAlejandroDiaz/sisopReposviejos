#ifndef GAMECARD_LIBGAMECARD_H_
#define GAMECARD_LIBGAMECARD_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/txt.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "../utilCommons/clientSide/clientUtils.h"
#include <string.h>
#include <semaphore.h>
#include "serverGameCard.h"

typedef struct {
	int tiempo_de_reintento_reconexion;
	int tiempo_de_reintento_operacion;
	int tiempo_retardo_operacion;
	char* punto_montaje_glass;
	char* ipBroker;
	char* puertoBroker;
	char* ipGameCard;
	char* puertoGameCard;
	t_config* configuracion;
}t_config_gameCard;

typedef struct{
	char* nombrePokemon;
	pthread_mutex_t mutexPokemon;
} t_exclusion_pokemon;

typedef struct {
	size_t tamanio_bloques;
	int cantidad_bloques;
	char* magic_number;
}t_data_GC;

typedef struct {
	uint32_t posicionX;
	uint32_t posicionY;
} t_posiciones;

t_log* logger;
t_config* config;
t_list* listaPokemonMutex;
t_config_gameCard* datosConfigGameCard;
t_data_GC* datosFileSystem;
t_bitarray* bitarray;

pthread_mutex_t m_bitarray;
pthread_mutex_t m_crearPokemon;
sem_t salidaSubscripcion;

pthread_t hiloNP;
pthread_t hiloGP;
pthread_t hiloCP;

// Funciones de Log y Config
void creacion_logger();
t_config_gameCard* read_and_log_config();

// Funciones de Creacion de Carpetas
void crearPuntoDeMontajeGC();
void crearBitMapGC();
void crearDirectorioParaPokemones();
void crearDirectorioYMetadataPokemonNuevo(char*, char*);
void crearArchivosBinTotalesFS();

// Funciones Sobre Seteos y Liberacion
void establecerLaMetadataDeGC();
void setearMetadataBloques();
void liberarMapeoEnMemoria();
void liberarDatosFileSystem();

// Funciones Sobre el BitArray
int buscarBloqueLibre();
void liberarBloqueParticion(char*);

// Funciones de Retorno de Direcciones
char* retornarDireccionBloqueAEscribir(int, char**);
char* retornarNuevaListaBloques(char**, int, int);
char* retornarDireccionPokemon(char*);
char* retornarMetadataPokemon(char*);
char* retornarDireccionBloque(char*);

// Funciones Sobre Los Diferentes Mensajes
t_exclusion_pokemon* obtenerMutexDelPokemon(char*);
int retornarCantidadDeBlocksEnParticion(char**);
bool reducirCantidadPosicionPokemon(t_msg_1*);
t_list* obtenerListaDePosiciones(char*);

// Funciones Sobre Archivos
char* escribirBloquesEnArchivoTemporal(char*, char**);
void reescribirDentroDeLosBloquesAsignados(int, int, char**, FILE*);
void recrearBloqueVacio(char*);
bool puedeAccederAlArchivo(char*, char*);
void cerrarArchivo(char*, char*);

// Cierre del Proceso
void finalizarPrograma();
void eliminarMutexPokemon(t_exclusion_pokemon*);

#endif
