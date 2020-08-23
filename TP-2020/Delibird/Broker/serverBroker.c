/*
 * serverBroker.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "libBroker.h"
#include "serverBroker.h"
#include "../utilCommons/clientSide/clientUtils.h"
#include "../utilCommons/serverSide/serverUtils.h"

void iniciar_servidor()
{
	int socket_servidor;

	socket_servidor = server_bind_listen(datosConfigBroker->ip_broker, datosConfigBroker->puerto_broker);

    while(1)
    	server_aceptar_clientes(socket_servidor);
}

void server_aceptar_clientes(int socket_servidor) {

	struct sockaddr_in dir_cliente;
	pthread_t thread;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread,NULL,(void*)server_atender_cliente, (void*)socket_cliente);
	pthread_detach(thread);
}

void* server_atender_cliente(void* socket){

	int socketFD = (int) socket;
	uint8_t codigoOp;

	codigoOp = recibir_cod_op(socketFD);

	server_procesar_mensaje(codigoOp, socketFD);

	return NULL;
}

void server_procesar_mensaje(uint8_t codOp, int socket) {

	log_info(logger, "Nueva conexion");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->codigo_operacion = codOp;
	uint32_t id = 0;
	uint32_t tam_msg;
	uint32_t tam_particion;
	char* ptr_cache;
	t_msg* nodo_mensaje;
	t_buffer* buffer;

	switch (paquete->codigo_operacion) {
	case SERVER_MENSAJE:;
		break;
	case broker_AP:;
		log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes AP");
		deserializar_mensaje(paquete, socket);
		t_msg_1* mensaje_AP = deserializar_msg_broker_AP(paquete->buffer->stream);
		tam_msg = obtener_tam_msg(codOp, mensaje_AP->pokemon_length, 0);
		tam_particion = obtener_tam_particion(tam_msg);

		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_AP, paquete->codigo_operacion, mensaje_AP->pokemon_length);
		enviarConfirmacionGameboy(socket, "APPEARED_POKEMON");

		nodo_mensaje = agregar_a_lista_memoria(mensaje_AP->idMensaje, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);

		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, team_AP);
		pthread_mutex_unlock(&mutex_mensajes);

		free(mensaje_AP->pokemon);
		free(mensaje_AP);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case broker_NP:;
		log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes NP");
		deserializar_mensaje(paquete, socket);
		id=generar_id();

		t_msg_2* mensaje_NP = deserializar_msg_broker_NP_sinID((char*)paquete->buffer->stream); //deserializo msg recibido
		tam_msg = obtener_tam_msg(codOp, mensaje_NP->pokemon_length, 0);
		tam_particion = obtener_tam_particion(tam_msg);

		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_NP, paquete->codigo_operacion, mensaje_NP->pokemon_length);
		enviarConfirmacionGameboy(socket, "NEW_POKEMON");

		nodo_mensaje = agregar_a_lista_memoria(id, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);
		//buffer = serializar_msg_broker_NP(mensaje_NP, id);
		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, gamecard_NP);
		//sem_post(&sem_cache_mensajes);
		pthread_mutex_unlock(&mutex_mensajes);

		free(mensaje_NP->pokemon);
		free(mensaje_NP);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case broker_LP:;
		log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes LP");
		deserializar_mensaje(paquete, socket);
		t_msg_5* mensaje_LP = deserializar_msg_broker_LP((char*)paquete->buffer->stream);
		tam_msg = obtener_tam_msg(codOp, mensaje_LP->pokemon_length, mensaje_LP->cantidad_coordenadas);
		tam_particion = obtener_tam_particion(tam_msg);

		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_LP, paquete->codigo_operacion, mensaje_LP->pokemon_length);
		// construyo el nodo y lo envio
		nodo_mensaje = agregar_a_lista_memoria(mensaje_LP->idMensaje, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);
		//buffer = serializar_msg_broker_LP(mensaje_LP);
		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, broker_LP);
		pthread_mutex_unlock(&mutex_mensajes);

		list_destroy_and_destroy_elements(mensaje_LP->coordenadas, (void*) free);
		free(mensaje_LP->pokemon);
		free(mensaje_LP);

		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case broker_GP:;
		log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes GP");
		deserializar_mensaje(paquete, socket);
		id=generar_id();

		t_msg_3b* mensaje_GP = deserializar_msg_broker_GP_sinID((char*)paquete->buffer->stream); //deserializo msg recibido
		tam_msg = obtener_tam_msg(codOp, mensaje_GP->pokemon_length, 0);
		tam_particion = obtener_tam_particion(tam_msg);

		if (send(socket, &id, sizeof(uint32_t), 0) <= 0) {
			log_error(logger, "Desconexion o error al enviar id correlativo.");
			pthread_exit(NULL);
		} else {
			log_debug(logger, "Se envió el id correlativo del GET POKEMON: %d", id);
		}

		//sem_wait(&sem_cache_mensajes);
		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_GP, paquete->codigo_operacion, mensaje_GP->pokemon_length);
		enviarConfirmacionGameboy(socket, "GET_POKEMON");

		nodo_mensaje = agregar_a_lista_memoria(id, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);
		//buffer = serializar_msg_broker_GP(mensaje_GP, id);
		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, gamecard_GP);
		//sem_post(&sem_cache_mensajes);
		pthread_mutex_unlock(&mutex_mensajes);

		free(mensaje_GP->pokemon);
		free(mensaje_GP);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case broker_CAUP:;
	log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes CAUP");
		deserializar_mensaje(paquete, socket);
		t_msg_4* mensaje_CAUP = deserializar_msg_broker_CAUP(paquete->buffer->stream);
		tam_msg = obtener_tam_msg(codOp, 0, 0);
		tam_particion = obtener_tam_particion(tam_msg);


		//sem_wait(&sem_cache_mensajes);
		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_CAUP, paquete->codigo_operacion, 0);
		enviarConfirmacionGameboy(socket, "CAUGHT_POKEMON");
		nodo_mensaje = agregar_a_lista_memoria(mensaje_CAUP->idMensaje, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);
		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, broker_CAUP);
		//sem_post(&sem_cache_mensajes);
		pthread_mutex_unlock(&mutex_mensajes);

		free(mensaje_CAUP);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case broker_CATP:;
		log_info(logger, "Llegada de un nuevo mensaje a la cola de mensajes CATP");
		deserializar_mensaje(paquete, socket);
		id=generar_id();

		t_msg_1* mensaje_CATP = deserializar_msg_broker_CATP_sinID((char*)paquete->buffer->stream); //deserializo msg recibido
		tam_msg = obtener_tam_msg(codOp, mensaje_CATP->pokemon_length, 0);
		tam_particion = obtener_tam_particion(tam_msg);

		if (send(socket, &id, sizeof(uint32_t), 0) <= 0) {
			log_error(logger, "Desconexion o error al enviar id correlativo.");
			pthread_exit(NULL);
		} else {
			log_debug(logger, "Se envió el id correlativo del CATCH: %d", id);
		}

		//sem_wait(&sem_cache_mensajes);
		pthread_mutex_lock(&mutex_mensajes);
		ptr_cache = escribir_en_cache(tam_particion, mensaje_CATP, paquete->codigo_operacion, mensaje_CATP->pokemon_length);
		enviarConfirmacionGameboy(socket, "CATCH_POKEMON");

		nodo_mensaje = agregar_a_lista_memoria(id, paquete->codigo_operacion, tam_msg, tam_particion, ptr_cache);
		//buffer = serializar_msg_broker_CATP(mensaje_CATP, id);
		buffer = serializar_mensaje(nodo_mensaje);
		enviar_mensaje_a_suscriptos(buffer, nodo_mensaje, gamecard_CP);
		//sem_post(&sem_cache_mensajes);
		pthread_mutex_unlock(&mutex_mensajes);

		free(mensaje_CATP->pokemon);
		free(mensaje_CATP);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		free(buffer->stream);
		free(buffer);
		break;

	case subscripcion_inicial:;
		bool seSuscribePor1eraVez;
		pthread_mutex_lock(&mutex_mensajes);
		t_sol_suscripcion* solicitud_suscripcion = deserializar_solicitud_suscripcion(paquete->buffer,socket);
		t_suscriptor* suscriptor = suscribir_cola(solicitud_suscripcion, socket, &seSuscribePor1eraVez); // devuelve NULL si ya estaba suscripto

		for (int i = 0; i < list_size(MENSAJES); i++) { // Reenvio los mensajes al suscriptor - Multihilo
			t_msg *mensaje = list_get(MENSAJES, i);

			if ( mensaje->op_code == obtenerCodOp("BROKER", solicitud_suscripcion->colaMensaje)) {

				t_msg_suscriptor* msg_suscriptor;

				if (seSuscribePor1eraVez) {

					msg_suscriptor = malloc(sizeof(t_msg_suscriptor));
					msg_suscriptor->ptr_suscriptor = suscriptor;
					msg_suscriptor->flag_ACK = 0;
					list_add(mensaje->suscriptores, msg_suscriptor);

				} else if (!seSuscribePor1eraVez) {

					bool IgualSocket (t_msg_suscriptor* suscriptor){
						return suscriptor->ptr_suscriptor->socket == socket;
					}

					msg_suscriptor = list_find(mensaje->suscriptores,(void*) IgualSocket);
				}

				if (msg_suscriptor->flag_ACK == 0){

					msg_suscriptor->flag_ACK = 1;
					pthread_t hiloEnviarMensaje;
					t_paramsHiloEnviar *paramsHiloEnviar = malloc(sizeof(t_paramsHiloEnviar));
					paramsHiloEnviar->buffer = malloc(sizeof(t_buffer));

					buffer = serializar_mensaje(mensaje);
					paramsHiloEnviar->buffer->size = buffer->size;
					paramsHiloEnviar->buffer->stream = malloc(buffer->size);
					memcpy(paramsHiloEnviar->buffer->stream, buffer->stream, buffer->size);
					paramsHiloEnviar->socket = socket;
					paramsHiloEnviar->cod_operacion = mensaje->op_code;
					paramsHiloEnviar->msg_suscriptor = msg_suscriptor;

					pthread_create(&hiloEnviarMensaje,NULL,(void*)enviarRecibirConfirmacion, paramsHiloEnviar);
					pthread_detach(hiloEnviarMensaje);

					if (string_contains(datosConfigBroker->algoritmo_reemplazo,"LRU")){
						mensaje->timestamp = obtenerTimeStamp();
					}

					free(buffer->stream);
					free(buffer);
				}
			}
		}
		pthread_mutex_unlock(&mutex_mensajes);

		free(solicitud_suscripcion->colaMensaje);
		if ( solicitud_suscripcion->ip_length > 0 ) {
			free(solicitud_suscripcion->ip);
			free(solicitud_suscripcion->puerto);
		}
		free(solicitud_suscripcion);

		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		break;
	case 0:
		pthread_exit(NULL); // del otro lado se cerro la conexion
	}
}


t_sol_suscripcion* deserializar_solicitud_suscripcion(t_buffer* buffer,int socket){

	t_sol_suscripcion* objeto = malloc(sizeof(t_sol_suscripcion));

	recv(socket, &(buffer->size), sizeof(uint32_t), 0);
	buffer->stream = malloc(buffer->size);
	recv(socket, buffer->stream,buffer->size, 0);

	void* stream = buffer->stream;

	memcpy(&(objeto->colaMensaje_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->colaMensaje = malloc(objeto->colaMensaje_length);
	memcpy(objeto->colaMensaje, stream, objeto->colaMensaje_length);
	stream += objeto->colaMensaje_length;

	memcpy(&(objeto->ip_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	if(objeto->ip_length == 0){
		objeto->ip = NULL;
		objeto->puerto = NULL;
		return objeto;
	}

	objeto->ip = malloc(objeto->ip_length);
	memcpy(objeto->ip, stream, objeto->ip_length);
	stream += objeto->ip_length;

	memcpy(&(objeto->puerto_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->puerto = malloc(objeto->puerto_length);
	memcpy(objeto->puerto, stream, objeto->puerto_length);
	stream += objeto->puerto_length;

	return objeto;
}

t_msg_1* deserializar_msg_broker_CATP_sinID (char* stream){

	t_msg_1* objeto = malloc(sizeof(t_msg_1));

	memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	objeto->pokemon = malloc(objeto->pokemon_length);
	memcpy(objeto->pokemon, stream, objeto->pokemon_length);
	stream += objeto->pokemon_length;

	memcpy(&(objeto->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(objeto->posicionY), stream, sizeof(uint32_t));

	return objeto;
}

t_msg_2* deserializar_msg_broker_NP_sinID (char* stream){

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

	log_debug(logger, "Deserializar NEW [Largo|'pokemon'|posX|posY|cant]=[%d|'%s'|%d|%d|%d]", objeto->pokemon_length, objeto->pokemon, objeto->posicionX, objeto->posicionY, objeto->cantidad);

	return objeto;
}

t_msg_3b* deserializar_msg_broker_GP_sinID (char* stream){

		t_msg_3b* objeto = malloc(sizeof(t_msg_3b));

		memcpy(&(objeto->pokemon_length), stream, sizeof(uint32_t));
		stream += sizeof(uint32_t);

		objeto->pokemon = malloc(objeto->pokemon_length);
		memcpy(objeto->pokemon, stream, objeto->pokemon_length);
		stream += objeto->pokemon_length;

		return objeto;
}


t_buffer* serializar_msg_broker_NP(t_msg_2* mensaje_NP, uint32_t idMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = mensaje_NP->pokemon_length + sizeof(uint32_t) * 5;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &mensaje_NP->pokemon_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, mensaje_NP->pokemon, mensaje_NP->pokemon_length);
	offset += mensaje_NP->pokemon_length;
	memcpy(stream + offset, &mensaje_NP->posicionX, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &mensaje_NP->posicionY, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &mensaje_NP->cantidad, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	return buffer;

}


t_buffer* serializar_msg_broker_LP(t_msg_5* objeto){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoNombrePokemon = strlen(objeto->pokemon) + 1;

	buffer->size = sizeof(uint32_t) + sizeof(uint32_t) + objeto->pokemon_length + sizeof(uint32_t)
							+ (objeto->cantidad_coordenadas * 2 *sizeof(uint32_t));
					//[id|len|pokemon|cantCoord|(posx|posy)+]

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &objeto->idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &objeto->pokemon_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, objeto->pokemon, objeto->pokemon_length);
	offset += objeto->pokemon_length;
	memcpy(stream + offset, &objeto->cantidad_coordenadas, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	log_debug(logger, "Envío del mensaje a los suscriptores: [idMensaje|largoNombrePokemon|'pokemon'|cantidad_coordenadas]=[%d|%d|%s|%d]",objeto->idMensaje, objeto->pokemon_length, objeto->pokemon, objeto->cantidad_coordenadas);
	for (int i = 0; i < objeto->cantidad_coordenadas; i++) {
		t_coordenadas* coords = list_get(objeto->coordenadas,i);

		memcpy(stream + offset, &coords->posicionX, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &coords->posicionY, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		log_debug(logger, "coordenadas: [x|y]=[%d|%d]",coords->posicionX, coords->posicionY);
	}

	buffer->stream = stream;
	return buffer;
}


t_buffer* serializar_msg_broker_CATP(t_msg_1* objeto,uint32_t idMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_msg_1* msg = objeto;

	uint32_t largoNombrePokemon = strlen(msg->pokemon) + 1;

	buffer->size = sizeof(uint32_t) * 4
				 + largoNombrePokemon;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, msg->pokemon, largoNombrePokemon);
	offset += largoNombrePokemon;
	memcpy(stream + offset, &msg->posicionX, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &msg->posicionY, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	return buffer;

}


t_buffer* serializar_msg_broker_GP(t_msg_3* objeto,uint32_t idMensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_msg_3* msg = objeto;

	uint32_t largoNombrePokemon = strlen(msg->pokemon) + 1;

	buffer->size = sizeof(uint32_t) * 2
				 + largoNombrePokemon;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, msg->pokemon, largoNombrePokemon);
	offset += largoNombrePokemon;
	memcpy(stream + offset, &idMensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	log_debug(logger, "GET a enviar: tam[Largo|'pokemon']=%d[%d|'%s'|%d]", buffer->size, largoNombrePokemon, msg->pokemon, idMensaje);
	return buffer;

}

t_buffer* serializar_msg_broker_AP(t_msg_1* msg){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoPokemon = strlen(msg->pokemon) + 1;

	buffer->size = sizeof(uint32_t) * 3
				 + largoPokemon;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoPokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, msg->pokemon, largoPokemon);
	offset += largoPokemon;
	memcpy(stream + offset, &msg->posicionX, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &msg->posicionY, sizeof(uint32_t));

	buffer->stream = stream;
	return buffer;

}


t_buffer* serializar_msg_broker_CAUP(t_msg_4* msg){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream, &msg->confirmacion, sizeof(uint32_t));

	buffer->stream = stream;
	return buffer;

}

t_buffer *serializar_mensaje(t_msg *mensaje) {

	int offset = 0;
	t_buffer * buffer = malloc(sizeof(t_buffer));
	char* stream;
	uint32_t pokLen, posx, posy, cantidad, id, confirmacion;
	char * pokemon;

	switch (mensaje->op_code) {
	case broker_NP:;
		log_debug(logger, "serializar new pokemon");
		buffer->size = sizeof(uint32_t) + 1 + mensaje->mensaje_length;  //[len|pokemon|posx|posy|cant|id]
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);

		memcpy(stream + offset, mensaje->mensaje + offset, sizeof(uint32_t));
		pokLen = (uint32_t) stream[offset];
		stream[offset]++;
		offset += sizeof(uint32_t);

		memcpy(stream + offset, mensaje->mensaje + offset, pokLen);
		pokemon = stream + offset;
		offset += pokLen;

		memcpy(stream + offset+1, mensaje->mensaje + offset, sizeof(uint32_t));
		posx = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		memcpy(stream + offset+1, mensaje->mensaje + offset, sizeof(uint32_t));
		posy = (uint32_t)stream[offset+1];
		offset += sizeof(uint32_t);

		memcpy(stream + offset+1, mensaje->mensaje + offset, sizeof(uint32_t));
		cantidad = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		memcpy(stream + offset+1, &(mensaje->id_mensaje), sizeof(uint32_t));
		id = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		log_debug(logger, "Envio de un mensaje NEW [Largo|pokemon|posX|posY|cant|id] = [%d|%s|%d|%d|%d|%d]", pokLen, pokemon, posx, posy, cantidad, id);
		break;
	case broker_LP:;
		log_debug(logger, "serializar localized");
		//TODO probar
		int offsetMensaje;
		buffer->size = sizeof(uint32_t) + 1 + mensaje->mensaje_length;  //[id|len|pokemon|cantCoord|(posx|posy)+]
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);

		memcpy(stream, &(mensaje->id_mensaje), sizeof(uint32_t));
		id = (uint32_t) stream[0];
		offset += sizeof(uint32_t);
		offsetMensaje = 0;

		memcpy(stream + offset, mensaje->mensaje + offsetMensaje, sizeof(uint32_t));
		pokLen = (uint32_t) stream[offset];
		stream[offset]++;
		offset += sizeof(uint32_t);
		offsetMensaje += sizeof(uint32_t);

		memcpy(stream + offset, mensaje->mensaje + offsetMensaje, pokLen);
		pokemon = stream + offset;
		offset += pokLen;
		offsetMensaje += pokLen;

		memcpy(stream + offset+1, mensaje->mensaje + offsetMensaje, sizeof(uint32_t));
		cantidad = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);
		offsetMensaje += sizeof(uint32_t);

		log_debug(logger, "Envio de un mensaje LOCALIZED [id|Largo|'pokemon'|cant de coord] = [%d|%d|'%s'|%d]", id, pokLen, pokemon, cantidad);

		for (int i = 0; i < cantidad; i++) {
			memcpy(stream + offset+1, mensaje->mensaje + offsetMensaje, sizeof(uint32_t));
			posx = (uint32_t) stream[offset+1];
			offset += sizeof(uint32_t);
			offsetMensaje += sizeof(uint32_t);

			memcpy(stream + offset+1, mensaje->mensaje + offsetMensaje, sizeof(uint32_t));
			posy = (uint32_t)stream[offset+1];
			offset += sizeof(uint32_t);
			offsetMensaje += sizeof(uint32_t);

			log_debug(logger, "[posX|posY]=[%d|%d]", posx, posy);
		}

		break;
	case broker_GP:;
		log_debug(logger, "serializar get pokemon");
		//TODO probar
		//buffer->size = sizeof(uint32_t) + 1 + mensaje->mensaje_length; // [len|pok|id]
		buffer->size = sizeof(uint32_t) + 1 + mensaje->mensaje_length; // [len|pok|id]
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);

		memcpy(stream + offset, mensaje->mensaje + offset, sizeof(uint32_t));
		pokLen = (uint32_t) stream[offset];
		stream[offset]++;
		offset += sizeof(uint32_t);

		memcpy(stream + offset, mensaje->mensaje + offset, pokLen); // pokemon
		pokemon = stream + offset;
		offset += pokLen;

		memcpy(stream + offset+1, &(mensaje->id_mensaje), sizeof(uint32_t));
		id = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		log_debug(logger, "Envio de un mensaje GET [len|pokemon|id] = [%d|'%s'|%d]", pokLen, pokemon, id);
		break;
	case broker_AP:
	case broker_CATP:;
		log_debug(logger, "serializar ap o catch");
		//TODO probar
		buffer->size = sizeof(uint32_t) + 1 + mensaje->mensaje_length;  // [tam|pokemon|posx|posy|id]
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);

		memcpy(stream + offset, mensaje->mensaje + offset, sizeof(uint32_t));
		pokLen = (uint32_t) stream[offset];
		stream[offset]++;
		offset += sizeof(uint32_t);

		memcpy(stream + offset, mensaje->mensaje + offset, pokLen);
		pokemon = stream + offset;
		offset += pokLen;

		memcpy(stream + offset+1, mensaje->mensaje + offset, sizeof(uint32_t));
		posx = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		memcpy(stream + offset+1, mensaje->mensaje + offset, sizeof(uint32_t));
		posy = (uint32_t)stream[offset+1];
		offset += sizeof(uint32_t);

		memcpy(stream + offset+1, &(mensaje->id_mensaje), sizeof(uint32_t));
		id = (uint32_t) stream[offset+1];
		offset += sizeof(uint32_t);

		//if (mensaje->op_code == broker_CATP )
		//	log_info(logger, "Llegada de un mensaje CATCH    [len|pok|x|y|id] = [%d|%s|%d|%d|%d]", pokLen, pokemon, posx, posy, id);
		//else
		log_debug(logger, "Envio de un mensaje CATCH/APPEARED [len|pok|x|y|id] = [%d|%s|%d|%d|%d]", pokLen, pokemon, posx, posy, id);

		break;
	case broker_CAUP:;
		log_debug(logger, "serializar caught pokemon");
		//TODO probar
		buffer->size = sizeof(uint32_t) + sizeof(uint32_t); // [id|atrapo?]
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);

		memcpy(stream + offset, &(mensaje->id_mensaje), sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy(stream + offset, mensaje->mensaje, sizeof(uint32_t));
		confirmacion = (uint32_t) stream[offset];
		offset += sizeof(uint32_t);

		log_debug(logger, "Envio de un mensaje CAUGHT [id|confirmacion]=[%d|%d]", mensaje->id_mensaje, confirmacion);
		break;
	default:
		log_error(logger, "Serializar error en cod operacion");
		buffer->size = 1;
		stream = malloc(buffer->size);
		memset(stream, 0x00, buffer->size);
		break;

	}

	buffer->stream = stream;

	return buffer;
}

uint32_t generar_id(){

	uint32_t id;
	pthread_mutex_lock(&mutex_ids);
	id = ID_CORRELATIVO++;
	pthread_mutex_unlock(&mutex_ids);

	log_debug(logger,"Se asigno el id = %d", id);

	return id;
}

void enviar_mensaje_a_suscriptos(t_buffer* buffer, t_msg* nodo_mensaje, uint8_t cod_operacion) {

	int count = 0;
	t_list* list_procesos = nodo_mensaje->suscriptores;

	while (count<list_size(list_procesos)){
		t_msg_suscriptor* msg_suscriptor = list_get(list_procesos,count);
		t_suscriptor* suscriptor = msg_suscriptor->ptr_suscriptor;
		if(msg_suscriptor->flag_ACK==0){

			msg_suscriptor->flag_ACK = 1;
			pthread_t hiloEnviarMensaje;
			t_paramsHiloEnviar *paramsHiloEnviar = malloc(sizeof(t_paramsHiloEnviar));
			paramsHiloEnviar->buffer = malloc(sizeof(t_buffer));
			paramsHiloEnviar->buffer->size = buffer->size;
			paramsHiloEnviar->buffer->stream = malloc(buffer->size);
			memcpy(paramsHiloEnviar->buffer->stream, buffer->stream, buffer->size);
			paramsHiloEnviar->socket = suscriptor->socket;
			paramsHiloEnviar->cod_operacion = cod_operacion;
			paramsHiloEnviar->msg_suscriptor = msg_suscriptor;

			pthread_create(&hiloEnviarMensaje,NULL,(void*)enviarRecibirConfirmacion, paramsHiloEnviar );
			pthread_detach(hiloEnviarMensaje);

		}
		count++;
	}
}

void enviarRecibirConfirmacion(t_paramsHiloEnviar *paramsHiloEnviar) {
	if (enviar_paquete(paramsHiloEnviar->buffer,paramsHiloEnviar->socket, paramsHiloEnviar->cod_operacion) > 0) {
		uint8_t respuesta = 0;

		//pthread_mutex_lock(&esperar_confirmacion);
		if (recv(paramsHiloEnviar->socket, &respuesta, sizeof(uint8_t), 0) > 0){
			if (respuesta == (uint8_t)confirmacion_recepcion) {
				//paramsHiloEnviar->msg_suscriptor->flag_ACK = 1;
				log_debug(logger, "Se recibio confirmacion de recepcion del proceso: %d", paramsHiloEnviar->socket);
			} else {
				log_error(logger, "Confirmacion de recepcion del mensaje errónea: '%d'", respuesta);
			}
		}
		//pthread_mutex_unlock(&esperar_confirmacion);
	}
	free(paramsHiloEnviar->buffer->stream);
	free(paramsHiloEnviar->buffer);
	free(paramsHiloEnviar);
}

int enviar_paquete(t_buffer* buffer, int socket, uint8_t op_code){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	int resultadoEnvio;

	paquete->codigo_operacion = op_code;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	resultadoEnvio = send(socket, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), MSG_NOSIGNAL);
	if (resultadoEnvio <= 0) {
		log_error(logger, "No se pudo conectar con el suscriptor. Socket: %d", socket);
	} else {
		log_info(logger,"Envío de un mensaje a un suscriptor socket: %d (tamaño: %d)", socket, paquete->buffer->size);
	}

	free(envio);
	free(paquete);

	return resultadoEnvio;
}

uint8_t obtenerCodOp(char* nombreProceso, char* colaMensaje) {

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

void enviarConfirmacionGameboy(int socket, char *colaMensaje) {
	if (esGameBoy(socket, colaMensaje)) {
		log_debug(logger, "Se envia al Game Boy confirmacion de recepcion de mensaje.");
		uint8_t confirmacionDeRecepcion = (uint8_t) confirmacion_recepcion;
		send(socket, &confirmacionDeRecepcion, sizeof(uint8_t), MSG_NOSIGNAL);
	}
}

bool esGameBoy(int socket, char *colaMensaje) {
	t_list* lista_global = list_segun_mensaje_suscripcion(colaMensaje);

	bool IgualSocket (t_suscriptor* suscriptor) {
		return suscriptor->socket == socket;
	}
	t_suscriptor *unSuscriptor = list_find(lista_global,(void*) IgualSocket);

	if (unSuscriptor != NULL && unSuscriptor->ip != NULL) {
		return false;
	}

	return true;
}
