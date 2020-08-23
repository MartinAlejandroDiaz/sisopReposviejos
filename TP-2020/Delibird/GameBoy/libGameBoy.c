#include <stdio.h>
#include <stdlib.h>
#include "libGameBoy.h"


void creacion_logger(){

	char* path = "gameBoy.log";
	char* nombrePrograma = "GameBoy";

	logger = log_create(path , nombrePrograma, 1, LOG_LEVEL_INFO);
}

t_config_gameBoy* read_and_log_config(){

	t_config* archivo_Config = config_create("gameBoy.config");

	if(archivo_Config == NULL){
		log_error(logger, "No se encuentra la direccion del Archivo de Configuracion");
		exit(1);
	}

	datosConfigGameBoy = malloc(sizeof(t_config_gameBoy));

	datosConfigGameBoy->ipBroker = config_get_string_value(archivo_Config, "IP_BROKER");
	datosConfigGameBoy->puertoBroker = config_get_string_value(archivo_Config, "PUERTO_BROKER");
	datosConfigGameBoy->ipTeam = config_get_string_value(archivo_Config, "IP_TEAM");
	datosConfigGameBoy->puertoTeam = config_get_string_value(archivo_Config, "PUERTO_TEAM");
	datosConfigGameBoy->ipGameCard = config_get_string_value(archivo_Config, "IP_GAMECARD");
	datosConfigGameBoy->puertoGameCard = config_get_string_value(archivo_Config, "PUERTO_GAMECARD");
	datosConfigGameBoy->configuracion = archivo_Config;

	return datosConfigGameBoy;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------//
// Funciones Sobre Proceso y Cola Que Se Envia

void enviar_sobre_CATP_o_AP(char* nombreProceso, char* colaMensaje, char* nombrePokemon, char* posicionX, char* posicionY, char* idMensaje){

	t_buffer* bufferCreado;

	if(idMensaje == NULL){
		bufferCreado = serializar_sobre_cuatro_parametros(nombrePokemon, (uint32_t) atoi(posicionX), (uint32_t) atoi(posicionY), (uint32_t) NULL);
	}
	else{
		bufferCreado = serializar_sobre_cuatro_parametros(nombrePokemon, (uint32_t) atoi(posicionX), (uint32_t) atoi(posicionY), (uint32_t) atoi(idMensaje));
	}

	enviar_paquete(bufferCreado, nombreProceso, colaMensaje);

	loggearSobreEnvioAlBroker(nombrePokemon, colaMensaje, nombreProceso);
}

void enviar_sobre_NP(char* nombreProceso, char* colaMensaje,
		char* nombrePokemon, char* posicionX, char* posicionY, char* cantidad,
		char* idMensaje) {

	t_buffer* bufferCreado;

	if (idMensaje == NULL) {
		bufferCreado = serializar_sobre_cinco_parametros(nombrePokemon, (uint32_t) atoi(posicionX),
				(uint32_t) atoi(posicionY), (uint32_t) atoi(cantidad),
				(uint32_t) NULL);
	}

	else {
		bufferCreado = serializar_sobre_cinco_parametros(nombrePokemon, (uint32_t) atoi(posicionX),
				(uint32_t) atoi(posicionY), (uint32_t) atoi(cantidad),
				(uint32_t) atoi(idMensaje));
	}

	enviar_paquete(bufferCreado, nombreProceso, colaMensaje);

	loggearSobreEnvioAlBroker(nombrePokemon, colaMensaje, nombreProceso);
}

void loggearSobreEnvioAlBroker(char* pokemon, char* colaMensaje, char* nombreProceso){

	if(string_contains(nombreProceso, "BROKER")){
	log_info(logger, "Se envio el mensaje sobre %s a la cola de mensajes: %s al %s",pokemon, colaMensaje, nombreProceso);
	}
}

void enviar_a_Broker_sobre_CAUP(char* nombreProceso, char* colaMensaje, char* id_mensaje, char* confirmacion){

	int respuesta = retornar_por_ok_o_fail(confirmacion);

	t_buffer* bufferCreado = serializar_sobre_tres_parametros((uint32_t) atoi(id_mensaje), (uint32_t) respuesta);

	enviar_paquete(bufferCreado, nombreProceso, colaMensaje);

	log_info(logger, "Se envio la confirmacion a la cola de mensajes: %s al %s", colaMensaje, nombreProceso);
}

void enviar_sobre_GP(char* nombreProceso, char* colaMensaje,
		char* nombrePokemon, char* idMensaje) {

	t_buffer* bufferCreado;

	if (idMensaje == NULL) {
		bufferCreado = serializar_sobre_dos_parametros(nombrePokemon, (uint32_t) NULL);
	}

	else {
		bufferCreado = serializar_sobre_dos_parametros(nombrePokemon, (uint32_t) atoi(idMensaje));
	}

	enviar_paquete(bufferCreado, nombreProceso, colaMensaje);

	loggearSobreEnvioAlBroker(nombrePokemon, colaMensaje, nombreProceso);
}

// FIN
//---------------------------------------------------------------------------------------------------------------------------------------------------//
// Funciones Sobre Las Distintas Serializaciones A Realizar Por Proceso-Cola

t_buffer* serializar_sobre_dos_parametros(char* nombrePokemon, uint32_t idMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoNombrePokemon = strlen(nombrePokemon) + 1;

	buffer->size = sizeof(uint32_t) * 2
				 + largoNombrePokemon;

	if(&idMensaje != NULL){
		buffer->size += sizeof(uint32_t);
	}

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombrePokemon, largoNombrePokemon);
	if(&idMensaje != NULL){
	offset += largoNombrePokemon;
	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	}

	buffer->stream = stream;

	return buffer;
}

