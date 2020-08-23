#include "serverGameCard.h"

void iniciar_servidor_GC()
{
	int socket_servidor;

	socket_servidor = server_bind_listen_GC(datosConfigGameCard->ipGameCard, datosConfigGameCard->puertoGameCard);

    while(necesitaContinuar)
    	server_aceptar_clientes_GC(socket_servidor);
}

void server_aceptar_clientes_GC(int socket_servidor) {

	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread,NULL,(void*)server_atender_cliente_GC, (void*)socket_cliente);
	pthread_detach(thread);
}

void* server_atender_cliente_GC(void* socket){

	int socketFD = (int) socket;

    uint8_t codigoOp = recibir_cod_op_GC(socketFD);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigoOp;
	paquete->buffer = procesarRestoDelContenido(socketFD);

	t_socketPaquete* contenidoRecibido = malloc(sizeof(t_socketPaquete));
	contenidoRecibido->paquete = paquete;
	contenidoRecibido->socket = socketFD;

	server_procesar_mensaje_GC(contenidoRecibido);

	return NULL;
}

void server_procesar_mensaje_GC(t_socketPaquete* paqueteRecibido) {

	t_msg_NP* newPokemon;
	t_msg_1 * catchPokemon;
	t_msg_3 * getPokemon;

		switch (paqueteRecibido->paquete->codigo_operacion) {
		case SERVER_MENSAJE:
			break;
		case gamecard_NP:
			newPokemon = deserializarPorNewPokemon(paqueteRecibido->paquete->buffer, paqueteRecibido->socket);
			agregarNuevaAparicionPokemon(newPokemon);
			enviarSobreAppearedPokemon(newPokemon);
			free(newPokemon->nombrePokemon);
			free(newPokemon);
			break;
		case gamecard_CP:
			catchPokemon = deserializarPorCatchPokemon(paqueteRecibido->paquete->buffer, paqueteRecibido->socket);
			bool respuesta = capturarPokemon(catchPokemon);
			enviarSobreCaughtPokemon(catchPokemon->idMensaje, respuesta);
			free(catchPokemon->pokemon);
			free(catchPokemon);
			break;
		case gamecard_GP:
			getPokemon = deserializarPorGetPokemon(paqueteRecibido->paquete->buffer, paqueteRecibido->socket);
			t_list* listaPosiciones = obtenerTodasLasPosicionesDelPokemon(getPokemon);
			enviarSobreLocalizedPokemon(getPokemon, listaPosiciones);
			free(getPokemon->pokemon);
			free(getPokemon);
			break;
		case 0:
			pthread_exit(NULL);
		}

		free(paqueteRecibido->paquete->buffer->stream);
		free(paqueteRecibido->paquete->buffer);
		free(paqueteRecibido->paquete);
		free(paqueteRecibido);
}

