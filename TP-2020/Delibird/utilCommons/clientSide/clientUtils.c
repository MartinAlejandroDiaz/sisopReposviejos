#include <stdio.h>
#include <stdlib.h>
#include "clientUtils.h"

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void* serializar_paquete(t_paquete* paquete, int *bytes){

	void * magic = malloc(*bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = SERVER_MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje)+1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


void suscribir_proceso(char* colaMensaje,int socket_Broker, char *ip, char* puerto){

	t_buffer* bufferCreado = serializar_solicitud_suscripcion(colaMensaje,ip,puerto);

	enviar_paquete_suscripcion(bufferCreado,colaMensaje,socket_Broker);
}


t_buffer* serializar_solicitud_suscripcion(char* colaMensaje,char *ip, char* puerto){

		t_buffer* buffer = malloc(sizeof(t_buffer));

		uint32_t largoColaMensaje = strlen(colaMensaje) + 1;
		uint32_t largoIP = strlen(ip) + 1;
		uint32_t largoPuerto = strlen(puerto) + 1;

		buffer->size = sizeof(uint32_t)*3
				     + largoColaMensaje
					 + largoIP
					 + largoPuerto;

		void* stream = malloc(buffer->size);
		int offset = 0;

		memcpy(stream + offset, &largoColaMensaje, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, colaMensaje, largoColaMensaje);
		offset += largoColaMensaje;
		memcpy(stream + offset, &largoIP, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, ip, largoIP);
		offset += largoIP;
		memcpy(stream + offset, &largoPuerto, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, puerto, largoPuerto);
		offset += largoPuerto;

		buffer->stream = stream;
		return buffer;
}



void enviar_paquete_suscripcion(t_buffer* buffer, char* colaMensaje,int socket_Broker){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) subscripcion_inicial;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	send(socket_Broker, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

int enviar_paquete_con_id_respuesta(t_buffer* buffer, int socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	int cantBytesEnviados;

	paquete->codigo_operacion = (uint8_t) confirmacion_recepcion;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	cantBytesEnviados = send(socket, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

//	printf("Id de respuesta enviado: { Tamanio: %d, Contenido: '%s'}\n", paquete->buffer->size,(char*)(paquete->buffer->stream));

	close(socket);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return cantBytesEnviados;
}

t_buffer* serializar_envio_id(uint32_t id){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);
	memcpy(stream, &id, sizeof(uint32_t));

	buffer->stream = stream;

	return buffer;
}

int retornar_id(uint32_t id, int socket){

	int cantBytesEnviados;
	t_buffer* buffer = serializar_envio_id(id);

	cantBytesEnviados = enviar_paquete_con_id_respuesta(buffer, socket);

	return cantBytesEnviados;
}

t_buffer* procesarRestoDelContenido(int socket){

	t_buffer* bufferRecibido = malloc(sizeof(t_buffer));

	recv(socket, &(bufferRecibido->size), sizeof(uint32_t), 0);
	bufferRecibido->stream = malloc(bufferRecibido->size);
	recv(socket, bufferRecibido->stream,bufferRecibido->size, 0);

	return bufferRecibido;
}



