#include "planificador.h"
#include "libTeam.h"

void crearHiloSobreAlgoritmoDePlanificacion(){

	pthread_t hiloAlgortimoPlanificador;

	if(string_contains(datosConfigTeam->algoritmo_planificacion, "FIFO")){
		pthread_create(&hiloAlgortimoPlanificador, NULL, (void*) &planificarPorFifo, NULL);
	}

	else if(string_contains(datosConfigTeam->algoritmo_planificacion, "RR")){
		pthread_create(&hiloAlgortimoPlanificador, NULL, (void*) &planificarPorRR, NULL);
	}

	else if(string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-SD")){
		pthread_create(&hiloAlgortimoPlanificador, NULL, (void*) &planificarPorSJFSinDesalojo, NULL);
	}

	else if(string_contains(datosConfigTeam->algoritmo_planificacion, "SJF-CD")){
		pthread_create(&hiloAlgortimoPlanificador, NULL, (void*) &planificarPorSRT, NULL);
	}

	pthread_detach(hiloAlgortimoPlanificador);
}

void planificarEntrenador(void* entrenadorPlanificable) {

	t_entrenador* entrenador = (t_entrenador*) entrenadorPlanificable;

	while (1) {

		sem_wait(&entrenador->puedeEjecutar);

		if(!entrenador->necesitaContinuar){
			break;
		}

		moverEntrenador(entrenador);

		t_entrenador* entrenadorEjecutado;

		// Esta Logica se asocia con el Pokemon
		if (entrenador->pokemonCaptura != NULL) {
			if (!seEncuentraEnLaMismaPosicion(entrenador)) {
				entrenadorEjecutado = queue_pop(colaExecute);
				entrenadorEjecutado->estado = READY;
				encolarEntrenador(colaReady, entrenadorEjecutado, m_colaReady);
				sem_post(&puedeEncolarEntrenador);
				log_info(logger, "Se Paso Al Entrenador N°%d A La Cola READY por tener Quantum: %d",
						entrenadorEjecutado->identificador, entrenadorEjecutado->quantumOtorgado);
				sem_post(&cantidadEntrenadoresEnReady);
			}

			if (seEncuentraEnLaMismaPosicion(entrenador)) {

				entrenadorEjecutado = queue_pop(colaExecute);

				// Esta validacion es para tomar en cuenta si queda Quantum para poder ENVIAR el CATCH
				// De poder, se ejecuta normalmente, de no ser asi se lo pasa a READY
				if (seMueveDeFormaLimitada()
						&& entrenador->quantumOtorgado == 0) {
					entrenadorEjecutado->estado = READY;
					encolarEntrenador(colaReady, entrenadorEjecutado,
							m_colaReady);
					sem_post(&puedeEncolarEntrenador);
					log_info(logger,
							"Se Paso Al Entrenador N°%d A La Cola READY por tener Quantum: %d",
							entrenadorEjecutado->identificador,
							entrenadorEjecutado->quantumOtorgado);
					sem_post(&cantidadEntrenadoresEnReady);
				}

				else {
					sem_post(&puedeEncolarEntrenador);
					log_info(logger,
							"Voy a realizar el CATCH para el Pokemon: %s en la Posicion (X=%d - Y=%d)",
							entrenador->pokemonCaptura->nombre,
							entrenador->pokemonCaptura->posicion_x,
							entrenador->pokemonCaptura->posicion_y);

					int resultado = enviarMensajeParaCapturarPokemon(
							entrenadorEjecutado);

					// Incremento el CPU por realizar el envio del mensaje
					entrenador->cantidadCPUEjecutado++;

					entrenadorEjecutado->estado = BLOCKED;
					encolarEntrenador(colaBlocked, entrenadorEjecutado,
							m_colaBlocked);
					log_info(logger,
							"Se Paso Al Entrenador N°%d A La Cola BLOCKED a la espera del resultado de CATCH",
							entrenadorEjecutado->identificador);

					if (resultado == -1) {
						calcularNuevoEstimadorSiEsSJF(entrenadorEjecutado);
						entrenadorEjecutado->totalCPUEjecutado += entrenadorEjecutado->cantidadCPUEjecutado;
						entrenadorEjecutado->cantidadCPUEjecutado = 0;
						iterarAccionesAlCapturarConExito(entrenadorEjecutado);
					}
				}
			}
		}

		// Esta Logica se asocia con el Entrenador
		else {

			if (!estaParaIntercambiar(entrenador)) {
				entrenadorEjecutado = queue_pop(colaExecute);
				entrenadorEjecutado->estado = READY;
				sem_post(&puedeEncolarEntrenador);
				encolarEntrenador(colaReady, entrenadorEjecutado, m_colaReady);
				log_info(logger, "Se Paso Al Entrenador N°%d A La Cola READY por tener Quantum: %d",
						entrenadorEjecutado->identificador, entrenadorEjecutado->quantumOtorgado);
				sem_post(&cantidadEntrenadoresEnReady);
			}

			if (estaParaIntercambiar(entrenador)) {
				iniciarIntercambioEntreEntrenadores(entrenador);
			}
		}
	}
}

