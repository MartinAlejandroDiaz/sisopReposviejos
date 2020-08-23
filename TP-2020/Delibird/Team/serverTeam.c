#include "serverTeam.h"
#include "planificador.h"

void iniciar_servidor_team()
{
	int socket_servidor;

	socket_servidor = server_bind_listen_team(datosConfigTeam->ip_team, datosConfigTeam->puerto_team);

    while(1)
    	server_aceptar_clientes_team(socket_servidor);
}

void server_aceptar_clientes_team(int socket_servidor) {

	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread,NULL,(void*)server_atender_cliente_team, (void*)socket_cliente);
	pthread_detach(thread);
}

void* server_atender_cliente_team(void* socket){

	int socketFD = (int) socket;
	uint8_t codigoOp;

	codigoOp = recibir_cod_op_team(socketFD);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigoOp;
	paquete->buffer = procesarRestoDelContenido(socketFD);

	t_socketPaquete* contenidoRecibido = malloc(sizeof(t_socketPaquete));
	contenidoRecibido->paquete = paquete;
	contenidoRecibido->socket = socketFD;

	server_procesar_mensaje_team(contenidoRecibido);

	return NULL;
}

void server_procesar_mensaje_team(t_socketPaquete* paqueteRecibido) {

	t_msg_1* msg_AP;
	t_msg_CAUP* msg_CAUP;
	t_msg_5* msg_LP;

		switch (paqueteRecibido->paquete->codigo_operacion) {
		case SERVER_MENSAJE:
			break;
		case team_AP:
			msg_AP = deserializar_msg_AP(paqueteRecibido->paquete->buffer);
			log_info(logger, "Llego un APPEARED_POKEMON: Id=%d - Pokemon: %s - Posicion (%d;%d)", msg_AP->idMensaje, msg_AP->pokemon, msg_AP->posicionX, msg_AP->posicionY);
			pthread_mutex_lock(&m_mensajeAppeared);
			inclusionPokemonACapturar(msg_AP->pokemon, msg_AP->posicionX,msg_AP->posicionY);
			pthread_mutex_unlock(&m_mensajeAppeared);
			free(msg_AP->pokemon);
			free(msg_AP);
			break;
		case broker_CAUP:
			msg_CAUP = deserializar_msg_CAUP(paqueteRecibido->paquete->buffer);
			log_info(logger, "Llego un CAUGHT_POKEMON: Id=%d - Confirmacion: %s", msg_CAUP->idMensajeCorrelativo, msg_CAUP->confirmacion);
			validarSiSePudoCapturar(msg_CAUP);
			free(msg_CAUP->confirmacion);
			free(msg_CAUP);
			break;
		case broker_LP:
			msg_LP = deserializar_msg_broker_LP(paqueteRecibido->paquete->buffer->stream);
			log_info(logger, "Llego un LOCALIZED_POKEMON: Id=%d - Pokemon: %s - Cantidad Coordenadas: %d", msg_LP->idMensaje, msg_LP->pokemon, msg_LP->cantidad_coordenadas);
			pthread_mutex_lock(&m_mensajeLocalized);
			guardarMensajeLocalized(msg_LP);
			pthread_mutex_unlock(&m_mensajeLocalized);
			liberarCantidadDeCoordenadas(msg_LP);
			break;

		case 0:
			pthread_exit(NULL);
		}

		free(paqueteRecibido->paquete->buffer->stream);
		free(paqueteRecibido->paquete->buffer);
		free(paqueteRecibido->paquete);
		free(paqueteRecibido);
}


uint8_t recibir_cod_op_team(int socket){
	uint8_t codigoOp;

	recv(socket, &codigoOp, sizeof(uint8_t), 0);

	return codigoOp;
}


int server_bind_listen_team(char* ip, char* puerto)
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


void suscribir_proceso_team(char* colaMensaje){

	t_buffer* bufferCreado = serializar_solicitud_suscripcion(colaMensaje,datosConfigTeam->ip_team,datosConfigTeam->puerto_team);

	int socket = enviar_paquete_team(bufferCreado,colaMensaje, "SUBSCRIPCION");

	pthread_t hiloSuscripcion;
	pthread_create(&hiloSuscripcion, NULL, (void*) &recibirMensajesSuscripcion ,(void*) socket);
	pthread_detach(hiloSuscripcion);
}

