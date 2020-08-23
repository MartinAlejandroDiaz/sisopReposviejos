#include "libTeam.h"

void creacion_logger(){

	char* nombrePrograma = "Team";
	logger = log_create(datosConfigTeam->log_file, nombrePrograma, 0, LOG_LEVEL_DEBUG);
}

t_config_team* read_and_log_config(){

	t_config* archivo_Config = config_create("team.config");

	if(archivo_Config == NULL){
		printf("No se encuentra la direccion del Archivo de Configuracion");
		exit(1);
		}

	datosConfigTeam = malloc(sizeof(t_config_team));
	datosConfigTeam->posiciones_entrenadores = config_get_array_value(archivo_Config, "POSICIONES_ENTRENADORES");
	datosConfigTeam->pokemon_entrenadores = config_get_array_value(archivo_Config, "POKEMON_ENTRENADORES");
	datosConfigTeam->objetivos_entrenadores = config_get_array_value(archivo_Config, "OBJETIVOS_ENTRENADORES");
	datosConfigTeam->tiempo_reconexion = config_get_int_value(archivo_Config, "TIEMPO_RECONEXION");
	datosConfigTeam->retardo_ciclo_cpu = config_get_int_value(archivo_Config, "RETARDO_CICLO_CPU");
	datosConfigTeam->algoritmo_planificacion = config_get_string_value(archivo_Config, "ALGORITMO_PLANIFICACION");
	datosConfigTeam->quantum = config_get_int_value(archivo_Config, "QUANTUM");
	datosConfigTeam->alpha = config_get_double_value(archivo_Config, "ALPHA");
	datosConfigTeam->ip_broker = config_get_string_value(archivo_Config, "IP_BROKER");
	datosConfigTeam->puerto_broker = config_get_string_value(archivo_Config, "PUERTO_BROKER");
	datosConfigTeam->estimacion_inicial = config_get_double_value(archivo_Config, "ESTIMACION_INICIAL");
	datosConfigTeam->ip_team = config_get_string_value(archivo_Config, "IP_TEAM");
	datosConfigTeam->puerto_team = config_get_string_value(archivo_Config, "PUERTO_TEAM");
	datosConfigTeam->log_file = config_get_string_value(archivo_Config, "LOG_FILE");
	datosConfigTeam->configuracion = archivo_Config;

	return datosConfigTeam;
}