void planificadorSeleccionEntrenador(){

	while(1){

		sem_wait(&pokemonsACapturar);
		// La idea de este semaforo es que se despierte cuando llega un LOCALIZED/APPEARED_POKEMON
		// Para planificar al mas cercano
		sem_wait(&puedeSeleccionarEntrenador);
		// Este SEMAFORO esta para que en base a la cantidad de ENTRENADORES disponibles este HILO trabaje

		t_pokemon* pokemon;
		t_entrenador* entrenadorCercano;

		// Situacion cuando es 1 Pokemon - N Entrenadores
		if((list_size(listaPokemonRequeridos) == 1) && (cantidadEntrenadoresDisponibles() >= 1)){
		pokemon = list_remove(listaPokemonRequeridos, 0);
		entrenadorCercano = despertarAlEntrenadorMasCercano(pokemon);
		}

		// Situacion cuando es M Pokemones - 1 Entrenador
		else{
			entrenadorCercano = list_find(colaBlocked->elements, (void*) seEncuentraOcupado);
			pokemon = asignarHaciaElMasCercano(entrenadorCercano);
		}


		// Lo desencolo de NEW y lo paso a READY
		t_entrenador* entrenadorADesencolar = desencolarEntrenadorSobreCola(entrenadorCercano);
		entrenadorADesencolar->estado = READY;
		entrenadorADesencolar->pokemonCaptura = pokemon;
		encolarEntrenador(colaReady, entrenadorCercano, m_colaReady);

		log_info(logger, "Se Paso El Entrenador N°%d a READY Para Capturar", entrenadorADesencolar->identificador);
		sem_post(&cantidadEntrenadoresEnReady);
	}
}

int cantidadEntrenadoresDisponibles(){

	int cantidadEnNew = list_count_satisfying(colaNew->elements,(void*) seEncuentraOcupado);
	int cantidadEnBlocked = list_count_satisfying(colaBlocked->elements,(void*) seEncuentraOcupado);

	return cantidadEnNew + cantidadEnBlocked;
}

t_pokemon* asignarHaciaElMasCercano(t_entrenador* entrenador) {

	int i = 0;
	t_pokemon* pokemonCercano = NULL;

	while (i < list_size(listaPokemonRequeridos)) {

		t_pokemon* pokemon = list_get(listaPokemonRequeridos, i);

		if (pokemonCercano == NULL) {
			pokemonCercano = pokemon;
		}

		else {

			elegirPokemonMasCercano(&pokemonCercano, entrenador, pokemon);

		}

		i++;
	}

	bool tienenLaMismaInfo(t_pokemon* pokemonCaptura){
		return string_contains(pokemonCaptura->nombre, pokemonCercano->nombre) &&
			   pokemonCaptura->posicion_x == pokemonCercano->posicion_x &&
			   pokemonCaptura->posicion_y == pokemonCercano->posicion_y;
	}

	return list_remove_by_condition(listaPokemonRequeridos, (void*) tienenLaMismaInfo);
}