t_msg_NP* deserializarPorNewPokemon(t_buffer* bufferRecibido, int socket){

	t_msg_NP* newPokemon = malloc(sizeof(t_msg_NP));

	void* stream = bufferRecibido->stream;

	memcpy(&(newPokemon->largoNombrePokemon),stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	newPokemon->nombrePokemon = malloc(newPokemon->largoNombrePokemon);
	memcpy(newPokemon->nombrePokemon, stream, newPokemon->largoNombrePokemon);
	stream += newPokemon->largoNombrePokemon;

	memcpy(&(newPokemon->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(newPokemon->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(newPokemon->cantidad), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(newPokemon->idMensaje), stream, sizeof(uint32_t));

	return newPokemon;
}

t_msg_1* deserializarPorCatchPokemon(t_buffer* bufferRecibido, int socket){

	t_msg_1* catchPokemon = malloc(sizeof(t_msg_1));

	void* stream = bufferRecibido->stream;

	memcpy(&(catchPokemon->pokemon_length),stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	catchPokemon->pokemon = malloc(catchPokemon->pokemon_length);
	memcpy(catchPokemon->pokemon, stream, catchPokemon->pokemon_length);
	stream += catchPokemon->pokemon_length;

	memcpy(&(catchPokemon->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(catchPokemon->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(catchPokemon->idMensaje), stream, sizeof(uint32_t));

	return catchPokemon;
}

t_msg_3* deserializarPorGetPokemon(t_buffer* bufferRecibido, int socket){

	t_msg_3* getPokemon = malloc(sizeof(t_msg_3));

	void* stream = bufferRecibido->stream;

	memcpy(&(getPokemon->pokemon_length),stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	getPokemon->pokemon = malloc(getPokemon->pokemon_length);
	memcpy(getPokemon->pokemon, stream, getPokemon->pokemon_length);
	stream += getPokemon->pokemon_length;

	memcpy(&(getPokemon->idMensaje), stream, sizeof(uint32_t));

	return getPokemon;
}

uint8_t recibir_cod_op_GC(int socket){
	uint8_t codigoOp;

	recv(socket, &codigoOp, sizeof(uint8_t), 0);

	return codigoOp;
}

int server_bind_listen_GC(char* ip, char* puerto)
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

void suscribir_proceso_GC(char* colaMensaje) {

	t_buffer* bufferCreado = serializar_solicitud_suscripcion(colaMensaje,
			datosConfigGameCard->ipGameCard,
			datosConfigGameCard->puertoGameCard);
	int socket;

	if ((socket = enviar_paquete_GC(bufferCreado, colaMensaje, "SUBSCRIPCION"))
			!= -1) {

		pthread_t hiloSuscripcion;
		pthread_create(&hiloSuscripcion, NULL,
				(void*) &recibirMensajesSuscripcion, (void*) socket);
		pthread_detach(hiloSuscripcion);

	}
	sem_post(&salidaSubscripcion);
}

int enviar_paquete_GC(t_buffer* buffer, char* colaMensaje, char* accion){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) devolverEncabezadoMensaje(colaMensaje, accion);
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	int socket_broker;
	int cantidadBytes = -1;

	do {
		if ((socket_broker = crear_conexion(datosConfigGameCard->ipBroker,
				datosConfigGameCard->puertoBroker)) != -1) {
			cantidadBytes = send(socket_broker, envio,
					buffer->size + sizeof(uint8_t) + sizeof(uint32_t), MSG_NOSIGNAL);
		} else {
			sleep(datosConfigGameCard->tiempo_de_reintento_reconexion);

			if(!necesitaContinuar){
				free(envio);
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);
				return socket_broker;
			}
		}
	} while (cantidadBytes == -1 && string_contains(accion, "SUBSCRIPCION"));

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return socket_broker;
}

void recibirMensajesSuscripcion(void* socket){

	int socketRecepcion = (int) socket;

	while(necesitaContinuar){

		uint8_t codigoOp = recibir_cod_op_GC(socketRecepcion);

		t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = codigoOp;
		paquete->buffer = procesarRestoDelContenido(socketRecepcion);

		uint8_t confirmacionDeRecepcion = (uint8_t) confirmacion_recepcion;
		send(socketRecepcion, &confirmacionDeRecepcion, sizeof(uint8_t), MSG_NOSIGNAL);

		t_socketPaquete* contenidoRecibido = malloc(sizeof(t_socketPaquete));
		contenidoRecibido->paquete = paquete;
		contenidoRecibido->socket = socketRecepcion;

		pthread_t hiloRecepcion;
		pthread_create(&hiloRecepcion, NULL, (void*) server_procesar_mensaje_GC, contenidoRecibido);
		pthread_detach(hiloRecepcion);
	}

		close(socketRecepcion);
}

void enviarSobreAppearedPokemon(t_msg_NP* pokemon){

	t_buffer* bufferAppeared = serializar_sobre_appeared_pokemon(pokemon);

	enviar_paquete_GC(bufferAppeared, "APPEARED_POKEMON", "MENSAJE");
}

t_buffer* serializar_sobre_appeared_pokemon(t_msg_NP* pokemon){

		t_buffer* buffer = malloc(sizeof(t_buffer));

		uint32_t largoNombrePokemon = string_length(pokemon->nombrePokemon) + 1;

		buffer->size = sizeof(uint32_t) * 4
					 + largoNombrePokemon;

		void* stream = malloc(buffer->size);
		int offset = 0;

		memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, pokemon->nombrePokemon, largoNombrePokemon);
		offset += largoNombrePokemon;
		memcpy(stream + offset, &pokemon->posicionX, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &pokemon->posicionY, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &pokemon->idMensaje, sizeof(uint32_t));

		buffer->stream = stream;
		return buffer;
}

int devolverEncabezadoMensaje(char* colaMensaje, char* accion) {

	if (string_contains(accion, "SUBSCRIPCION")) {

		return subscripcion_inicial;
	}

	else {

		if (string_contains(colaMensaje, "APPEARED_POKEMON")) {

			return broker_AP;
		}

		else if (string_contains(colaMensaje, "CAUGHT_POKEMON")) {

			return broker_CAUP;
		}

		else if (string_contains(colaMensaje, "LOCALIZED_POKEMON")) {

			return broker_LP;
		}
		return 0;
	}
}

void enviarSobreCaughtPokemon(uint32_t idMensaje, bool respuesta){

	t_buffer* bufferAppeared = serializar_sobre_caught_pokemon(idMensaje, respuesta);

	enviar_paquete_GC(bufferAppeared, "CAUGHT_POKEMON", "MENSAJE");
}

t_buffer* serializar_sobre_caught_pokemon(uint32_t idMensaje, bool respuesta) {

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t resultado = (uint32_t) respuesta;

	buffer->size = sizeof(uint32_t) * 2;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &resultado, sizeof(uint32_t));

	buffer->stream = stream;
	return buffer;
}

void enviarSobreLocalizedPokemon(t_msg_3* pokemon, t_list* listaPosiciones){

	t_buffer* bufferLocalized = serializar_sobre_localized_pokemon(pokemon, listaPosiciones);

	enviar_paquete_GC(bufferLocalized, "LOCALIZED_POKEMON", "MENSAJE");

	list_destroy_and_destroy_elements(listaPosiciones, (void*) free);
}

t_buffer* serializar_sobre_localized_pokemon(t_msg_3* pokemon, t_list* listaPosiciones){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoNombrePokemon = string_length(pokemon->pokemon) + 1;

	buffer->size = sizeof(uint32_t) * 3
				 + largoNombrePokemon;

	int cantidadPosiciones = list_size(listaPosiciones);

	if(cantidadPosiciones != 0){
		buffer->size += 2* sizeof(uint32_t) * cantidadPosiciones;
	}

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &pokemon->idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, pokemon->pokemon, largoNombrePokemon);
	offset += largoNombrePokemon;

	memcpy(stream + offset, &cantidadPosiciones, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	// Iterar En Base A La Cantidad De Coordenadas
	if (cantidadPosiciones != 0) {
		for (int i = 0; i < cantidadPosiciones; i++) {

	t_posiciones* posiciones = list_get(listaPosiciones, i);
	memcpy(stream + offset, &(posiciones->posicionX), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(posiciones->posicionY), sizeof(uint32_t));
	offset += sizeof(uint32_t);

		}
	}

	buffer->stream = stream;
	return buffer;
}