void establecerCaracteristicasDeLosEntrenadores() {

	bool flag1 = true;

	int cantidadElementos = 0;
	while(datosConfigTeam->pokemon_entrenadores[cantidadElementos] != NULL) { cantidadElementos++; }

	int cantidadEntrenadores = retornarCantidadDeEntrenadores(
			datosConfigTeam->posiciones_entrenadores);

	for (int i = 0; i < cantidadEntrenadores; i++) {

		t_entrenador* entrenador = malloc(sizeof(t_entrenador));
		entrenador->identificador = identificadorEntrenador;
		identificadorEntrenador++;
		entrenador->pokemon_obtenidos = list_create();
		entrenador->pokemon_objetivos = list_create();
		entrenador->pokemon_no_necesitados = list_create();
		entrenador->estimador = datosConfigTeam->estimacion_inicial;
		entrenador->contadorIntercambio = 0;
		entrenador->estimadorAnterior = datosConfigTeam->estimacion_inicial;
		entrenador->objetivosCumplidos = false;
		entrenador->cantidadCPUEjecutado = 0;
		entrenador->totalCPUEjecutado = 0;
		entrenador->estaOcupado = false;
		entrenador->necesitaContinuar = true;
		entrenador->estado = NEW;
		entrenador->pokemonCaptura = NULL;
		entrenador->pokemonACambiar = NULL;
		entrenador->entrenadorIntercambio = NULL;
		entrenador->catchEnviado = NULL;

		char** posiciones = string_split(
				datosConfigTeam->posiciones_entrenadores[i], "|");

		entrenador->posicion_x = atoi(posiciones[0]);
		entrenador->posicion_y = atoi(posiciones[1]);

		string_iterate_lines(posiciones, (void*) free);
		free(posiciones);

		//Para Calcular Cantidad de Pokemon's A Disposicion

		if (cantidadElementos > 0 && datosConfigTeam->pokemon_entrenadores[i] != NULL && flag1) {
			if (string_contains(datosConfigTeam->pokemon_entrenadores[i],
					"|")) {

				char** pokemones = string_split(
						datosConfigTeam->pokemon_entrenadores[i], "|");
				int cantidadPokemones = retornarCantidadDeEntrenadores(
						pokemones);

				for (int j = 0; j < cantidadPokemones; j++) {

					bool flagEncontrado = false;

					if (list_is_empty(entrenador->pokemon_obtenidos)) {
						t_node_pokemon* pokemon = malloc(
								sizeof(t_node_pokemon));
						pokemon->cantidad_pokemon = 1;
						pokemon->nombre_pokemon = string_duplicate(pokemones[j]);

						list_add(entrenador->pokemon_obtenidos, pokemon);
					}

					else {

						int index = 0;
						int cantidadPokemons = list_size(
								entrenador->pokemon_obtenidos);

						while (index < cantidadPokemons) {

							t_node_pokemon* pokemonEnLista = list_get(
									entrenador->pokemon_obtenidos, index);

							if (string_contains(pokemonEnLista->nombre_pokemon,
									pokemones[j])) {
								pokemonEnLista->cantidad_pokemon++;
								flagEncontrado = true;
							}
							index++;
						}

						if (!flagEncontrado) {
							t_node_pokemon* pokemon = malloc(
									sizeof(t_node_pokemon));
							pokemon->cantidad_pokemon = 1;
							pokemon->nombre_pokemon = string_duplicate(pokemones[j]);

							list_add(entrenador->pokemon_obtenidos, pokemon);
						}
					}
				}

				string_iterate_lines(pokemones, (void*) free);
				free(pokemones);
			}

			else if(!string_is_empty(datosConfigTeam->pokemon_entrenadores[i])){
				t_node_pokemon* pokemon = malloc(sizeof(t_node_pokemon));
				pokemon->cantidad_pokemon = 1;
				pokemon->nombre_pokemon =
						string_duplicate(datosConfigTeam->pokemon_entrenadores[i]);

				list_add(entrenador->pokemon_obtenidos, pokemon);
				free(datosConfigTeam->pokemon_entrenadores[i]);
			}
		}
		else
		{
			flag1 = false;
		}

		//Para Calcular Objetivos Totales de Cada Uno

		char** objetivosIndiv = string_split(
				datosConfigTeam->objetivos_entrenadores[i], "|");
		int cantidadPokemonesObjetivos = retornarCantidadDeEntrenadores(
				objetivosIndiv);

		for (int k = 0; k < cantidadPokemonesObjetivos; k++) {

			bool flagEncontrado = false;

			if (list_is_empty(entrenador->pokemon_objetivos)) {
				t_node_pokemon* pokemon = malloc(sizeof(t_node_pokemon));
				pokemon->cantidad_pokemon = 1;
				pokemon->nombre_pokemon = string_duplicate(objetivosIndiv[k]);

				list_add(entrenador->pokemon_objetivos, pokemon);
			}

			else {

				int index2 = 0;
				int cantidadPokemonsObjet = list_size(
						entrenador->pokemon_objetivos);

				while (index2 < cantidadPokemonsObjet) {

					t_node_pokemon* pokemonEnLista = list_get(
							entrenador->pokemon_objetivos, index2);

					if (string_contains(pokemonEnLista->nombre_pokemon,
							objetivosIndiv[k])) {
						pokemonEnLista->cantidad_pokemon++;
						flagEncontrado = true;
					}

					index2++;
				}

				if (!flagEncontrado) {
					t_node_pokemon* pokemonObj = malloc(sizeof(t_node_pokemon));
					pokemonObj->cantidad_pokemon = 1;
					pokemonObj->nombre_pokemon = string_duplicate(objetivosIndiv[k]);

					list_add(entrenador->pokemon_objetivos, pokemonObj);
				}

			}

		}

		string_iterate_lines(objetivosIndiv, (void*) free);
		free(objetivosIndiv);

		// Cantidad Maxima A Obtener Por Entrenador
		establecerLaCantidadMaximaQuePuedeCapturar(entrenador);
		// Establecer Todos Los Objetivos A Capturar del Team
		establecerLosPokemonObjetivoDelTeam(entrenador);

		// Crear Hilo Para Planificar Entrenador
		sem_init(&entrenador->puedeEjecutar, 0, 0);
		pthread_t hiloEntrenador;
		pthread_create(&hiloEntrenador, NULL, (void*) &planificarEntrenador, entrenador);
		pthread_detach(hiloEntrenador);

		pthread_mutex_lock(&m_colaNew);
		queue_push(colaNew, entrenador);
		pthread_mutex_unlock(&m_colaNew);

		contadorEntrenadoresEnTeam++;

		sem_post(&puedeSeleccionarEntrenador);
	}

	string_iterate_lines(datosConfigTeam->objetivos_entrenadores, (void*) free);
}

