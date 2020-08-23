#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<pthread.h>
#include"../compartidos.h"


typedef struct {
	uint32_t posicionX;
	uint32_t posicionY;
} t_coordenadas;

pthread_t thread;

void* server_recibir_buffer(int*, int);

void server_iniciar_servidor(char*, char*);
int server_bind_listen(char* ip, char* puerto);
void server_esperar_cliente(int);
void* server_recibir_mensaje(int socket_cliente, int* size);
int server_recibir_operacion(int);
void server_process_request(uint8_t, int);
void* server_serve_client(void*);
void* server_serializar_paquete(t_paquete* paquete, int bytes);
void server_devolver_mensaje(void* payload, int size, int socket_cliente);
void deserializarPorDentro(t_buffer* buffer, int);
uint8_t recibir_cod_op(int socket);
void deserializar_mensaje(t_paquete* paquete, int socket);

//broker
t_msg_3b* deserializar_msg_broker_GP (char* stream);
t_msg_2* deserializar_msg_broker_NP (char* stream);
t_msg_1* deserializar_msg_broker_CATP (char* stream);
t_msg_4* deserializar_msg_broker_CAUP (char* stream);
t_msg_1* deserializar_msg_broker_AP (char* stream);
t_msg_5* deserializar_msg_broker_LP (char* stream);

//gameboy
t_msg_1* deserializar_msg_gameboy_AP (char* stream);




#endif
