/*
 * libBroker.h
 *
 *  Created on: 20 abr. 2020
 *      Author: utnso
 */

#ifndef LIBBROKER_H_
#define LIBBROKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include "memoria.h"
#include "serverBroker.h"
#include <signal.h>
#include <unistd.h>
#include <time.h>

// Colores para el printf
#define _RED     "\x1b[31m"
#define _GREEN   "\x1b[32m"
#define _YELLOW  "\x1b[33m"
#define _BLUE    "\x1b[34m"
#define _MAGENTA "\x1b[35m"
#define _CYAN    "\x1b[36m"
#define _RESET   "\x1b[0m"

// Estructuras Definidas
typedef struct {
	uint32_t tamano_memoria; // TAMANO_MEMORIA
	uint32_t tamano_minimo_particion; // TAMANO_MINIMO_PARTICION
	char* algoritmo_memoria; // ALGORITMO_MEMORIA
	char* algoritmo_reemplazo; // ALGORITMO_REEMPLAZO
	char* algoritmo_particion_libre; // ALGORITMO_PARTICION_LIBRE
	char* ip_broker; // IP_BROKER
	char* puerto_broker; // PUERTO_BROKER
	uint32_t frecuencia_compactacion; // FRECUENCIA_COMPACTACION
	char* log_file; // LOG_FILE
	uint32_t debug_mode;
	t_config* configuracion;
}t_config_broker;

// Variables Globales
t_log* logger;
t_config* config;
t_config_broker* datosConfigBroker;
int socket_Broker;
int socket_Team;
int socket_GameCard;

t_list *SUSCRIPTORES_NEWPOK;
t_list *SUSCRIPTORES_CATCHPOK;
t_list *SUSCRIPTORES_GETPOK;
t_list *SUSCRIPTORES_APPEARED;
t_list *SUSCRIPTORES_CAUGHTPOK;
t_list *SUSCRIPTORES_LOCALIZED;

t_list *MENSAJES;
t_list *PARTICIONES_LIBRES;

char *CACHE; //cache de memoria donde se guardan los mensajes
char * randomMalloc; //random malloc para propositos de pruebas

sem_t sem_cache_mensajes;

// Funciones Definidas
void iniciar_logger();
void read_and_log_config();
void liberar_infoConfig();
void inicializar();
t_suscriptor* suscribir_cola(t_sol_suscripcion* sol_suscripcion, int socketSuscriptor, bool *yaSeSuscribioAntes); // retorna nuevo suscriptor
t_suscriptor* obtenerSuscriptor (t_sol_suscripcion* sol_suscripcion, t_list* lista_global);
t_list* list_segun_mensaje_suscripcion(char*);
t_list* list_segun_cod_operacion (int);
void dump ();
void controlador_de_senial (int n);
char* retornarFechaHora ();
void liberarMemoria();
void liberarEstructuraSuscriptor(t_suscriptor*);
void liberarEstructuraMensaje(t_msg*);

#endif /* LIBBROKER_H_ */