int retornarCantidadDeEntrenadores(char** entrenadores){

	int cantidadEntrenadores = 0;

	while(entrenadores[cantidadEntrenadores] != NULL){
		cantidadEntrenadores++;
	}
	return cantidadEntrenadores;
}

void establecerLaCantidadMaximaQuePuedeCapturar(t_entrenador* entrenador) {

	int contadorMaximo = 0;
	bool encontrado = false;

	int i = 0;
	while (i < list_size(entrenador->pokemon_objetivos)) {

		t_node_pokemon* pokemon = list_get(entrenador->pokemon_objetivos, i);

		int j = 0;
		while (j < list_size(entrenador->pokemon_obtenidos) && pokemon != NULL) {

			t_node_pokemon* pokemonAComparar = list_get(entrenador->pokemon_obtenidos, j);

			if (string_contains(pokemon->nombre_pokemon,
					pokemonAComparar->nombre_pokemon)) {

				if((pokemon->cantidad_pokemon -= pokemonAComparar->cantidad_pokemon) == 0){

					removerDeLaListaDeObjetivos(entrenador, pokemon->nombre_pokemon);
					i--;
				}

				else {
					contadorMaximo += pokemon->cantidad_pokemon;
				}
				encontrado = true;
				break;
			}

			j++;
		}

		if(!encontrado){
			contadorMaximo += pokemon->cantidad_pokemon;
		}

		i++;
		encontrado = false;
	}

	entrenador->cantidad_maxima_a_capturar = contadorMaximo;
}

void removerDeLaListaDeObjetivos(t_entrenador* entrenador, char* pokemonNombre){

	bool tienenElMismoNombre(t_node_pokemon* pokemon){
		return string_contains(pokemon->nombre_pokemon, pokemonNombre);
	}

	t_node_pokemon* pokemonAEliminar = list_remove_by_condition(entrenador->pokemon_objetivos, (void*) tienenElMismoNombre);

	if(pokemonAEliminar != NULL){
		free(pokemonAEliminar->nombre_pokemon);
		free(pokemonAEliminar);
	}
}

