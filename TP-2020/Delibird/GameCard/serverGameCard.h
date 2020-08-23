#ifndef GAMECARD_SERVERGAMECARD_H_
#define GAMECARD_SERVERGAMECARD_H_

#include <commons/collections/list.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include "libGameCard.h"
#include "../utilCommons/compartidos.h"
#include "../utilCommons/serverSide/serverUtils.h"

typedef struct {
	uint32_t largoNombrePokemon;
	char* nombrePokemon;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t cantidad;
	uint32_t idMensaje;
}t_msg_NP;

pthread_t thread;
bool necesitaContinuar;

// Funciones Sobre Servidor GC
void iniciar_servidor_GC();
int server_bind_listen_GC(char*, char*);
void server_aceptar_clientes_GC(int);
void* server_atender_cliente_GC(void*);
void server_procesar_mensaje_GC(t_socketPaquete*);

// Funciones Sobre Lo que Recibe y Procesa
uint8_t recibir_cod_op_GC(int);
t_msg_NP* deserializarPorNewPokemon(t_buffer*, int);
t_msg_1* deserializarPorCatchPokemon(t_buffer*, int);
t_msg_3* deserializarPorGetPokemon(t_buffer*, int);
void recibirMensajesSuscripcion(void*);

// Funciones Sobre Lo Que Serializa y Envia
int devolverEncabezadoMensaje(char*, char*);
void suscribir_proceso_GC(char*);
int enviar_paquete_GC(t_buffer*, char*, char*);
t_buffer* serializar_sobre_appeared_pokemon(t_msg_NP*);
void enviarSobreAppearedPokemon(t_msg_NP*);
t_buffer* serializar_sobre_caught_pokemon(uint32_t, bool);
void enviarSobreCaughtPokemon(uint32_t, bool);
t_buffer* serializar_sobre_localized_pokemon(t_msg_3*, t_list*);
void enviarSobreLocalizedPokemon(t_msg_3*, t_list*);

// Funciones Al Recibir Sobre New Pokemon
void agregarNuevaAparicionPokemon(t_msg_NP*);
void agregarPosicionPokemon(t_msg_NP*);
void actualizarCantidadPokemon(t_msg_NP*, int, t_config*, char*);
void actualizarOAgregarKey(char*, t_msg_NP*);

// Funciones Al Recibir Sobre Catch Pokemon
bool capturarPokemon(t_msg_1*);

// Funciones Al Recibir Sobre Get Pokemon
t_list* obtenerTodasLasPosicionesDelPokemon(t_msg_3* getPokemon);

#endif /* GAMECARD_SERVERGAMECARD_H_ */
