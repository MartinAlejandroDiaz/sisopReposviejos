/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"

/*
 * Recibe un paquete a serializar, y un puntero a un int en el que dejar
 * el tamaño del stream de bytes serializados que devuelve
 */
void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	void * magic = malloc(*bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(uint8_t));
	desplazamiento+= sizeof(uint8_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

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

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	printf("[eze borrar] - Numero de Socket de conexion: %d\n", socket_cliente);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = broker_NP; //////// !!!!! modificar aca !!!!!!!!! /////////////
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje)+1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t);

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	printf("[eze borrar] - Mensaje enviado: { Tamanio: %d, Contenido: '%s'}\n", paquete->buffer->size,(char*)(paquete->buffer->stream));
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

char* recibir_mensaje(int socket_cliente)
{
	uint8_t cod_op;
	int size;
	void * buffer = NULL;

	recv(socket_cliente, &cod_op, sizeof(uint8_t), MSG_WAITALL); // recibe codigo de operacion
	printf("[eze borrar] - Mensaje recibido: { CodigoOperacion: %d }\n", cod_op);

	switch (cod_op) {

	case MENSAJE: // el paquete recibido es de tipo MENSAJE

		recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL); // recibe tamanio mensaje
		printf("[eze borrar] - Mensaje recibido: { Tamanio: %d }\n", size);

		buffer = malloc(size);

		recv(socket_cliente, buffer, size, MSG_WAITALL); // recibe contenido del mensaje
		printf("[eze borrar] - Mensaje recibido: { Contenido: '%s' }\n", (char*)buffer);

		break;
	case 0:
		return NULL; // Se cerró la conexion del Servidor
	}

	return buffer;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