void establecerLosPokemonObjetivoDelTeam(t_entrenador* entrenador) {

	t_list* pokemonesTotales = entrenador->pokemon_objetivos;

	for (int i = 0; i < list_size(pokemonesTotales); i++) {

		bool flagEncontrado = false;

		t_node_pokemon* pokemon = list_get(pokemonesTotales, i);

		if (list_is_empty(objetivosGlobalesTeam)) {

			t_node_pokemon* pokemonPrimero = malloc(sizeof(t_node_pokemon));
			pokemonPrimero->nombre_pokemon = string_duplicate(pokemon->nombre_pokemon);
			pokemonPrimero->cantidad_pokemon = pokemon->cantidad_pokemon;

			list_add(objetivosGlobalesTeam, pokemonPrimero);
		}

		else {

			int j = 0;
			int tamanioLista = list_size(objetivosGlobalesTeam);

			while (j < tamanioLista) {

				t_node_pokemon* pokemonGuardado = list_get(
						objetivosGlobalesTeam, j);

				if (string_contains(pokemon->nombre_pokemon,
						pokemonGuardado->nombre_pokemon)) {
					pokemonGuardado->cantidad_pokemon +=
							pokemon->cantidad_pokemon;
					flagEncontrado = true;
				}
				j++;
			}
			if (!flagEncontrado) {

				t_node_pokemon* pokemonNuevo = malloc(sizeof(t_node_pokemon));
				pokemonNuevo->nombre_pokemon = string_duplicate(pokemon->nombre_pokemon);
				pokemonNuevo->cantidad_pokemon = pokemon->cantidad_pokemon;

				list_add(objetivosGlobalesTeam, pokemonNuevo);
			}
		}

	}

}

void moverEntrenadorSinRestriccion(int* posicion, int pasos, int* cpuEjecutado) {

	if (pasos < 0) {
		for (int i = 0; i < abs(pasos); i++) {
			(*posicion) += 1;
			(*cpuEjecutado) += 1;
			sleep(datosConfigTeam->retardo_ciclo_cpu);
		}
	} else {
		for (int i = 0; i < abs(pasos); i++) {
			(*posicion) -= 1;
			(*cpuEjecutado) += 1;
			sleep(datosConfigTeam->retardo_ciclo_cpu);
		}
	}
}

void moverEntrenadorLimitado(int* posicion, int pasos, int* cantidadAEjecutar, double* estimador, int* cpuEjecutado) {

	int cantidadMovida = 0;

	while ((*cantidadAEjecutar) != 0 && cantidadMovida != abs(pasos)) {

		if (pasos < 0) {

			(*posicion) += 1;
			(*cpuEjecutado) += 1;
			sleep(datosConfigTeam->retardo_ciclo_cpu);

		} else {

			(*posicion) -= 1;
			(*cpuEjecutado) += 1;
			sleep(datosConfigTeam->retardo_ciclo_cpu);
		}

		(*cantidadAEjecutar) -= 1;
		cantidadMovida++;
		reducirElEstimadorDelEntrenador(estimador);
	}
}

void reducirElEstimadorDelEntrenador(double* estimador) {

	// Si el estimador esta EJEMPLO: e = 1.5, al restarle 1 quedaria 0.5 y si ejecutase
	// para no quedar en negativo si resto 1 a e = 0.5 lo dejo igual a 0

	if (string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-CD")) {

		if ((*estimador) >= 1) {
			(*estimador) -= 1;
		}
		else{
			(*estimador) = 0;
		}
	}
}

