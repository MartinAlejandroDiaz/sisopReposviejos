/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include"serverUtils.h"

void server_iniciar_servidor(char* ip, char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    while(1)
    	server_esperar_cliente(socket_servidor);
}

void server_esperar_cliente(int socket_servidor) {

	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread,NULL,(void*)server_serve_client, (void*)socket_cliente);
	pthread_detach(thread);
}

void* server_serve_client(void* socket){

	int socketFD = (int) socket;
	uint8_t codigoOp;

	recv(socketFD, &codigoOp, sizeof(uint8_t), 0);

	server_process_request(codigoOp, socketFD);

	return NULL;
}

// Esta Funcion Habra Que Modificarlo Para Cada Proceso Que Lo Use Con Sus Casos
void server_process_request(uint8_t codOp, int socket) {

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->codigo_operacion = codOp;

	void* msg = NULL;
		switch (paquete->codigo_operacion) {
		case SERVER_MENSAJE:
			break;
		case broker_NP:
			printf("Llego a broker_NP\n");
			break;
		case broker_AP:
			printf("Llego a broker_AP\n");
			break;
		case broker_CATP:
			printf("Llego a broker_CATP\n");
			break;
		case broker_CAUP:
			printf("Llego broker_CAUP\n");
			break;
		case broker_GP:
			printf("Llego a broker_GP\n");
			break;
		case team_AP:
			printf("Llego a team_AP\n");
			deserializarPorDentro(paquete->buffer, socket);
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
			break;
		case gamecard_NP:
			printf("Llego a gamecard_NP\n");
			break;
		case gamecard_CP:
			printf("Llego a gamecard_CP\n");
			break;
		case gamecard_GP:
			printf("Llego a gamecard_GP\n");
			break;
		case subscripcion_inicial:
			printf("Llego a la suscripcion\n");
			break;
		case 0:
			pthread_exit(NULL);
		}
}

void deserializarPorDentro(t_buffer* buffer, int socket){

	t_msg_1* objeto = malloc(sizeof(t_msg_1));

	recv(socket, &(buffer->size), sizeof(uint32_t), 0);
	buffer->stream = malloc(buffer->size);
	recv(socket, buffer->stream,buffer->size, 0);

	void* stream = buffer->stream;

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));

	printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);
	printf("Posicion X = %d - Posicion Y = %d\n", objeto->posicionX, objeto->posicionY);
}

void* server_serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void server_devolver_mensaje(void* payload, int size, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = SERVER_MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = server_serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

int server_bind_listen(char* ip, char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

uint8_t recibir_cod_op(int socket){
	uint8_t codigoOp;

	recv(socket, &codigoOp, sizeof(uint8_t), 0);

	return codigoOp;
}


t_msg_1* deserializar_msg_broker_AP (char* stream){

	t_msg_1* objeto = malloc(sizeof(t_msg_1));

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);
	//printf("Posicion X = %d - Posicion Y = %d\n", objeto->posicionX, objeto->posicionY);
	//printf("ID = %d\n", objeto->idMensaje);

	return objeto;
}

t_msg_1* deserializar_msg_gameboy_AP (char* stream){

	t_msg_1* objeto = malloc(sizeof(t_msg_1));
	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	uint32_t aux = 0;
	memcpy(&(objeto->idMensaje), &aux, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);
	//printf("Posicion X = %d - Posicion Y = %d\n", objeto->posicionX, objeto->posicionY);
	//printf("ID = %d\n", objeto->idMensaje);

	return objeto;
}

t_msg_4* deserializar_msg_broker_CAUP (char* stream){

	t_msg_4* objeto = malloc(sizeof(t_msg_4));

	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->confirmacion), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//printf("ID = %d\n", objeto->idMensaje);
	//printf("Confirmacion = %d\n", objeto->confirmacion);

	return objeto;

}


t_msg_1* deserializar_msg_broker_CATP (char* stream){

	t_msg_1* objeto = malloc(sizeof(t_msg_1));

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);
	//printf("Posicion X = %d - Posicion Y = %d\n", objeto->posicionX, objeto->posicionY);
	//printf("ID = %d\n", objeto->idMensaje);

	return objeto;
}

t_msg_2* deserializar_msg_broker_NP (char* stream){

	t_msg_2* objeto = malloc(sizeof(t_msg_2));

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->cantidad), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	//printf("Empiezo a deserializar el mensaje...\n");
	//printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);
	//printf("Posicion X = %d - Posicion Y = %d\n", objeto->posicionX, objeto->posicionY);
	//printf("Cantidad = %d\n", objeto->cantidad);
	//printf("ID = %d\n", objeto->idMensaje);

	return objeto;
}

t_msg_3b* deserializar_msg_broker_GP (char* stream){

	t_msg_3b* objeto = malloc(sizeof(t_msg_3b));

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	//memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	//stream += sizeof(uint32_t);

	//printf("Empiezo a deserializar el mensaje...\n");
	//printf("Tamanio Pokemon = %d - Pokemon Nombre = %s\n", objeto->pokemon_length, objeto->pokemon);

	return objeto;
}

t_msg_5* deserializar_msg_broker_LP (char* stream) {

	t_msg_5* objeto = malloc(sizeof(t_msg_5));

	memcpy(&(objeto->idMensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->cantidad_coordenadas), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Iterar En Base A La Cantidad De Coordenadas
	t_list * coordenadas = list_create();
	objeto->coordenadas = coordenadas;
	for (int i = 0; i < objeto->cantidad_coordenadas; i++) {

		t_coordenadas* coord = malloc(sizeof(t_coordenadas));

		//t_coordenadas* posiciones = list_create(listaPosiciones, i);
		memcpy(&(coord->posicionX), stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);
		memcpy(&(coord->posicionY), stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);

		list_add(coordenadas, coord);
	}

	//printf("ID = %d - Tamanio Pokemon = %d - Pokemon Nombre = '%s' \n", objeto->idMensaje, objeto->pokemon_length, objeto->pokemon);
	//printf("cantidad Coordenadas = %d", objeto->cantidad_coordenadas);
	//for (int j = 0; j < objeto->cantidad_coordenadas; j++) {
	//	t_coordenadas* coord;
	//	coord = list_get(coordenadas, j);
	//	printf(" - [%d:", coord->posicionX);
	//	printf("%d]", coord->posicionY);
	//}
	//printf("\n");

	return objeto;
}


void deserializar_mensaje(t_paquete* paquete, int socket) {

	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

}
