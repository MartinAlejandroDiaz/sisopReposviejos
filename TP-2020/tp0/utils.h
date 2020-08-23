/*
 * conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

typedef enum
{
	MENSAJE=1,
	broker_NP,
	broker_AP,
	broker_CATP,
	broker_CAUP,
	broker_GP,
	team_AP,
	gamecard_NP,
	gamecard_CP,
	gamecard_GP,
	modo_suscriptor,
	subscripcion_inicial
}op_code;

typedef struct
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	uint8_t codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
char* recibir_mensaje(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(int socket_cliente);

#endif /* UTILS_H_ */
