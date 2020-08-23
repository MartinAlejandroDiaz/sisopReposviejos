#ifndef CLIENTSIDE_CLIENTUTILS_H_
#define CLIENTSIDE_CLIENTUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "../compartidos.h"
#include <commons/string.h>


int crear_conexion(char *, char*);
void* serializar_paquete(t_paquete*, int*);
void enviar_mensaje(char*, int);
void suscribir_proceso(char*,int,char*,char*);
t_buffer* serializar_solicitud_suscripcion(char*,char*,char*);
int retornar_header_para_suscripcion(char*);
void enviar_paquete_suscripcion(t_buffer*, char*,int);

int enviar_paquete_con_id_respuesta(t_buffer* buffer, int socket);
t_buffer* serializar_envio_id(uint32_t id);
int retornar_id(uint32_t id, int socket);
t_buffer* procesarRestoDelContenido(int socket);

#endif