t_buffer* serializar_sobre_tres_parametros(uint32_t id_mensaje, uint32_t confirmacion){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t) * 2;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &id_mensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &confirmacion, sizeof(uint32_t));

	buffer->stream = stream;
	return buffer;

}

t_buffer* serializar_sobre_cuatro_parametros(char* nombrePokemon, uint32_t posX, uint32_t posY, uint32_t idMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoPokemon = strlen(nombrePokemon) + 1;

	buffer->size = sizeof(uint32_t) * 3
				 + largoPokemon;

	if(&idMensaje != NULL) { buffer->size += sizeof(uint32_t); }

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoPokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombrePokemon, largoPokemon);
	offset += largoPokemon;
	memcpy(stream + offset, &posX, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &posY, sizeof(uint32_t));
	if(&idMensaje != NULL){
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	}

	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_sobre_cinco_parametros(char* nombrePokemon, uint32_t posicionX, uint32_t posicionY, uint32_t cantidad, uint32_t idMensaje){

		t_buffer* buffer = malloc(sizeof(t_buffer));

		uint32_t largoNombrePokemon = strlen(nombrePokemon) + 1;

		buffer->size = sizeof(uint32_t) * 4
					 + largoNombrePokemon;
		if(&idMensaje != NULL) { buffer->size += sizeof(uint32_t); }

		void* stream = malloc(buffer->size);
		int offset = 0;

		memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, nombrePokemon, largoNombrePokemon);
		offset += largoNombrePokemon;
		memcpy(stream + offset, &posicionX, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &posicionY, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &cantidad, sizeof(uint32_t));
		if(&idMensaje != NULL){
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
		}

		buffer->stream = stream;
		return buffer;
}

// FIN
//---------------------------------------------------------------------------------------------------------------------------------------------------//
// Funcion Unica Para El Envio De Paquete

void enviar_paquete(t_buffer* buffer, char* nombreProceso, char* colaMensaje){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) retornar_header_para_paquete(nombreProceso, colaMensaje);
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	int bytesEnviados = -1;
	int socketConexion;

	do{

	if((socketConexion = retornar_socket_sobre_proceso(nombreProceso)) != -1){
	bytesEnviados = send(socketConexion, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);
	}

	}while(bytesEnviados == -1);

	recibirConfirmacionBroker(nombreProceso, socketConexion);

	close(socketConexion);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// FIN
//---------------------------------------------------------------------------------------------------------------------------------------------------//