t_entrenador* desencolarEntrenadorSobreCola(t_entrenador* entrenador){

	if(entrenador->estado == NEW){
		return desencolarEntrenadorCercano(colaNew, entrenador, m_colaNew);
	}

	else{
		return desencolarEntrenadorCercano(colaBlocked, entrenador, m_colaBlocked);
	}
}

void encolarEntrenador(t_queue* cola, t_entrenador* entrenador, pthread_mutex_t mutex){

	pthread_mutex_lock(&mutex);
	queue_push(cola, entrenador);
	pthread_mutex_unlock(&mutex);
}

t_entrenador* desencolarEntrenadorCercano(t_queue* cola, t_entrenador* entrenador, pthread_mutex_t mutex){

		bool coincideElIdEntrenador(void* elemento){
			t_entrenador*  entrenadorABuscar = (t_entrenador*) elemento;
			return entrenador->identificador ==  entrenadorABuscar->identificador;
		}

		pthread_mutex_lock(&mutex);
		t_entrenador* entrenadorEncontrado = list_remove_by_condition(cola->elements, coincideElIdEntrenador);
		pthread_mutex_unlock(&mutex);

		return entrenadorEncontrado;
}

// Esta funcion es para quien asignarle la mision de Capturar al POKEMON que aparecio en el MAPA
t_entrenador* despertarAlEntrenadorMasCercano(t_pokemon* pokemon) {

	t_entrenador* entrenadorMasCercano = NULL;

	if (queue_size(colaNew) != 0) {

		// Filtro para cada entrenador que no esta ocupado y puede seguir capturando POKEMONES
		t_list* listaEntrenadoresDisponiblesNew = list_filter(colaNew->elements,
				(void*) seEncuentraOcupado);
		establecerEntrenadorAlPokemon(listaEntrenadoresDisponiblesNew,
				&entrenadorMasCercano, pokemon);

		list_destroy(listaEntrenadoresDisponiblesNew);
	}

	if (queue_size(colaBlocked) != 0) {
		t_list* listaEntrenadoresDisponiblesBlocked = list_filter(
				colaBlocked->elements, (void*) seEncuentraOcupado);
		establecerEntrenadorAlPokemon(listaEntrenadoresDisponiblesBlocked,
				&entrenadorMasCercano, pokemon);

		list_destroy(listaEntrenadoresDisponiblesBlocked);
	}

	return entrenadorMasCercano;
}

bool seEncuentraOcupado(t_entrenador* entrenador) {
	return (!entrenador->estaOcupado) && (entrenador->cantidad_maxima_a_capturar != 0);
}

// Funcion que se encarga de establecer que ENTRENADOR es el más cercano
void establecerEntrenadorAlPokemon(t_list* listaEntrenadores,
		t_entrenador** entrenadorCercano, t_pokemon* pokemon) {

	int i = 0;
	while (i < list_size(listaEntrenadores)) {

		t_entrenador* entrenador = list_get(listaEntrenadores, i);

		if (*entrenadorCercano == NULL) {
			*entrenadorCercano = entrenador;
		}

		else {

			*entrenadorCercano = seleccionarQuienEstaMasCerca(entrenadorCercano,
					entrenador, pokemon);

		}

		i++;
	}

}

// Funcion para comparar la distancia entre 2 Entrenadores y devuelvo quien está más cercano
t_entrenador* seleccionarQuienEstaMasCerca(t_entrenador** entrenador1,
		t_entrenador* entrenador2, t_pokemon* pokemon) {

	int distanciaPrimerEntrenador = distanciaPokemonEntrenador(*entrenador1,
			pokemon);
	int distanciaSegundoEntrenador = distanciaPokemonEntrenador(entrenador2,
			pokemon);

	if (distanciaPrimerEntrenador < distanciaSegundoEntrenador) {
		return *entrenador1;
	}

	else if (distanciaSegundoEntrenador < distanciaPrimerEntrenador) {
		return entrenador2;
	}

	//Este caso es por si tienen la misma distancia, le asigno al primero que se selecciono primero
	else {
		return *entrenador1;
	}

}

