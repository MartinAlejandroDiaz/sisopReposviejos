/*
 * memoria.h
 *
 *  Created on: 14 may. 2020
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


#include <sys/time.h>
#include "libBroker.h"
#include "serverBroker.h"
#include "../utilCommons/compartidos.h"
#include <math.h>


// Tipos de datos

typedef struct {
	char* ptr_cache;
	uint32_t tamanio;
}t_particion_libre;

typedef struct t_msg {
	unsigned long long timestamp;
	uint32_t id_mensaje;
	uint8_t op_code;
	uint32_t mensaje_length;
	uint32_t particion_length;
	char* mensaje;
	t_list* suscriptores;
} t_msg;

pthread_mutex_t mutex_agregar_msg_a_memoria;
pthread_mutex_t esperar_confirmacion;

// Funciones
void inicializarMemoria();
//t_msg* agregar_a_memoria(uint32_t id, uint8_t cod_op, uint32_t tamStream, char *stream);
unsigned long long obtenerTimeStamp();
//t_list* generar_lista_suscriptos(int cod_op);
t_list* suscriptos_segun_cod_op(int cod_op);

char * obtenerPosicionLibreEnCache(uint32_t largoMensaje);
void fragmentarParticionLibre(t_particion_libre* ptrParticionLibre, uint32_t largoMensaje, int indice);
void ordenarParticionesLibresSegunTamanio();
t_particion_libre * newParticionLibre(char* ptrCache, uint32_t largoMensaje);
void agregar_a_memoria (t_msg* nodo_mensaje);
t_list* generar_lista_suscriptos(t_list* lista_de_suscritores);
char* escribir_en_cache(uint32_t tam_particion, void *src, uint8_t codOp, uint32_t largo_pokemon);
char * busquedaParticionLibre(uint32_t largoMensaje);
void ordenarParticionesLibresSegunInicio();
void mover_particion(t_particion_libre* particionActual,t_particion_libre* particionSiguiente);
int compactar ();
void eliminarParticionLRU ();
void consolidar_particiones_dinamicas (t_msg* msg);
void consolidar_buddy_system (t_msg* msg);
void eliminarParticionFIFO ();
void agregar_mensaje_a_nodo(t_msg * nodo_msg,char* stream);
t_msg * inicializar_nodo_mensaje (uint32_t id, uint8_t cod_op);
t_msg* agregar_a_lista_memoria(uint32_t id, uint8_t cod_op, uint32_t tam_msg ,uint32_t tam_particion, char* ptr_cache);
double exponente_buddy (int largo_mensaje);
uint32_t obtener_tam_msg(uint8_t codOp, uint32_t pokemon_length, uint32_t cantidad_coordenadas);
void cache_imprimir1();
uint32_t obtener_tam_particion(uint32_t tam_msg);
t_particion_libre *obtenerParticionCompaniero(t_particion_libre *nuevaPartLibre);
t_particion_libre *unificar(t_particion_libre *nuevaPartLibre, t_particion_libre *partCompaniero);
int obtenerTamCacheBuddy();

#endif /* MEMORIA_H_ */