void moverEntrenador(t_entrenador* entrenador){

	int movimientosEn_x, movimientosEn_y;

	//Calculo Distancia Entrenador - Pokemon
	if(entrenador->pokemonCaptura != NULL){
	movimientosEn_x = (entrenador->posicion_x) - (entrenador->pokemonCaptura->posicion_x);
	movimientosEn_y = (entrenador->posicion_y) - (entrenador->pokemonCaptura->posicion_y);
	}

	//Calculo Distancia Entrenador - Entrenador
	else if(entrenador->entrenadorIntercambio){
	movimientosEn_x = (entrenador->posicion_x) - (entrenador->entrenadorIntercambio->posicion_x);
	movimientosEn_y = (entrenador->posicion_y) - (entrenador->entrenadorIntercambio->posicion_y);
	}

	if(seMueveDeFormaLimitada()){
		 moverEntrenadorLimitado(&entrenador->posicion_x, movimientosEn_x, &entrenador->quantumOtorgado, &entrenador->estimador, &entrenador->cantidadCPUEjecutado);
		 moverEntrenadorLimitado(&entrenador->posicion_y, movimientosEn_y, &entrenador->quantumOtorgado, &entrenador->estimador, &entrenador->cantidadCPUEjecutado);
	}

	else {
		moverEntrenadorSinRestriccion(&entrenador->posicion_x, movimientosEn_x, &entrenador->cantidadCPUEjecutado);
		moverEntrenadorSinRestriccion(&entrenador->posicion_y, movimientosEn_y, &entrenador->cantidadCPUEjecutado);
	}

	imprimirPosiciones(entrenador);
}

void imprimirPosiciones(t_entrenador* entrenador){

	if(entrenador->pokemonCaptura != NULL){
	log_info(logger, "Entrenador°%d: (PosX = %d ; PosY = %d) - %s: (PosX = %d ; PosY = %d)", entrenador->identificador, entrenador->posicion_x, entrenador->posicion_y, entrenador->pokemonCaptura->nombre,entrenador->pokemonCaptura->posicion_x, entrenador->pokemonCaptura->posicion_y);
	}
	else{
	log_info(logger, "Entrenador°%d: (PosX = %d ; PosY = %d) - Entrenador°%d: (PosX = %d ; PosY = %d)", entrenador->identificador, entrenador->posicion_x, entrenador->posicion_y, entrenador->entrenadorIntercambio->identificador, entrenador->entrenadorIntercambio->posicion_x, entrenador->entrenadorIntercambio->posicion_y);
	}
}

bool seMueveDeFormaLimitada(){
	return string_contains(datosConfigTeam->algoritmo_planificacion, "RR") ||
		   string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-CD");
}

void enviarMensajeSobreSentenciaGetPokemon() {

	for (int i = 0; i < list_size(objetivosGlobalesTeam); i++) {

		t_node_pokemon* pokemon = list_get(objetivosGlobalesTeam, i);
		pthread_t hiloEnvio;
		pthread_create(&hiloEnvio, NULL, (void*) enviarMensajeGetPokemon,
				pokemon->nombre_pokemon);
		pthread_detach(hiloEnvio);
	}
}

void enviarMensajeGetPokemon(void* pokemonParaGet){

	char* pokemon = (char*) pokemonParaGet;

	t_buffer* bufferCreado = serializar_sobre_nombre_pokemon(pokemon);

	if(enviar_paquete_GET(bufferCreado) == -1){
		log_error(logger, "No se pudo comunicar con el BROKER, se realizara el GET por DEFAULT");
		log_error(logger, "No existen Locaciones Sobre el Pokemon %s", pokemon);
	}
}

t_buffer* serializar_sobre_nombre_pokemon(char* nombrePokemon){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t largoNombrePokemon = strlen(nombrePokemon) + 1;

	buffer->size = sizeof(uint32_t) * 1
				 + largoNombrePokemon;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &largoNombrePokemon, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombrePokemon, largoNombrePokemon);

	buffer->stream = stream;

	return buffer;
}

int enviar_paquete_GET(t_buffer* buffer){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint8_t) broker_GP;
	paquete->buffer = buffer;

	void* envio = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(envio + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(envio + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(envio + offset, paquete->buffer->stream, paquete->buffer->size);

	int socketConexion;

	if((socketConexion = crear_conexion(datosConfigTeam->ip_broker, datosConfigTeam->puerto_broker)) == -1){
		free(envio);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);

		return socketConexion;
	}

	send(socketConexion, envio, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), MSG_NOSIGNAL);

	uint32_t idMensaje;
	recv(socketConexion, &idMensaje, sizeof(uint32_t), MSG_WAITALL);
	close(socketConexion);

	//TODO: Despues vendria crear la estructura del Mensaje "GET" junto con el "ID" que envia Broker
	// Y guardarlo para empezar a planificar una vez llegado la Respuesta por "LOCALIZED"

	free(envio);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return socketConexion;
}