void elegirPokemonMasCercano(t_pokemon** pokemonCercano, t_entrenador* entrenador, t_pokemon* pokemon){

	int distanciaPrimerPokemon = distanciaPokemonEntrenador(entrenador, *pokemonCercano);
	int distanciaSegundoPokemon = distanciaPokemonEntrenador(entrenador, pokemon);

	if(distanciaSegundoPokemon < distanciaPrimerPokemon){
		*pokemonCercano = pokemon;
	}
}

t_entrenador* elDeMenorQuantum() {
	int iteraciones = queue_size(colaReady);
	//agarro el primer entrenador de la cola de ready
	t_entrenador* entrenadorPlanificable = list_get(colaReady->elements, 0);
	for (int i = 1; i < iteraciones; i++) {
		//agarro el segundo entrenador de la cola de ready
		t_entrenador* entrenadorPosible = list_get(colaReady->elements, i);

		// Si tienen el mismo valor de Estimacion, se compara por quien está más cerca de su captura sobre el Pokemon
		if (tienenLaMismaEstimacion(entrenadorPlanificable->estimador,
				entrenadorPosible->estimador)) {

			int distanciaEntrenador1;
			int distanciaEntrenador2;

			if (entrenadorPlanificable->pokemonCaptura != NULL
					&& entrenadorPosible->pokemonCaptura != NULL) {

				distanciaEntrenador1 = distanciaPokemonEntrenador(
						entrenadorPlanificable,
						entrenadorPlanificable->pokemonCaptura);
				distanciaEntrenador2 = distanciaPokemonEntrenador(
						entrenadorPosible, entrenadorPosible->pokemonCaptura);
			}

			else if (entrenadorPlanificable->entrenadorIntercambio != NULL
					&& entrenadorPosible->entrenadorIntercambio != NULL) {

				distanciaEntrenador1 = distanciaEntreEntrenadores(
						entrenadorPlanificable);
				distanciaEntrenador2 = distanciaEntreEntrenadores(
						entrenadorPosible);

			}

			if (esMayorQue((double) distanciaEntrenador1,
					(double) distanciaEntrenador2)) {
				entrenadorPlanificable = entrenadorPosible;
			}

		}

		// De no ser sus estimaciones IGUALES, veo quien estaría más cerca de su Captura
		else {

			if (esMayorQue(entrenadorPlanificable->estimador,
					entrenadorPosible->estimador)) {
				entrenadorPlanificable = entrenadorPosible;
			}

		}

	}

	return desencolarEntrenadorCercano(colaReady, entrenadorPlanificable,
			m_colaReady);
}

bool esMayorQue(double primerValor, double segundoValor){
	return primerValor > segundoValor;
}

bool seEncuentraEnLaMismaPosicion(t_entrenador* entrenador){

	return entrenador->posicion_x == entrenador->pokemonCaptura->posicion_x &&
		   entrenador->posicion_y == entrenador->pokemonCaptura->posicion_y;
}

bool estaParaIntercambiar(t_entrenador* entrenador){

	return entrenador->posicion_x == entrenador->entrenadorIntercambio->posicion_x &&
		   entrenador->posicion_y == entrenador->entrenadorIntercambio->posicion_y;
}

void planificarPorFifo(){

	while(1){

	sem_wait(&cantidadEntrenadoresEnReady);
	sem_wait(&puedeEncolarEntrenador);
	t_entrenador* entrenadorAPlanificar = queue_pop(colaReady);
	validacionSobreCambioDeContextoDeEntrenador(entrenadorAPlanificar);
	entrenadorAPlanificar->contadorIntercambio = 0;
	entrenadorAPlanificar->estado = EXECUTE;
	encolarEntrenador(colaExecute,entrenadorAPlanificar,m_colaExecute);
	log_info(logger, "Se Paso el Entrenador N°%d a EXECUTE para Ejecutar", entrenadorAPlanificar->identificador);
	sem_post(&entrenadorAPlanificar->puedeEjecutar);
	}
}