int retornar_header_para_paquete(char* nombreProceso, char* colaMensaje) {

	if (string_contains(nombreProceso, "BROKER")) {
		if (string_contains(colaMensaje, "NEW_POKEMON")) {
			return broker_NP;
		}

		else if (string_contains(colaMensaje, "APPEARED_POKEMON")) {
			return broker_AP;
		}

		else if (string_contains(colaMensaje, "CATCH_POKEMON")) {
			return broker_CATP;
		}

		else if (string_contains(colaMensaje, "CAUGHT_POKEMON")) {
			return broker_CAUP;
		}

		else {
			return broker_GP;
		}
	}

	else if (string_contains(nombreProceso, "TEAM")) {
		return team_AP;
	}

	else if (string_contains(nombreProceso, "GAMECARD")) {

		if (string_contains(colaMensaje, "NEW_POKEMON")) {
			return gamecard_NP;
		}

		else if (string_contains(colaMensaje, "CATCH_POKEMON")) {
			return gamecard_CP;
		}

		else {
			return gamecard_GP;
		}

	}

	else {
		return SERVER_MENSAJE;
	}
}

int retornar_socket_sobre_proceso(char* nombreProceso) {

	int conexionSocket;

	if (string_contains(nombreProceso, "BROKER")) {
		conexionSocket = crear_conexion(datosConfigGameBoy->ipBroker, datosConfigGameBoy->puertoBroker);
	}

	else if (string_contains(nombreProceso, "TEAM")) {
		conexionSocket = crear_conexion(datosConfigGameBoy->ipTeam, datosConfigGameBoy->puertoTeam);
	}

	else if (string_contains(nombreProceso, "GAMECARD")) {
		conexionSocket = crear_conexion(datosConfigGameBoy->ipGameCard, datosConfigGameBoy->puertoGameCard);
	}

	if(conexionSocket == -1){
		log_info(logger, "No me pude conectar al Proceso = %s", nombreProceso);
		return conexionSocket;
	}

	log_info(logger, "Me Conecte al Proceso = %s Con el Socket = %d", nombreProceso, conexionSocket);

	return conexionSocket;
}

int retornar_por_ok_o_fail(char* confirmacion) {

	if (string_contains(confirmacion, "OK")) {
		return 1;
	} else {
		return 0;
	}
}

// Funcion para el uso de Scripts en BASH
void ejecutarParametrosSobreScript(int cantidadArgumentos, char** listaArgumentos){

	if(cantidadArgumentos <= 3){
		log_error(logger, "Faltan Parametros Para Ejecutar %s", listaArgumentos[0]);
		return;
	}

	palabra palabraReservada = transformarStringToEnum(listaArgumentos[1]);

	switch (palabraReservada){

	case BROKER:

		if(string_contains(listaArgumentos[2], "NEW_POKEMON")){
			enviar_sobre_NP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], listaArgumentos[6], NULL);
		}
		else if(string_contains(listaArgumentos[2], "APPEARED_POKEMON")){
			enviar_sobre_CATP_o_AP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], listaArgumentos[6]);
		}
		else if(string_contains(listaArgumentos[2], "CATCH_POKEMON")){
			enviar_sobre_CATP_o_AP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], NULL);
		}
		else if(string_contains(listaArgumentos[2], "CAUGHT_POKEMON")){
			enviar_a_Broker_sobre_CAUP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4]);
		}
		else if(string_contains(listaArgumentos[2], "GET_POKEMON")){
			enviar_sobre_GP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], NULL);
		}


		break;

	case TEAM:

		enviar_sobre_CATP_o_AP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], NULL);
		break;

	case GAMECARD:

		if(string_contains(listaArgumentos[2], "NEW_POKEMON")){
			enviar_sobre_NP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], listaArgumentos[6], listaArgumentos[7]);
		}
		else if(string_contains(listaArgumentos[2], "CATCH_POKEMON")){
			enviar_sobre_CATP_o_AP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4], listaArgumentos[5], listaArgumentos[6]);
		}
		else if(string_contains(listaArgumentos[2], "GET_POKEMON")){
			enviar_sobre_GP(listaArgumentos[1], listaArgumentos[2], listaArgumentos[3], listaArgumentos[4]);
		}

		break;

	case SUSCRIPTOR:

		iniciarModoSuscripcion(listaArgumentos[2], listaArgumentos[3]);

		break;

	default:
		log_info(logger, "Comando No Reconocido");
		break;
	}
}