// Devuelve la cantidad de pasos que tiene que hacer un entrenador para llegar a un pokemon
// Suma la distancia de x e y
int distanciaPokemonEntrenador(t_entrenador* entrenador, t_pokemon* pokemon) {
	return (abs(entrenador->posicion_x - pokemon->posicion_x)
			+ abs(entrenador->posicion_y - pokemon->posicion_y));
}

int distanciaEntreEntrenadores(t_entrenador* entrenador1) {

	return abs(
			(entrenador1->posicion_x
					- entrenador1->entrenadorIntercambio->posicion_x))
			+ abs(
					(entrenador1->posicion_y
							- entrenador1->entrenadorIntercambio->posicion_y));
}

bool loNecesitoGlobalmente(char* nombrePokemon){

	bool coincideElNombre(t_node_pokemon* pokemonObjetivo){
		return string_contains(pokemonObjetivo->nombre_pokemon, nombrePokemon);
	}

	return list_any_satisfy(objetivosGlobalesTeam, (void*) coincideElNombre);
}

/*
me fijo si necesito al pokemon, para ello comparo que el pokemon lo tenga menos veces en
la lista de localized que en la de objetivos globales.
*/
bool necesitoAgregarPokemon(char* pokemon){

	int cantidadDeVecesEnObjetivosGlobales = 0;

	bool coincideElNombre(t_node_pokemon* pokemonObjetivo){
		return string_contains(pokemonObjetivo->nombre_pokemon, pokemon);
	}
	t_node_pokemon* pokemonAEncontrar = list_find(objetivosGlobalesTeam, (void*) coincideElNombre);

	if(pokemonAEncontrar != NULL){
		cantidadDeVecesEnObjetivosGlobales = pokemonAEncontrar->cantidad_pokemon;
	}

	bool sonElMismoPokemon(char* pokemonObjetivo){
		return string_contains(pokemonObjetivo, pokemon);
	}

	int cantidadDeVecesEnLocalized = list_count_satisfying(pokemonesYaRecibidos, (void*) sonElMismoPokemon);

	return cantidadDeVecesEnLocalized <= cantidadDeVecesEnObjetivosGlobales;
}

int enviarMensajeParaCapturarPokemon(t_entrenador* entrenador){

	t_buffer* bufferCreado = serializar_msg_CATP(entrenador->pokemonCaptura);

	return enviar_paquete_CATCH(bufferCreado, entrenador);
}

void agregarPokemonComoObtenido(t_entrenador* entrenador, char* nombrePokemon){

	bool agregueComoNoNecesitado = false;

	bool coincideElNombre(t_node_pokemon* pokemonGuardado) {
		return string_contains(pokemonGuardado->nombre_pokemon, nombrePokemon);
	}

	t_node_pokemon* pokemon = list_find(entrenador->pokemon_objetivos, (void*) coincideElNombre);

	if(pokemon == NULL){

		t_pokemon* pokemonNuevo = malloc(sizeof(t_pokemon));
		pokemonNuevo->nombre = string_duplicate(nombrePokemon);
		pokemonNuevo->posicion_x = entrenador->pokemonCaptura->posicion_x;
		pokemonNuevo->posicion_y = entrenador->pokemonCaptura->posicion_y;
		list_add(entrenador->pokemon_no_necesitados, pokemonNuevo);
		agregueComoNoNecesitado = true;
	}

	t_node_pokemon* pokemonQueTengo = list_find(entrenador->pokemon_obtenidos, (void*) coincideElNombre);

	if((pokemonQueTengo == NULL) && (!agregueComoNoNecesitado)){
		t_node_pokemon* pokemonNuevo = malloc(sizeof(t_node_pokemon));
		pokemonNuevo->nombre_pokemon = string_duplicate(nombrePokemon);
		pokemonNuevo->cantidad_pokemon = 1;
		list_add(entrenador->pokemon_obtenidos, pokemonNuevo);
	}

	else if ((pokemonQueTengo != NULL) && (!agregueComoNoNecesitado)){
		pokemonQueTengo->cantidad_pokemon++;
	}

	if(entrenador->pokemonCaptura != NULL){
		free(entrenador->pokemonCaptura->nombre);
		free(entrenador->pokemonCaptura);
		entrenador->pokemonCaptura = NULL;
	}
}