void planificarPorRR(){

	while(1){
		sem_wait(&cantidadEntrenadoresEnReady);
		sem_wait(&puedeEncolarEntrenador);
		t_entrenador* entrenadorAPlanificar =  queue_pop(colaReady);
		validacionSobreCambioDeContextoDeEntrenador(entrenadorAPlanificar);
		entrenadorAPlanificar->quantumOtorgado = datosConfigTeam->quantum;
		if (entrenadorAPlanificar->entrenadorIntercambio != NULL
				&& entrenadorAPlanificar->contadorIntercambio == 0) {
			entrenadorAPlanificar->contadorIntercambio = 5;
		}
		entrenadorAPlanificar->estado = EXECUTE;
		encolarEntrenador(colaExecute,entrenadorAPlanificar,m_colaExecute);
		log_info(logger, "Se Paso el Entrenador N°%d a EXECUTE para Ejecutar", entrenadorAPlanificar->identificador);
		sem_post(&entrenadorAPlanificar->puedeEjecutar);
	}
}

void planificarPorSJFSinDesalojo() {

	while (1) {

		sem_wait(&cantidadEntrenadoresEnReady);
		sem_wait(&puedeEncolarEntrenador);
		t_entrenador* entrenadorDesencolado = elDeMenorQuantum();
		validacionSobreCambioDeContextoDeEntrenador(entrenadorDesencolado);
		entrenadorDesencolado->contadorIntercambio = 0;
		entrenadorDesencolado->estado = EXECUTE;

		encolarEntrenador(colaExecute, entrenadorDesencolado, m_colaExecute);
		log_info(logger, "Se Paso el Entrenador N°%d a EXECUTE para Ejecutar", entrenadorDesencolado->identificador);
		sem_post(&entrenadorDesencolado->puedeEjecutar);
	}
}

void planificarPorSRT() {

	while (1) {

		sem_wait(&cantidadEntrenadoresEnReady);
		sem_wait(&puedeEncolarEntrenador);

		t_entrenador* entrenadorDesencolado = elDeMenorQuantum();
		validacionSobreCambioDeContextoDeEntrenador(entrenadorDesencolado);
		if (entrenadorDesencolado->entrenadorIntercambio != NULL
				&& entrenadorDesencolado->contadorIntercambio == 0) {
			entrenadorDesencolado->contadorIntercambio = 5;
		}
		entrenadorDesencolado->estado = EXECUTE;
		// Le doy 1 de Quantum para simular la idea de que Ejecute cada 1 CPU
		entrenadorDesencolado->quantumOtorgado = 1;

		encolarEntrenador(colaExecute, entrenadorDesencolado, m_colaExecute);
		log_info(logger, "Se Paso el Entrenador N°%d a EXECUTE para Ejecutar", entrenadorDesencolado->identificador);
		sem_post(&entrenadorDesencolado->puedeEjecutar);
	}
}

void algoritmoDeteccionDeadlock() {

	while (1) {

		// Este semaforo es cuando un Entrenador atrape un Pokemon que NO NECESITA
		sem_wait(&entrenadoresEnDeadlock);
		sem_wait(&entrenadoresEnDeadlock);
		//TODO: Posiblemente un Semaforo Mas
		if(puede){
		sem_wait(&puedeIniciarAlgoritmo);
		puede = false;
		}
		log_info(logger, "Iniciando Algoritmo de Deteccion de Deadlock");
		// Ejecucion de Busqueda Condicion Para Intercambio

		bool tienenPokemonQueNoNecesitan(t_entrenador* entrenador) {
			return (list_size(entrenador->pokemon_no_necesitados) != 0) && (!entrenador->estaOcupado);
		}

		t_list* entrenadoresParaIntercambiar = list_filter(
				colaBlocked->elements, (void*) tienenPokemonQueNoNecesitan);

		int i = 0;
		bool cumpleCondicion = false;

		while (i < list_size(entrenadoresParaIntercambiar)) {

			int j = i + 1;

			t_entrenador* entrenador1 = list_get(entrenadoresParaIntercambiar, i);

			while (j < list_size(entrenadoresParaIntercambiar)) {

				t_entrenador* entrenador2 = list_get(entrenadoresParaIntercambiar, j);

				bool necesita1 = necesitaUnPokemonDelOtro(entrenador1,
						entrenador2);
				bool necesita2 = necesitaUnPokemonDelOtro(entrenador2,
						entrenador1);

				if ((necesita1 && necesita2) || (necesita1 && !necesita2)) {
					asignarYMoverEntrenadorAIntercambiar(entrenador1,
							entrenador2);
					cumpleCondicion = true;
					break;
				} else if (!necesita1 && necesita2) {
					asignarYMoverEntrenadorAIntercambiar(entrenador2,
							entrenador1);
					cumpleCondicion = true;
					break;
				}

				j++;
			}

			// Para salir del While Principal
			if (cumpleCondicion) {
				break;
			}

			i++;
		}
			list_destroy(entrenadoresParaIntercambiar);
	}
}