int enviar_paquete_team(t_buffer* buffer, char* colaMensaje, char* accion){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) devolverEncabezadoMensajeTeam(colaMensaje, accion);
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
		if ((socket_broker = crear_conexion(datosConfigTeam->ip_broker,
				datosConfigTeam->puerto_broker)) != -1) {
			log_info(logger, "Me Conecte al Broker Con el Socket Nro°%d", socket_broker);
			cantidadBytes = send(socket_broker, envio,
					buffer->size + sizeof(uint8_t) + sizeof(uint32_t), MSG_NOSIGNAL);
		} else {
			log_error(logger, "No se pudo Conectar al Broker - Reintentando Conexion");
			sleep(datosConfigTeam->tiempo_reconexion);
		}
	} while (cantidadBytes == -1);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return socket_broker;
}

int enviar_paquete_CATCH(t_buffer* buffer, t_entrenador* entrenador){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) broker_CATP;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	int socket_broker;

	if((socket_broker = crear_conexion(datosConfigTeam->ip_broker, datosConfigTeam->puerto_broker)) == -1){

		log_error(logger, "No se pudo Conectar al Broker, se realizara el CATCH por DEFAULT");

		entrenador->catchEnviado = retornarMensajeCatch(entrenador, -1);

		free(envio);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		return socket_broker;
	}

	send(socket_broker, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), MSG_NOSIGNAL);

	// Espero a Recibir el Id que el BROKER me genera
	uint32_t idMensaje;
	recv(socket_broker, &idMensaje, sizeof(uint32_t), 0);

	entrenador->catchEnviado = retornarMensajeCatch(entrenador, idMensaje);

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return socket_broker;
}

t_msg_1* retornarMensajeCatch(t_entrenador* entrenador, uint32_t idMensaje){

	t_msg_1* mensajeCaptura = malloc(sizeof(t_msg_1));
	mensajeCaptura->pokemon = string_duplicate(entrenador->pokemonCaptura->nombre);
	mensajeCaptura->posicionX = entrenador->pokemonCaptura->posicion_x;
	mensajeCaptura->posicionY = entrenador->pokemonCaptura->posicion_y;
	mensajeCaptura->idMensaje = idMensaje;

	return mensajeCaptura;
}

void recibirMensajesSuscripcion(void* socket){

	int socketRecepcion = (int) socket;

	while(1){

		uint8_t codigoOp = recibir_cod_op_team(socketRecepcion);

		t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = codigoOp;
		paquete->buffer = procesarRestoDelContenido(socketRecepcion);

		uint8_t confirmacionDeRecepcion = (uint8_t) confirmacion_recepcion;
		send(socketRecepcion, &confirmacionDeRecepcion, sizeof(uint8_t), MSG_NOSIGNAL);

		t_socketPaquete* contenidoRecibido = malloc(sizeof(t_socketPaquete));
		contenidoRecibido->paquete = paquete;
		contenidoRecibido->socket = socketRecepcion;

		pthread_t hiloRecepcion;
		pthread_create(&hiloRecepcion, NULL, (void*) server_procesar_mensaje_team, contenidoRecibido);
		pthread_detach(hiloRecepcion);
	}
}

int devolverEncabezadoMensajeTeam(char* colaMensaje, char* accion) {

	if (string_contains(accion, "SUBSCRIPCION")) {

		return subscripcion_inicial;
	}

	else {

		if (string_contains(colaMensaje, "GET_POKEMON")) {

			return broker_GP;
		}

		else if (string_contains(colaMensaje, "CATCH_POKEMON")) {

			return broker_CATP;
		}

		return 0;
	}
}