void actualizarObjetivosDelEntrenador(t_list* listaObjetivos, char* pokemonAgregado, int idEntrenador){

	bool coincideElNombre(t_node_pokemon* pokemonGuardado){
		return string_contains(pokemonGuardado->nombre_pokemon, pokemonAgregado);
	}

	t_node_pokemon* pokemon = list_find(listaObjetivos, (void*) coincideElNombre);

	if(pokemon != NULL){

		pokemon->cantidad_pokemon--;

		if(pokemon->cantidad_pokemon == 0){
			t_node_pokemon* pokemonRemovido = list_remove_by_condition(listaObjetivos, (void*) coincideElNombre);
			free(pokemonRemovido->nombre_pokemon);
			free(pokemonRemovido);
		}

	}
}

void actualizarObjetivosGlobalesDelTeam(char* pokemonAtrapado) {

	pthread_mutex_lock(&soloActualizaUno);

	bool coincideElNombre(t_node_pokemon* pokemonGuardado) {
		return string_contains(pokemonGuardado->nombre_pokemon, pokemonAtrapado);
	}

	t_node_pokemon* pokemon = list_find(objetivosGlobalesTeam,
			(void*) coincideElNombre);

	pokemon->cantidad_pokemon--;

	if (pokemon->cantidad_pokemon == 0) {
		t_node_pokemon* pokemonRemovido = list_remove_by_condition(
				objetivosGlobalesTeam, (void*) coincideElNombre);
		eliminarLosPokemonsDeRespaldo(pokemonRemovido->nombre_pokemon);
		log_info(logger, "Se Capturaron Todos Los %s que necesitaba el Team",
				pokemonAtrapado);
		free(pokemonRemovido->nombre_pokemon);
		free(pokemonRemovido);
	}

	pthread_mutex_unlock(&soloActualizaUno);
}

void eliminarLosPokemonsDeRespaldo(char* pokemon){

	int i = 0;

	while(i < list_size(listaPokemonDeRespaldo)){

		t_pokemon* pokemonRespaldo = list_remove(listaPokemonDeRespaldo, i);

		if(string_contains(pokemon, pokemonRespaldo->nombre)){
		free(pokemonRespaldo->nombre);
		free(pokemonRespaldo);
		}

		else{
			list_add_in_index(listaPokemonDeRespaldo, i, pokemonRespaldo);
			i++;
		}
	}
}

bool tienenLaMismaEstimacion(double estimadorPrimero, double estimadorSegundo){
	return estimadorPrimero == estimadorSegundo;
}

void calcularNuevoEstimadorSiEsSJF(t_entrenador* entrenador){

	if(seEstaEjecutandoElSJF()){
		double nuevoEstimador = calcularEstimador(entrenador);
		entrenador->estimador = nuevoEstimador;
		entrenador->estimadorAnterior = nuevoEstimador;
	}
}

double calcularEstimador(t_entrenador* entrenador){

	double alpha = datosConfigTeam->alpha;

	return (entrenador->cantidadCPUEjecutado * alpha) +
		   ((1 - alpha) * entrenador->estimadorAnterior);
}

bool seEstaEjecutandoElSJF(){
	return string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-SD") ||
		   string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-CD");
}