// Funcion para buscar si Entrenador A Necesita un Pokemon del Entrenador B
bool necesitaUnPokemonDelOtro(t_entrenador* entrenador1, t_entrenador* entrenador2){

	t_list* pokemonesNoNecesitados = entrenador2->pokemon_no_necesitados;

	int i=0;

	while(i < list_size(pokemonesNoNecesitados)){

		t_pokemon* pokemonNoNecesario = list_get(pokemonesNoNecesitados, i);

		bool loRequiero(t_node_pokemon* pokemonRequerido){
			return string_contains(pokemonRequerido->nombre_pokemon, pokemonNoNecesario->nombre);
		}

		if(list_any_satisfy(entrenador1->pokemon_objetivos, (void*) loRequiero)){
			entrenador1->pokemonACambiar = string_duplicate(pokemonNoNecesario->nombre);
			return true;
		}

		i++;
	}
	return false;
}

void asignarYMoverEntrenadorAIntercambiar(t_entrenador* entrenadorQueSeMueve, t_entrenador* entrenadorQueEspera){

	t_entrenador_intercambio* entrenador_a_intercambiar = malloc(sizeof(t_entrenador_intercambio));
	entrenador_a_intercambiar->identificador = entrenadorQueEspera->identificador;
	entrenador_a_intercambiar->posicion_x = entrenadorQueEspera->posicion_x;
	entrenador_a_intercambiar->posicion_y = entrenadorQueEspera->posicion_y;

	entrenadorQueSeMueve->entrenadorIntercambio = entrenador_a_intercambiar;
	entrenadorQueSeMueve->estaOcupado = true;
	entrenadorQueEspera->estaOcupado = true;
	t_entrenador* entrenadorDesencolado = desencolarEntrenadorCercano(colaBlocked, entrenadorQueSeMueve, m_colaBlocked);
	entrenadorDesencolado->estado = READY;
	encolarEntrenador(colaReady, entrenadorDesencolado, m_colaReady);

	log_info(logger, "Se paso al Entrenador N°%d a READY para Intercambiar con Entrenador N°%d", entrenadorQueSeMueve->identificador, entrenadorQueEspera->identificador);
	sem_post(&cantidadEntrenadoresEnReady);
}

void iniciarIntercambioEntreEntrenadores(t_entrenador* entrenador) {

	if(seMueveDeFormaLimitada()){
		intercambiarConRestriccion(entrenador);
	}

	else {
		intercambiarSinRestriccion(entrenador);
	}

	// Finaliza de Ejecutar los Ciclos de CPU para el Intercambio
	if (entrenador->contadorIntercambio == 0) {
		log_info(logger, "Se Inicia el Intercambio entre Entrenador N°%d - Entrenador N°%d", entrenador->identificador, entrenador->entrenadorIntercambio->identificador);
		// LOG_PROPIO
		log_info(logger, "El CPU Total Ejecutado Fue %d", entrenador->cantidadCPUEjecutado);
		entrenador->totalCPUEjecutado += entrenador->cantidadCPUEjecutado;
		entrenador->cantidadCPUEjecutado = 0;
		ejecutarIntercambioDePokemones(entrenador);
	}

	else {

		t_entrenador* entrenadorEjecutado = queue_pop(colaExecute);
		entrenadorEjecutado->estado = READY;
		encolarEntrenador(colaReady, entrenadorEjecutado, m_colaReady);
		sem_post(&puedeEncolarEntrenador);
		log_info(logger,
				"Se Paso Al Entrenador N°%d A La Cola READY Por Tener Quantum = %d",
				entrenadorEjecutado->identificador,
				entrenadorEjecutado->quantumOtorgado);
		sem_post(&cantidadEntrenadoresEnReady);
	}

}