void recibirConfirmacionBroker(char *proceso, int socketConexion) {
	if (string_contains(proceso, "BROKER"))
	{
		uint8_t rtaConfirmacion = 0;
		int bytesRecibidos = recv(socketConexion, &rtaConfirmacion, sizeof(uint8_t), 0);
	}
}
// Funciones Sobe la Suscripcion en GameBoy
void iniciarModoSuscripcion(char* colaMensaje, char* tiempoConexion){

	t_buffer* buffer = serializar_suscripcion(colaMensaje);

	int socketConexion = enviar_paquete_GB(buffer, colaMensaje);

	t_suscripto* suscripcion = malloc(sizeof(t_suscripto));
	suscripcion->socket = socketConexion;
	suscripcion->tiempo = atoi(tiempoConexion);

	pthread_t hiloSuscripcion;
	pthread_create(&hiloSuscripcion, NULL, (void*) &recibirMensajesPorSuscripcion, suscripcion);
	pthread_join(hiloSuscripcion, NULL);
}

t_buffer* serializar_suscripcion(char* colaMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoColaMensaje = strlen(colaMensaje) + 1;

	buffer->size = sizeof(uint32_t) * 2
			     + largoColaMensaje;

	void* stream = malloc(buffer->size);
	int offset = 0;
	int sinIp = 0;

	memcpy(stream + offset, &largoColaMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, colaMensaje, largoColaMensaje);
	offset += largoColaMensaje;
	memcpy(stream + offset, &sinIp, sizeof(uint32_t));

	buffer->stream = stream;
	return buffer;
}

int enviar_paquete_GB(t_buffer* buffer, char* colaMensaje){

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

	int socketConexion;
	int cantidadBytes = -1;

	do {

		if ((socketConexion = retornar_socket_sobre_proceso("BROKER")) != -1) {
			cantidadBytes = send(socketConexion, envio,
					buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);
		}

	} while (cantidadBytes == -1);

	log_info(logger, "Me Suscribi a la Cola de Mensajes de %s", colaMensaje);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return socketConexion;
}

void recibirMensajesPorSuscripcion(t_suscripto* suscripcion){

	time_t tiempoActual = time(NULL);
	time_t tiempoPasado = time(NULL);

	while(suscripcion->tiempo > (tiempoActual - tiempoPasado)){

		recibir_mensaje(suscripcion->socket);
		tiempoActual = time(NULL);

	}

	close(suscripcion->socket);
	log_info(logger, "Nuevo: Actual = %u - Pasado = %u - Total = %u", tiempoActual, tiempoPasado, (tiempoActual - tiempoPasado));
}

void recibir_mensaje(int socket){
		uint8_t codigoOp;

	if(recv(socket, &codigoOp, sizeof(uint8_t), MSG_DONTWAIT) != -1){

		t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = codigoOp;
		paquete->buffer = procesarRestoDelContenido(socket);

		uint8_t confirmacionDeRecepcion = (uint8_t) confirmacion_recepcion;
		send(socket, &confirmacionDeRecepcion, sizeof(uint8_t), MSG_NOSIGNAL);

		t_socketPaquete* contenidoRecibido = malloc(sizeof(t_socketPaquete));
		contenidoRecibido->paquete = paquete;
		contenidoRecibido->socket = socket;

		pthread_t hiloRecepcion;
		pthread_create(&hiloRecepcion, NULL, (void*) server_procesar_mensaje_GB, contenidoRecibido);
		pthread_detach(hiloRecepcion);
	}
}

void server_procesar_mensaje_GB(t_socketPaquete* paqueteRecibido) {

		switch (paqueteRecibido->paquete->codigo_operacion) {
		case SERVER_MENSAJE:
			break;
		case broker_CAUP:
			break;
		case broker_LP:;
			break;
		case team_AP:
			break;
		case gamecard_NP:
			break;
		case gamecard_CP:
			break;
		case gamecard_GP:
			break;

		case 0:
			pthread_exit(NULL);
		}

		free(paqueteRecibido->paquete->buffer->stream);
		free(paqueteRecibido->paquete->buffer);
		free(paqueteRecibido->paquete);
		free(paqueteRecibido);
}

palabra transformarStringToEnum(char* palReservada) {

	palabra palabraTransformada;

	if (string_contains(palReservada, "BROKER")) {
		palabraTransformada = BROKER;
	}

	else if (string_contains(palReservada, "TEAM")) {
		palabraTransformada = TEAM;
	}

	else if (string_contains(palReservada, "GAMECARD")) {
		palabraTransformada = GAMECARD;
	}

	else if (string_contains(palReservada, "SUSCRIPTOR")) {
		palabraTransformada = SUSCRIPTOR;
	}

	else if (string_contains(palReservada, "SALIR")) {
		palabraTransformada = SALIR;
	}

	return palabraTransformada;
}