t_msg_1* deserializar_msg_AP(t_buffer* buffer){

	t_msg_1* appearedPokemon = malloc(sizeof(t_msg_1));

	void* stream = buffer->stream;

	memcpy(&(appearedPokemon->pokemon_length),stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	appearedPokemon->pokemon = malloc(appearedPokemon->pokemon_length);
	memcpy(appearedPokemon->pokemon, stream, appearedPokemon->pokemon_length);
	stream += appearedPokemon->pokemon_length;

	memcpy(&(appearedPokemon->posicionX), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(appearedPokemon->posicionY), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	if(stream != NULL){
	memcpy(&(appearedPokemon->idMensaje), stream, sizeof(uint32_t));
	}

	return appearedPokemon;
}

void inclusionPokemonACapturar(char* nombre, uint32_t posicion_x, uint32_t posicion_y){

	if(loNecesitoGlobalmente(nombre) && necesitoAgregarPokemon(nombre)){

		t_pokemon* nodePokemon = retornarEstructuraDePokemon(nombre, posicion_x, posicion_y);

		list_add(pokemonesYaRecibidos, string_duplicate(nombre));

		list_add(listaPokemonRequeridos, nodePokemon);

		sem_post(&pokemonsACapturar);
	}

	// Esta opcion esta por si necesito al Pokemon, pero tengo para capturar la cantidad NECESARIA
	// las posibles posiciones sobrantes las tengo que guardar por las dudas
	else if(loNecesitoGlobalmente(nombre) && (!necesitoAgregarPokemon(nombre))){

		t_pokemon* nodePokemon = retornarEstructuraDePokemon(nombre, posicion_x, posicion_y);
		list_add(listaPokemonDeRespaldo, nodePokemon);
	}
}

// Esta funcion es en caso que el CAUGHT FALLE, entonces valido si en la Lista de Respaldo
// Tengo una posicion GUARDADA sobrante y la agrego a la Lista de Pokemons Requerido para que lo CAPTUREN
void mandarAPlanificarUnPokemonAuxiliar(char* nombrePokemon){

	bool existeUnPokemonSobrante(t_pokemon* pokemonSobrante){
		return string_contains(pokemonSobrante->nombre, nombrePokemon);
	}

	t_pokemon* pokemon = list_find(listaPokemonDeRespaldo, (void*) existeUnPokemonSobrante);

	if(pokemon != NULL){
		list_add(listaPokemonRequeridos, pokemon);
		sem_post(&pokemonsACapturar);
	}
}

bool estePokemonYaLoRecibiPorAlgunMensaje(char* nombrePokemon){

	bool esElMismo(char* pokemonObjetivo){
		return string_contains(pokemonObjetivo, nombrePokemon);
	}

	return list_any_satisfy(pokemonesYaRecibidos, (void*) esElMismo);
}


void guardarMensajeLocalized(t_msg_5* localizedPokemon) {

	if (!loNecesitoGlobalmente(localizedPokemon->pokemon)) {
		log_info(logger, "No se Requiere Capturar un %s",
				localizedPokemon->pokemon);
		return;
	}

	if (!estePokemonYaLoRecibiPorAlgunMensaje(localizedPokemon->pokemon)) {

		for (int i = 0; i < localizedPokemon->cantidad_coordenadas; i++) {
			t_coordenadas* coordenadas = list_get(localizedPokemon->coordenadas,
					i);

			pthread_mutex_lock(&m_mensajeAppeared);
			inclusionPokemonACapturar(localizedPokemon->pokemon,
					coordenadas->posicionX, coordenadas->posicionY);
			pthread_mutex_unlock(&m_mensajeAppeared);
		}
	}
}

t_buffer* serializar_msg_CATP(t_pokemon* pokemonCatch){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoNombrePokemon = strlen(pokemonCatch->nombre) + 1;

	buffer->size = sizeof(uint32_t) * 3
				 + largoNombrePokemon;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, pokemonCatch->nombre, largoNombrePokemon);
	offset += largoNombrePokemon;
	memcpy(stream + offset, &pokemonCatch->posicion_x, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pokemonCatch->posicion_y, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	return buffer;
}

t_msg_CAUP* deserializar_msg_CAUP (t_buffer* buffer){

	t_msg_CAUP* mensaje = malloc(sizeof(t_msg_CAUP));
	uint32_t resultado;

	void* stream = buffer->stream;

	memcpy(&(mensaje->idMensajeCorrelativo), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&resultado, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	mensaje->confirmacion = devolverResultadoComoString(resultado);

	return mensaje;
}

char* devolverResultadoComoString(uint32_t resultado){

	if (resultado == 1) {
		return string_duplicate("OK");
	}

	else {
		return string_duplicate("FAIL");
	}
}

void validarSiSePudoCapturar(t_msg_CAUP* caughtPokemon){

	bool estanEsperandoUnaCaptura(t_entrenador* entrenador){
		return entrenador->catchEnviado != NULL;
	}

	bool correspondeElIdDelMensaje(t_entrenador* entrenadorABuscar){

		return entrenadorABuscar->catchEnviado->idMensaje == caughtPokemon->idMensajeCorrelativo;
	}

	t_list* entrenadoresConCaptura = list_filter(colaBlocked->elements, (void*) estanEsperandoUnaCaptura);

	t_entrenador* entrenadorObjetivo = list_find(entrenadoresConCaptura, (void*) correspondeElIdDelMensaje);
	list_destroy(entrenadoresConCaptura);

	if(entrenadorObjetivo != NULL){

		// En caso de tratarse con SJF, Calculo el Nuevo Estimador en base al CPU ejecutado
		// y el estimador anterior que utilizo
		calcularNuevoEstimadorSiEsSJF(entrenadorObjetivo);

		// Para cumplir con la Metrica Nro°3 del TP
		entrenadorObjetivo->totalCPUEjecutado += entrenadorObjetivo->cantidadCPUEjecutado;
		entrenadorObjetivo->cantidadCPUEjecutado = 0;

		if(string_contains(caughtPokemon->confirmacion, "OK")){

			iterarAccionesAlCapturarConExito(entrenadorObjetivo);
		}

		else if(string_contains(caughtPokemon->confirmacion, "FAIL")){

			char* copiaNombrePokemon = string_duplicate(entrenadorObjetivo->pokemonCaptura->nombre);
			liberarMensajeSobreCatch(entrenadorObjetivo);
			entrenadorObjetivo->catchEnviado = NULL;
			entrenadorObjetivo->estaOcupado = false;
			mandarAPlanificarUnPokemonAuxiliar(copiaNombrePokemon);
			free(copiaNombrePokemon);
			sem_post(&puedeSeleccionarEntrenador);
		}

	}
	else { log_error(logger, "El Id: %d del Mensaje Recibido No corresponde a ningun Entrenador en el Team", caughtPokemon->idMensajeCorrelativo);}
}

// En esta funcion meto toda la Logica asociada a cuando se captura un Pokemon
// Ya sea por un CATCH DEFAULT o por CAUGHT 'OK'
// Verificando despues si con eso puede finalizar o debe continuar
void iterarAccionesAlCapturarConExito(t_entrenador* entrenador){

	actualizarObjetivosGlobalesDelTeam(entrenador->catchEnviado->pokemon);
	agregarPokemonComoObtenido(entrenador, entrenador->catchEnviado->pokemon);
	actualizarObjetivosDelEntrenador(entrenador->pokemon_objetivos, entrenador->catchEnviado->pokemon, entrenador->identificador);
	liberarMensajeSobreCatch(entrenador);
	entrenador->catchEnviado = NULL;
	entrenador->cantidad_maxima_a_capturar--;
	entrenador->estaOcupado = false;

	if(list_size(entrenador->pokemon_objetivos) == 0){

		entrenador->objetivosCumplidos = true;
		t_entrenador* entrenadorDesencolado = desencolarEntrenadorCercano(colaBlocked, entrenador, m_colaBlocked);
		entrenadorDesencolado->estado = FINISHED;
		encolarEntrenador(colaFinished,entrenadorDesencolado,m_colaFinished);
		log_info(logger, "Se Paso Al Entrenador N°%d A La Cola FINISHED habiendo FINALIZADO sus Objetivos", entrenadorDesencolado->identificador);
		sem_post(&cantidadEntrenadoresEnTeam);
	}

		if(entrenador->cantidad_maxima_a_capturar != 0){
		sem_post(&puedeSeleccionarEntrenador);
		}
		sem_post(&esperarCapturas);
}

void liberarCantidadDeCoordenadas(t_msg_5* mensajeLP){

	list_iterate(mensajeLP->coordenadas, (void*) free);
	list_destroy(mensajeLP->coordenadas);
	free(mensajeLP->pokemon);
	free(mensajeLP);
}

void seFinalizaronLasCapturas(){

	int cantidadCapturas = retornarCantidadTotalDeCaptura();

	for(int i=0; i < cantidadCapturas; i++){
		sem_wait(&esperarCapturas);
	}

	if(list_size(objetivosGlobalesTeam) == 0){
		log_info(logger, "Se capturaron todos los pokemones del TEAM");
		int i=0;

		while(i < queue_size(colaBlocked)){

			t_entrenador* entrenadorBloqueado = list_get(colaBlocked->elements, i);
			int cantidadNoNecesitada = list_size(entrenadorBloqueado->pokemon_no_necesitados);
			for(int j=0; j < cantidadNoNecesitada; j++){
				sem_post(&entrenadoresEnDeadlock);
			}
			i++;
		}
		sem_post(&puedeIniciarAlgoritmo);
	}
}

int retornarCantidadTotalDeCaptura() {

	int i = 0;
	int contadorCapturas = 0;

	while (i < list_size(objetivosGlobalesTeam)) {
		t_node_pokemon* pokemon = list_get(objetivosGlobalesTeam, i);
		contadorCapturas += pokemon->cantidad_pokemon;
		i++;
	}

	return contadorCapturas;
}