void intercambiarSinRestriccion(t_entrenador* entrenador) {

	// Caso Sin Limites FIFO - (SJF-SD)
	for (int i = 0; i < 5; i++) {
		entrenador->cantidadCPUEjecutado++;
		sleep(datosConfigTeam->retardo_ciclo_cpu);
	}
}

void intercambiarConRestriccion(t_entrenador* entrenador) {

	// Caso Limitado RR - (SJF-CD)
	while ((entrenador->contadorIntercambio != 0)
			&& (entrenador->quantumOtorgado != 0)) {
		entrenador->contadorIntercambio--;
		entrenador->quantumOtorgado--;
		entrenador->cantidadCPUEjecutado++;
		reducirElEstimadorDelEntrenador(&(entrenador->estimador));
		sleep(datosConfigTeam->retardo_ciclo_cpu);
	}
}

void ejecutarIntercambioDePokemones(t_entrenador* entrenador){

		bool tieneElMismoID(t_entrenador* entrenadorBusqueda){
			return entrenadorBusqueda->identificador == entrenador->entrenadorIntercambio->identificador;
		}

		t_entrenador* entrenadorAIntercambiar = list_find(colaBlocked->elements, (void*) tieneElMismoID);

		// Libero la estructura para identificar al Entrenador con el cual INTERCAMBIO
		free(entrenador->entrenadorIntercambio);

		bool esElQueBuscaba(t_pokemon* pokemon){
			return string_contains(pokemon->nombre, entrenador->pokemonACambiar);
		}

		// Parte que Actualiza al Entrenador que se movio para Intercambiar
		// Sé que intercambié entonces lo elimino del otro entrenador
		t_pokemon* pokemonRemovido = list_remove_by_condition(entrenadorAIntercambiar->pokemon_no_necesitados, (void*) esElQueBuscaba);
		free(pokemonRemovido->nombre);
		free(pokemonRemovido);

		agregarPokemonComoObtenido(entrenador, entrenador->pokemonACambiar);
		actualizarObjetivosDelEntrenador(entrenador->pokemon_objetivos, entrenador->pokemonACambiar, entrenador->identificador);

		//-- Hasta Acá --//

		bool esElQueBuscabaElOtro(t_pokemon* pokemon){

			return string_contains(pokemon->nombre, entrenadorAIntercambiar->pokemonACambiar);
		}

		// Parte que Actualiza al Entrenador que se está Bloqueado para Intercambiar
		if(entrenadorAIntercambiar->pokemonACambiar == NULL){
			t_pokemon* pokemonCambio = list_get(entrenador->pokemon_no_necesitados, 0);

			// Copia del Pokemon No Necesitado
			t_pokemon* pokemonQueNoQuiero = malloc(sizeof(t_pokemon));
			pokemonQueNoQuiero->nombre = string_duplicate(pokemonCambio->nombre);
			pokemonQueNoQuiero->posicion_x = pokemonCambio->posicion_x;
			pokemonQueNoQuiero->posicion_y = pokemonCambio->posicion_y;
			entrenadorAIntercambiar->pokemonCaptura = pokemonQueNoQuiero;

			entrenadorAIntercambiar->pokemonACambiar = string_duplicate(pokemonCambio->nombre);
		}

		t_pokemon* pokemonRemovido2 = list_remove_by_condition(entrenador->pokemon_no_necesitados, (void*) esElQueBuscabaElOtro);
		free(pokemonRemovido2->nombre);
		free(pokemonRemovido2);

		agregarPokemonComoObtenido(entrenadorAIntercambiar, entrenadorAIntercambiar->pokemonACambiar);
		actualizarObjetivosDelEntrenador(entrenadorAIntercambiar->pokemon_objetivos, entrenadorAIntercambiar->pokemonACambiar, entrenadorAIntercambiar->identificador);

		// Libero esto Cuando el entrenador que se le intercambia no necesita ese pokemon que se le entrega
		if(entrenadorAIntercambiar->pokemonCaptura != NULL){
			free(entrenadorAIntercambiar->pokemonCaptura->nombre);
			free(entrenadorAIntercambiar->pokemonCaptura);
			entrenadorAIntercambiar->pokemonCaptura = NULL;
		}

		log_info(logger,
			"Se Intercambio del Entrenador N°%d Pokemon: %s con el Entrenador N°%d Pokemon: %s",
			entrenador->identificador, entrenador->pokemonACambiar,
			entrenadorAIntercambiar->identificador,
			entrenadorAIntercambiar->pokemonACambiar);

		free(entrenador->pokemonACambiar);
		free(entrenadorAIntercambiar->pokemonACambiar);
		entrenadorAIntercambiar->pokemonACambiar = NULL;
		//-- Hasta Acá --//

		// A partir de aca verifico si ambos tienen todos los pokemones que necesitaban
		transicionarEntrenadorAlFinalizarIntercambio(entrenador, "EXECUTE");
		transicionarEntrenadorAlFinalizarIntercambio(entrenadorAIntercambiar, "BLOCKED");

		deadlockResueltos++;
}

