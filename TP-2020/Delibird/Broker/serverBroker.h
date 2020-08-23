/*
 * serverBroker.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef SERVERBROKER_H_
#define SERVERBROKER_H_

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <pthread.h>
#include "libBroker.h"
#include "memoria.h"
#include "../utilCommons/clientSide/clientUtils.h"
#include "../utilCommons/serverSide/serverUtils.h"
#include "../utilCommons/compartidos.h"

// Variables globales
pthread_t threadServidor;
pthread_mutex_t mutex_ids;
pthread_mutex_t mutex_mensajes;
uint32_t ID_CORRELATIVO;

typedef struct {
	uint32_t id_mensaje_correlativo;
	uint32_t confirmacion;
} t_msg_broker_CAUP;

typedef struct {
	uint32_t pokemon_length;
	char* pokemon;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t id_mensaje_correlativo;
} t_msg_broker_AP;

typedef struct {
	char* ip;
	char* puerto;
	int socket;
}t_suscriptor;

typedef struct {
	t_suscriptor* ptr_suscriptor;
	uint32_t flag_ACK;
}t_msg_suscriptor;

typedef struct {
	t_suscriptor* ptr_suscriptor;
	uint32_t flag_ACK;
	t_buffer * buffer;
	int socket;
	uint8_t cod_operacion;
	t_msg_suscriptor * msg_suscriptor;
}t_paramsHiloEnviar;

typedef struct t_msg t_msg;

// Funciones
void iniciar_servidor();
void server_aceptar_clientes(int socket_servidor);
void* server_atender_cliente(void* socket);
void server_procesar_mensaje(uint8_t codOp, int socket);
t_sol_suscripcion* deserializar_solicitud_suscripcion(t_buffer* buffer,int socket);
uint32_t generar_id();
t_buffer* serializar_msg_broker_AP(t_msg_1* msg);
t_buffer* serializar_msg_broker_CAUP(t_msg_4* msg);
//void confirmar_suscripcion(char* colaMensaje,int socket, uint8_t op_code);
t_buffer* deserializar_y_serializar_segun_op(uint8_t cod_op,char* mensaje,uint32_t largoMensaje,uint32_t idMensaje);
t_msg_2* deserializar_msg_broker_NP_sinID (char* stream);
t_msg_3b* deserializar_msg_broker_GP_sinID (char* stream);
t_msg_1* deserializar_msg_broker_CATP_sinID (char* stream);
t_buffer* serializar_msg_broker_NP(t_msg_2* objeto,uint32_t idMensaje);
t_buffer* serializar_msg_broker_GP(t_msg_3* objeto,uint32_t idMensaje);
t_buffer* serializar_msg_broker_CATP(t_msg_1* objeto,uint32_t idMensaje);
void enviar_mensaje_a_suscriptos(t_buffer* buffer, t_msg* nodo_mensaje, uint8_t cod_operacion);
int enviar_paquete(t_buffer* buffer, int socket, uint8_t op_code);
t_buffer* serializar_msg_broker_LP(t_msg_5* objeto);
uint8_t obtenerCodOp(char* nombreProceso, char* colaMensaje);
//void enviar_mensaje_al_suscriptor(uint32_t largoMensaje, char* mensaje, uint8_t op_code, int socket, t_msg_suscriptor* msg_suscriptor);
void enviarRecibirConfirmacion(t_paramsHiloEnviar *paramsHiloEnviar);
t_buffer *serializar_mensaje(t_msg *mensaje);
void enviarConfirmacionGameboy(int socket, char *colaMensaje);
bool esGameBoy(int socket, char *colaMensaje);

#endif /* SERVERBROKER_H_ */