t_pokemon* retornarEstructuraDePokemon(char* nombrePokemon, uint32_t posX, uint32_t posY){

	t_pokemon* nodePokemon = malloc(sizeof(t_pokemon));
	nodePokemon->nombre = string_duplicate(nombrePokemon);
	nodePokemon->posicion_x = posX;
	nodePokemon->posicion_y = posY;

	return nodePokemon;
}

void liberarMensajeSobreCatch(t_entrenador* entrenador) {
	free(entrenador->catchEnviado->pokemon);
	free(entrenador->catchEnviado);
}

// Funcion para Cumplir con la Metrica Nro°2 del TP
void validacionSobreCambioDeContextoDeEntrenador(t_entrenador* entrenadorAEjecutar){

	if(contextoEntrenador == NULL){
		contextoEntrenador = entrenadorAEjecutar;
	}

	else{

		if(contextoEntrenador->identificador != entrenadorAEjecutar->identificador){
			contadorCC++;
			contextoEntrenador = entrenadorAEjecutar;
		}
	}
}

void establecerMetricasAlFinalizarTeam(){

	for(int i=0; i < contadorEntrenadoresEnTeam; i++){
		sem_wait(&cantidadEntrenadoresEnTeam);
	}

	// Metricas Punto°1
	log_info(logger, "1. Cantidad de ciclos de CPU totales: %d", calcularCantidadTotalDeCiclosDeCPU());
	// Metricas Punto°2
	log_info(logger, "2. Cantidad de cambios de contexto realizados: %d", contadorCC);
    // Metricas Punto°3
	mostrarCPUEjecutadoPorEntrenador();
	// Metricas Punto°4 para Deadlock

	log_info(logger, "4. Deadlocks producidos y resueltos: %d", deadlockResueltos);

	// Libero de la memoria la estructura de los entrenadores
	queue_clean_and_destroy_elements(colaFinished, (void*) liberarEstructuraEntrenador);

	free(datosConfigTeam->objetivos_entrenadores);
	free(datosConfigTeam->pokemon_entrenadores);
	printf("TEAM Finalizado\n");
}

int calcularCantidadTotalDeCiclosDeCPU(){

	int i = 0;
	int cpuTotal = 0;

	while (i < contadorEntrenadoresEnTeam) {

		t_entrenador* entrenadorFinalizado = list_get(colaFinished->elements, i);
		cpuTotal += entrenadorFinalizado->totalCPUEjecutado;
		i++;
	}

	return cpuTotal;
}

void mostrarCPUEjecutadoPorEntrenador(){

	log_info(logger, "3. Cantidad de ciclos de CPU realizados por entrenador:");

	int i = 0;

	while (i < contadorEntrenadoresEnTeam) {

		t_entrenador* entrenadorFinalizado = list_get(colaFinished->elements,
				i);
		log_info(logger, "Entrenador Nro°%d - Cantidad CPU Total: %d",
				entrenadorFinalizado->identificador,
				entrenadorFinalizado->totalCPUEjecutado);
		i++;
	}
}

void liberarEstructuraEntrenador(t_entrenador* entrenadorFinalizado){

	entrenadorFinalizado->necesitaContinuar = false;
	sem_post(&entrenadorFinalizado->puedeEjecutar);

	list_clean_and_destroy_elements(entrenadorFinalizado->pokemon_obtenidos, (void*) liberarPokemonObjetivos);
	list_destroy(entrenadorFinalizado->pokemon_obtenidos);
	list_destroy(entrenadorFinalizado->pokemon_objetivos);
	list_destroy(entrenadorFinalizado->pokemon_no_necesitados);
	sem_destroy(&entrenadorFinalizado->puedeEjecutar);
	free(entrenadorFinalizado);
}

void liberarPokemonObjetivos(t_node_pokemon* pokemonLiberado){
	free(pokemonLiberado->nombre_pokemon);
	free(pokemonLiberado);
}