void transicionarEntrenadorAlFinalizarIntercambio(t_entrenador* entrenador, char* tipoCola) {

	t_entrenador* entrenadorDesencolado;

	if (string_contains(tipoCola, "EXECUTE")) {

		if (list_size(entrenador->pokemon_objetivos) == 0) {
			entrenador->objetivosCumplidos = true;
			entrenadorDesencolado = desencolarEntrenadorCercano(colaExecute,
					entrenador, m_colaExecute);
			sem_post(&puedeEncolarEntrenador);
			entrenadorDesencolado->estado = FINISHED;
			entrenadorDesencolado->estaOcupado = false;
			encolarEntrenador(colaFinished, entrenadorDesencolado,
					m_colaFinished);
			log_info(logger, "Se Paso Al Entrenador N°%d A La Cola FINISHED habiendo FINALIZADO sus Objetivos",
					entrenadorDesencolado->identificador);
			sem_post(&cantidadEntrenadoresEnTeam);
		}

		else {

			entrenadorDesencolado = desencolarEntrenadorCercano(colaExecute,
					entrenador, m_colaExecute);
			sem_post(&puedeEncolarEntrenador);
			calcularNuevoEstimadorSiEsSJF(entrenadorDesencolado);
			entrenadorDesencolado->estado = BLOCKED;
			entrenadorDesencolado->estaOcupado = false;
			encolarEntrenador(colaBlocked, entrenadorDesencolado,
					m_colaBlocked);
			log_info(logger, "Se Paso Al Entrenador N°%d A La Cola BLOCKED al haber Intercambiado",
					entrenadorDesencolado->identificador);
			sem_post(&entrenadoresEnDeadlock);
		}
	}

	else {

		if (list_size(entrenador->pokemon_objetivos) == 0) {
			entrenador->objetivosCumplidos = true;
			entrenadorDesencolado = desencolarEntrenadorCercano(colaBlocked,
					entrenador, m_colaBlocked);
			entrenadorDesencolado->estado = FINISHED;
			entrenadorDesencolado->estaOcupado = false;
			encolarEntrenador(colaFinished, entrenadorDesencolado,
					m_colaFinished);
			log_info(logger, "Se Paso Al Entrenador N°%d A La Cola FINISHED habiendo FINALIZADO sus Objetivos",
					entrenadorDesencolado->identificador);
			sem_post(&cantidadEntrenadoresEnTeam);
		}

		else {
			entrenador->estaOcupado = false;
			sem_post(&entrenadoresEnDeadlock);
		}

	}
}
