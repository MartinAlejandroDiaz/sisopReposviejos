/*
 * memoria.c
 *
 *  Created on: 14 may. 2020
 *      Author: utnso
 */

#include "memoria.h"
#include "libBroker.h"


void inicializarMemoria() {
	CACHE = malloc(datosConfigBroker->tamano_memoria);
	memset(CACHE, '_', datosConfigBroker->tamano_memoria);
	//cache_imprimir1();
	MENSAJES = list_create();
	PARTICIONES_LIBRES = list_create();

	list_add(PARTICIONES_LIBRES, newParticionLibre(CACHE, datosConfigBroker->tamano_memoria));
}

void eliminarParticionFIFO (){
	t_msg * primer_mensaje = list_remove(MENSAJES,0);

	if (string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
		consolidar_buddy_system(primer_mensaje);
	}else{
		consolidar_particiones_dinamicas(primer_mensaje);
	}

	log_info(logger, "Se elimino un mensaje mediante el algoritmo de reemplazo FIFO, posicion: %d",primer_mensaje->mensaje-CACHE);

	list_iterate(primer_mensaje->suscriptores, (void*) free);
	list_destroy(primer_mensaje->suscriptores);
	free(primer_mensaje);
}

void eliminarParticionLRU (){
	bool menorTimeStamp(t_msg * msg1,t_msg * msg2){
		return (msg1->timestamp) < (msg2->timestamp);
	}

	list_sort(MENSAJES, (void*) menorTimeStamp);
	t_msg * primer_mensaje = list_remove(MENSAJES,0);

	if (string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
		consolidar_buddy_system(primer_mensaje);
	}else{
		consolidar_particiones_dinamicas(primer_mensaje);
	}

	log_info(logger, "Se elimino un mensaje mediante el algoritmo de reemplazo LRU, posicion: %d",primer_mensaje->mensaje-CACHE);

	list_iterate(primer_mensaje->suscriptores, (void*) free);
	list_destroy(primer_mensaje->suscriptores);
	free(primer_mensaje);
}


void consolidar_particiones_dinamicas (t_msg* msg){

	//primero tengo que verificar si tengo particiones libres a los lados

	bool IgualDelante (t_particion_libre* particion){
		return particion->ptr_cache==(msg->mensaje + msg->mensaje_length);
	}

	bool IgualDetras (t_particion_libre* particion){
			return particion->ptr_cache==(msg->mensaje - particion->tamanio);
	}

	t_particion_libre* particionDerecha = list_find(PARTICIONES_LIBRES,(void*) IgualDelante);
	t_particion_libre* particionIzquierda = list_find(PARTICIONES_LIBRES,(void*) IgualDetras);


// si tengo a los lados particiones libres, muevo sus punteros y ajusto tamanio

	if (particionDerecha != NULL & particionIzquierda == NULL){ //si tengo una particion libre adelante
		particionDerecha->ptr_cache -= msg->mensaje_length;
		particionDerecha->tamanio += msg->mensaje_length;
		log_debug(logger, "El msg tiene una libre particion adelante");

	}else if (particionDerecha == NULL & particionIzquierda != NULL){ //si tengo una particion libre detras
		particionIzquierda->tamanio += msg->mensaje_length;
		log_debug(logger, "El msg tiene una libre particion atras");

	}else if (particionDerecha != NULL & particionIzquierda != NULL){ // si tengo ambas particiones libres a los lados
		particionIzquierda->tamanio += msg->mensaje_length + particionDerecha->tamanio;
		list_remove_by_condition(PARTICIONES_LIBRES, (void*) IgualDelante);
		free(particionDerecha);
		log_debug(logger, "El msg tiene una particion libre delante y atras");

	}else{ // no tengo particiones libres a los lados
		list_add(PARTICIONES_LIBRES, newParticionLibre(msg->mensaje, msg->mensaje_length));
		log_debug(logger, "El msg no tiene particiones libres a los lados");
	}

	memset(msg->mensaje, '_', msg->mensaje_length);

	log_debug(logger,"El mensaje se ha borrado correctamente, mediante el algoritmo de reemplazo '%s', estado de la cache luego de borrar el mensaje y consolidar: ", datosConfigBroker->algoritmo_reemplazo);
	cache_imprimir1();

}

void consolidar_buddy_system (t_msg* msg) {
	uint32_t tam_particion_consolidado = msg->particion_length;
	t_particion_libre *nuevaPartLibre = NULL;
	t_particion_libre *partCompaniero = NULL;
	log_debug(logger, "-------- Se elimina el sig mensaje ----------------------------");
	log_debug(logger, "Se elimina el msg Pos: %d Tamanio %d", msg->mensaje-CACHE, msg->particion_length);

	nuevaPartLibre = newParticionLibre(msg->mensaje, tam_particion_consolidado);
	list_add(PARTICIONES_LIBRES, nuevaPartLibre);

	partCompaniero = obtenerParticionCompaniero(nuevaPartLibre);

	while (partCompaniero != NULL) {
		log_debug(logger, "Particion Companiero Posicion %d Tamanio %d", partCompaniero->ptr_cache-CACHE, partCompaniero->tamanio);

		nuevaPartLibre = unificar(nuevaPartLibre, partCompaniero);
		log_debug(logger, "Nueva Part. libre Posicion %d Tamanio %d", nuevaPartLibre->ptr_cache-CACHE, nuevaPartLibre->tamanio);

		free(partCompaniero);
		partCompaniero = obtenerParticionCompaniero(nuevaPartLibre);
	};
	memset(msg->mensaje, '_', msg->particion_length);
	log_debug(logger, "Estado de la Cache luego de eliminar:");
	cache_imprimir1();
}

t_particion_libre *obtenerParticionCompaniero(t_particion_libre *nuevaPartLibre) {
	int posicionPart = (nuevaPartLibre->ptr_cache-CACHE) / nuevaPartLibre->tamanio;

	if (posicionPart % 2) {
		/* x es impar */
		t_particion_libre* particionIzquierda;

		for (int i = 0; i < list_size(PARTICIONES_LIBRES); i++) {

			particionIzquierda = list_get(PARTICIONES_LIBRES, i);

			if (particionIzquierda->ptr_cache == nuevaPartLibre->ptr_cache - nuevaPartLibre->tamanio
				&& particionIzquierda->tamanio == nuevaPartLibre->tamanio) {
				return list_remove(PARTICIONES_LIBRES,i);
			}
		}
	} else {
		/* x es par */
		t_particion_libre * particionDerecha;
		for (int i = 0; i < list_size(PARTICIONES_LIBRES); i++) {

			particionDerecha = list_get(PARTICIONES_LIBRES, i);

			if (particionDerecha->ptr_cache == nuevaPartLibre->ptr_cache + nuevaPartLibre->tamanio
				&& particionDerecha->tamanio == nuevaPartLibre->tamanio) {
				return list_remove(PARTICIONES_LIBRES,i);
			}
		}
	}

	return NULL; // retorna NULL en caso de no encontrar
}

t_particion_libre *unificar(t_particion_libre *nuevaPartLibre, t_particion_libre *partCompaniero) {
	nuevaPartLibre->tamanio += partCompaniero->tamanio;
	if (nuevaPartLibre->ptr_cache > partCompaniero->ptr_cache) {
		nuevaPartLibre->ptr_cache = partCompaniero->ptr_cache;
	}

	return nuevaPartLibre;
}

int compactar (){

	log_info(logger,"Ejecucion de compactacion...:");

	ordenarParticionesLibresSegunInicio ();

	int cant_particiones_libres = list_size(PARTICIONES_LIBRES);
	int i;

	if (cant_particiones_libres==1){
		return -1; // no se puede compactar mas
	}

	for (i = 0 ; i< (cant_particiones_libres-1);i++){ //-1 porque cuando ya solo queda una particion es porque es la particion libre
		t_particion_libre* particionActual = (t_particion_libre*) list_get(PARTICIONES_LIBRES,i);
		t_particion_libre* particionSiguiente = (t_particion_libre*) list_get(PARTICIONES_LIBRES,i+1);

		mover_particion(particionActual,particionSiguiente);

		t_particion_libre* particion_a_eliminar = list_remove(PARTICIONES_LIBRES,i);
		free(particion_a_eliminar);
	}

	t_particion_libre* unicaParticionLibre = list_get(PARTICIONES_LIBRES,0);

	log_debug(logger,"Compactacion finalizada");

	memset(unicaParticionLibre->ptr_cache, '_', unicaParticionLibre->tamanio);

	cache_imprimir1();
	return 1;
}

void mover_particion(t_particion_libre* particionLibreActual,t_particion_libre* particionLibreSiguiente){

	//me paro donde empieza el elemento a desplazar
	char* elementoActual = particionLibreActual->ptr_cache + (particionLibreActual->tamanio); //elementoActual -> esta luego de la particion libre

	// inicializo variables auxiliares
	t_msg* msg;
	uint32_t tamanioElementoActual;
	uint32_t desplazamiento = 0;

	//f auxiliar
	bool existe_en_lista_de_msgs (t_msg* msg){
			return elementoActual==msg->mensaje;
	}

	//me esplazo desde el inicio del elemento hasta que llego al primer espacio libre
	while (elementoActual != particionLibreSiguiente->ptr_cache){ //Me desplazo mientras que la particion libre sea diferente a la actual
		msg = list_find(MENSAJES,(void*) existe_en_lista_de_msgs);
		if (msg != NULL){ //si lo encuentro desplazo
			tamanioElementoActual = msg->particion_length;
			elementoActual += tamanioElementoActual;
			desplazamiento += tamanioElementoActual;
			msg->mensaje -= particionLibreActual->tamanio;// Por ultimo tengo que desplazar todos los ptrs de la lista de mensajes

		}
	}

	char* inicioPrimerElemento = particionLibreActual->ptr_cache + (particionLibreActual->tamanio);

	//muevo elemento a posicion libre
	memmove(particionLibreActual->ptr_cache,inicioPrimerElemento,desplazamiento);

	//uno particion libre siguiente con la actual
	particionLibreSiguiente->ptr_cache = particionLibreActual->ptr_cache+desplazamiento; // la nueva particion libre empieza donde termina el desplazamiento
	particionLibreSiguiente->tamanio = particionLibreActual->tamanio+particionLibreSiguiente->tamanio;



	//luego tengo que borrar la particion libre actual de la lista



}

void ordenarParticionesLibresSegunInicio(){ //ordeno particiones segun su inicio

	bool particionAnterior(t_particion_libre * act,t_particion_libre * sig){
		return (act->ptr_cache) < (sig->ptr_cache);
	}

	list_sort(PARTICIONES_LIBRES, (void*) particionAnterior);

}

double exponente_buddy (int largo_mensaje){

	double base = 2.0;
	double exponente = 1.0;
	double len_msg = (double)largo_mensaje;

	while ((pow(base,exponente)<len_msg)){
		exponente = exponente + 1.0;
	}


	return exponente;

}


char* escribir_en_cache(uint32_t tam_particion, void *src, uint8_t codOp, uint32_t largo_pokemon) {
	int offset = 0;
	largo_pokemon -= 1;
	char* ptrCache = busquedaParticionLibre(tam_particion);

	switch (codOp) {
	case broker_NP:; //[tam|pokemon|posx|posy|cant]
		t_msg_2 *msg_NP = src;
		memcpy(ptrCache + offset, &(largo_pokemon), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, msg_NP->pokemon, largo_pokemon);
		offset += largo_pokemon;
		memcpy(ptrCache + offset, &(msg_NP->posicionX), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, &(msg_NP->posicionY), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, &(msg_NP->cantidad), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		log_debug(logger, "Llegada de un nuevo mensaje NEW_POKEMON=[Largo|'pokemon'|posX|posY|cant]=[%d|'%s'|%d|%d|%d]", largo_pokemon, msg_NP->pokemon, msg_NP->posicionX, msg_NP->posicionY, msg_NP->cantidad);
		log_info(logger,"Almacenamiento de NEW_POKEMON en memoria, posicion: %d", ptrCache - CACHE);
		break;
	case broker_LP:; //[tam|pokemon|cant coord|posx|posy]
		t_msg_5 *msg_LP = src;
		//memcpy(...) ignorar id
		//offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, &(largo_pokemon), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, msg_LP->pokemon, largo_pokemon);
		offset += largo_pokemon;
		memcpy(ptrCache + offset, &(msg_LP->cantidad_coordenadas), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		log_debug(logger, "Llegada de un nuevo mensaje LOCALIZED_POKEMON=[id|Largo|'pokemon'|cant de coord]=[%d|%d|'%s'|%d]", msg_LP->idMensaje, largo_pokemon, msg_LP->pokemon, msg_LP->cantidad_coordenadas);
		int iteraciones = (msg_LP->cantidad_coordenadas);
		for (int i = 0; i < iteraciones; i++) {
			t_coordenadas* coord;
			coord = list_get(msg_LP->coordenadas, i);
			memcpy(ptrCache + offset, &(coord->posicionX), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(ptrCache + offset, &(coord->posicionY), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			log_debug(logger, "[posX|posY]=[%d|%d]", coord->posicionX, coord->posicionY);
		}
		log_info(logger,"Almacenamiento de LOCALIZED en memoria, posicion: %d", ptrCache - CACHE);

		break;
	case broker_GP:; // [tam|pokemon]
		t_msg_3b *msg_GP = src;
		memcpy(ptrCache + offset, &(largo_pokemon), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, msg_GP->pokemon, largo_pokemon);
		offset += largo_pokemon;
		log_debug(logger, "Llegada de un nuevo mensaje GET_POKEMON=[Largo|'pokemon']=[%d|'%s']", largo_pokemon, msg_GP->pokemon);
		log_info(logger,"Almacenamiento de GET_POKEMON en memoria, posicion: %d", ptrCache - CACHE);
		break;
	case broker_AP:   // [tam|pokemon|posx|posy]
	case broker_CATP:;
		t_msg_1 *msg_AP_CATP = src;
		memcpy(ptrCache + offset, &(largo_pokemon), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, msg_AP_CATP->pokemon, largo_pokemon);
		offset += largo_pokemon;
		memcpy(ptrCache + offset, &(msg_AP_CATP->posicionX), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, &(msg_AP_CATP->posicionY), sizeof(uint32_t));
		offset += sizeof(uint32_t);

		(codOp == broker_AP ? log_debug(logger, "Llegada de un nuevo mensaje APPEARED_POKEMON=Largo|'pokemon'|posX|posY]=[%d|'%s'|%d|%d]", largo_pokemon, msg_AP_CATP->pokemon, msg_AP_CATP->posicionX, msg_AP_CATP->posicionY) : log_debug(logger, "Llegada de un nuevo mensaje CATCH_POKEMON=Largo|'pokemon'|posX|posY]=[%d|'%s'|%d|%d]", msg_AP_CATP->pokemon_length, msg_AP_CATP->pokemon, msg_AP_CATP->posicionX, msg_AP_CATP->posicionY));
		(codOp == broker_AP ? log_info(logger, "Almacenamiento de APPEARED_POKEMON en memoria, posicion : %d", ptrCache - CACHE) : log_info(logger, "Almacenamiento de CATCH_POKEMON en memoria, posicion : %d", ptrCache - CACHE));

		break;
	case broker_CAUP:; // [atrapo?]
		t_msg_4 *msg_CAUP = src;
		//memcpy(ptrCache + offset, &(msg_CAUP->idMensaje), sizeof(uint32_t));
		//offset += sizeof(uint32_t);
		memcpy(ptrCache + offset, &(msg_CAUP->confirmacion), sizeof(uint32_t));
		offset += sizeof(uint32_t);

		log_debug(logger, "Llegada de un nuevo mensaje CAUGHT_POKEMON=[id|confirmacion]=[%d|%d]", msg_CAUP->idMensaje, msg_CAUP->confirmacion);
		log_info(logger,"Almacenamiento de CAUGHT_POKEMON en memoria, posicion: %d", ptrCache - CACHE);

		break;
	}
	cache_imprimir1();

	return ptrCache;
}

void cache_imprimir1() {
	if (datosConfigBroker->debug_mode == 1) {
		unsigned char ch;

		for (int j = 0; j < datosConfigBroker->tamano_memoria; j++) {
			ch = (unsigned char)CACHE[j];

			if (CACHE[j] == '_') {
				printf(_BLUE "_" _RESET);
			} else if (CACHE[j] == 0) {
				printf(_GREEN "-" _RESET);
			//} else if ((unsigned int)ch >= 48 && (unsigned int)ch <= 57) { // '0' al '9'
			//	printf("%c", CACHE[j]);
			} else if ((unsigned int)ch >= 65 && (unsigned int)ch <= 90) { // 'A' al 'Z'
				printf(_GREEN "%c" _RESET, CACHE[j]);
			} else if ((unsigned int)ch >= 97 && (unsigned int)ch <= 122) { // 'a' al 'z'
				printf(_GREEN "%c" _RESET, CACHE[j]);
			} else if ((unsigned int)ch < 256) {
				printf(_GREEN "%u" _RESET, (unsigned int)ch);
			} else {
				for (int i = 0; i < 8; i++) {
					printf(_GREEN "%d" _RESET, !!((CACHE[j] << i) & 0x80));
				}
			}
			printf(_BLUE "|" _RESET);
		}
		printf("\n");

 		ordenarParticionesLibresSegunInicio();
		t_particion_libre * particionLibre = NULL;
		for (int i = 0; i < list_size(PARTICIONES_LIBRES); i++) {
			
			particionLibre = list_get(PARTICIONES_LIBRES, i);

			printf("posicion %d ", particionLibre->ptr_cache-CACHE);
			printf("tamanio %d|", particionLibre->tamanio);
		}
		printf("\n");
	}
}

char * busquedaParticionLibre(uint32_t tam_particion) {
	int count = 1;
	char *ptr_cache;

	ptr_cache = obtenerPosicionLibreEnCache(tam_particion);

	while (ptr_cache == NULL) {
		if (datosConfigBroker->frecuencia_compactacion != -1 && count > datosConfigBroker->frecuencia_compactacion && !string_contains(datosConfigBroker->algoritmo_memoria,"BS")) {
			compactar();
			ptr_cache = obtenerPosicionLibreEnCache(tam_particion);
			count = 1;
		} else if (datosConfigBroker->frecuencia_compactacion == -1 && list_size(MENSAJES) == 0 && !string_contains(datosConfigBroker->algoritmo_memoria,"BS")) {
			compactar();
			ptr_cache = obtenerPosicionLibreEnCache(tam_particion);
			count = 1;
		}

		if (ptr_cache == NULL) {
			if (string_contains(datosConfigBroker->algoritmo_reemplazo,"FIFO")){
				//pthread_mutex_lock(&esperar_confirmacion);
				eliminarParticionFIFO();
				//pthread_mutex_unlock(&esperar_confirmacion);
				if (count == datosConfigBroker->frecuencia_compactacion && !string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
					compactar();
				}
			}else{
				//pthread_mutex_lock(&esperar_confirmacion);
				eliminarParticionLRU();
				//pthread_mutex_unlock(&esperar_confirmacion);
				if (count == datosConfigBroker->frecuencia_compactacion && !string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
					compactar();
				}
			}

			ptr_cache = obtenerPosicionLibreEnCache(tam_particion);
			count++;
		}
	}

	return ptr_cache;
	/*	1) Buscar
		count=1

		while (no encontro) {
		  if (FREC_COMP != -1 && count > FREC_COMP){
		    2) Compactar
		       Buscar
		       count=1
		  } else if (FREC_COMP == -1 && MENSAJES == NULL) {
		    2) Compactar
		       Buscar
		       count=1
		  }
		  if (no encontro) {
		    3) Eliminar
		       Buscar
		       count++
		  }
		}*/
}

char * obtenerPosicionLibreEnCache(uint32_t tam_particion){
	int count = 0;
	t_particion_libre* particionActual;
	char* ptr_cache;
	double exponente_mensaje_buddy = exponente_buddy(tam_particion);

	if (string_contains(datosConfigBroker->algoritmo_particion_libre,"BF")) //En caso de Best Fit, ordeno lista de particiones libres, para siempre agarrar la mas chica
		ordenarParticionesLibresSegunTamanio();
  else
    ordenarParticionesLibresSegunInicio();

	if (string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
		tam_particion = (int) pow (2.0,exponente_mensaje_buddy);
	}

	// recorro lista de particiones libres hasta que encuentro una que me alcance el tamano
	while (count < list_size(PARTICIONES_LIBRES)){
		particionActual = list_get(PARTICIONES_LIBRES, count);

		if (tam_particion <= particionActual->tamanio) {
			ptr_cache = particionActual->ptr_cache;


			if (string_contains(datosConfigBroker->algoritmo_memoria,"PARTICIONES")){ // PARTICIONAMIENTO DINAMICO

				if (tam_particion < particionActual->tamanio) { //parto en dos la particion libre
					particionActual->ptr_cache += tam_particion;
					particionActual->tamanio -= tam_particion;
				} else {
					free(list_remove(PARTICIONES_LIBRES, count));
				}

			}else{ //BUDDY SYSTEM
				uint32_t tamanio_particion = particionActual->tamanio;
				char* ptr_particionNueva;

				while (exponente_buddy(tamanio_particion) !=  exponente_mensaje_buddy){ //tengo que seguir particionando a la mitad hasta que quede lo mas chica posible la particion
					tamanio_particion = (int) ((tamanio_particion) / 2); //saco tamanio de las particiones a dividir
					ptr_particionNueva = particionActual->ptr_cache + tamanio_particion;
					memset(ptr_particionNueva, '_', tamanio_particion);

					list_add_in_index(PARTICIONES_LIBRES,count+1,newParticionLibre(ptr_particionNueva, tamanio_particion));//y creo part. derecha libre
				}

				free(list_remove(PARTICIONES_LIBRES, count));
			}

			return ptr_cache;
		}

		count ++;
	}

	return NULL; // no encontró particion libre
}

void fragmentarParticionLibre(t_particion_libre* ptrParticionLibre, uint32_t largoMensaje, int indice){

	//parto en dos la particion libre
	if (largoMensaje == ptrParticionLibre->tamanio) {

		free(list_remove(PARTICIONES_LIBRES, indice));

	} else if (largoMensaje < ptrParticionLibre->tamanio) {

 		ptrParticionLibre->ptr_cache += largoMensaje;

		ptrParticionLibre->tamanio -= largoMensaje;

	}
}

void ordenarParticionesLibresSegunTamanio(){

	bool mayorTamanio(t_particion_libre * act,t_particion_libre * sig){
		return (act->tamanio) < (sig->tamanio);
	}

	list_sort(PARTICIONES_LIBRES, (void*) mayorTamanio);

}

t_particion_libre * newParticionLibre(char* ptrCache, uint32_t largoMensaje){
	t_particion_libre* nodo_particion_libre;

	nodo_particion_libre = malloc(sizeof(t_particion_libre));
	nodo_particion_libre->ptr_cache = ptrCache;
	nodo_particion_libre->tamanio = largoMensaje;

	return nodo_particion_libre;
}

t_msg* agregar_a_lista_memoria(uint32_t id, uint8_t cod_op, uint32_t tam_msg ,uint32_t tam_particion, char* ptr_cache) {

	t_msg* nodo_mensaje = inicializar_nodo_mensaje (id, cod_op);

	switch (cod_op) {
	case broker_NP:;
		log_debug(logger, "broker_NP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_NEWPOK);
		break;

	case broker_AP:;
		log_debug(logger, "broker_AP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_APPEARED);
		break;

	case broker_GP:;
		log_debug(logger, "broker_GP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_GETPOK);
		break;

	case broker_LP:;
		log_debug(logger, "broker_LP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_LOCALIZED);
		break;

	case broker_CATP:;
		log_debug(logger, "broker_CATP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_CATCHPOK);
		break;

	case broker_CAUP:;
		log_debug(logger, "broker_CAUP");
		nodo_mensaje->suscriptores = generar_lista_suscriptos(SUSCRIPTORES_CAUGHTPOK);
		break;

	default:
		log_debug(logger, "Error de cod_op en agregar a lista de memoria");
	}

	nodo_mensaje->particion_length = tam_particion;
	nodo_mensaje->mensaje_length = tam_msg;
	nodo_mensaje->mensaje = ptr_cache;

	agregar_a_memoria (nodo_mensaje);
	return nodo_mensaje;
}

void agregar_a_memoria (t_msg* nodo_mensaje){
	pthread_mutex_lock(&mutex_agregar_msg_a_memoria);
	list_add(MENSAJES, nodo_mensaje);
	pthread_mutex_unlock(&mutex_agregar_msg_a_memoria);
}


t_list* generar_lista_suscriptos(t_list* suscritores_en_la_lista_global){
	t_list* suscriptores = list_create();
	int count = 0;

	while (count<list_size(suscritores_en_la_lista_global)){
		t_msg_suscriptor * nuevo_suscriptor;
		nuevo_suscriptor = malloc(sizeof(t_msg_suscriptor));

		nuevo_suscriptor -> ptr_suscriptor = list_get(suscritores_en_la_lista_global,count);
		nuevo_suscriptor -> flag_ACK = 0;

		list_add(suscriptores,nuevo_suscriptor);

		count++;
	}
	log_debug(logger,"----------------------> generar_lista_suscriptos %d", count);

	return suscriptores;
}

t_msg * inicializar_nodo_mensaje (uint32_t id, uint8_t cod_op){
	t_msg * nodo_mensaje;

	nodo_mensaje = malloc(sizeof(t_msg));

	nodo_mensaje->timestamp = obtenerTimeStamp();
	nodo_mensaje->id_mensaje = id;
	nodo_mensaje->op_code = cod_op;

	return nodo_mensaje;
}


unsigned long long obtenerTimeStamp()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long millisecondsSinceEpoch =
	    (unsigned long long)(tv.tv_sec) * 1000 +
	    (unsigned long long)(tv.tv_usec) / 1000;

	return millisecondsSinceEpoch;
}

uint32_t obtener_tam_msg(uint8_t codOp, uint32_t pokemon_length, uint32_t cantidad_coordenadas) {
	uint32_t tam_segun_cod_op;

	switch (codOp) {
	case broker_NP:;
		tam_segun_cod_op = pokemon_length - 1 + sizeof(uint32_t) * 4; //[tam|pokemon|posx|posy|cant]
		break;

	case broker_LP:;
		tam_segun_cod_op = pokemon_length - 1 + sizeof(uint32_t) * 2 + (cantidad_coordenadas * sizeof(uint32_t) * 2); //[tam|pokemon|cantCoord|(posx|posy)+]
		break;

	case broker_GP:;
		tam_segun_cod_op = pokemon_length - 1 + sizeof(uint32_t); // [tam|pokemon]
		break;

	case broker_AP:;
		tam_segun_cod_op = pokemon_length - 1 + sizeof(uint32_t) * 3; // [tam|pokemon|posx|posy]
		break;

	case broker_CATP:;
		tam_segun_cod_op = pokemon_length - 1 + sizeof(uint32_t) * 3; // [tam|pokemon|posx|posy]
		break;

	case broker_CAUP:;
		tam_segun_cod_op = sizeof(uint32_t); // [atrapo?]
		break;
	}

	if (tam_segun_cod_op > datosConfigBroker->tamano_minimo_particion) {
		return tam_segun_cod_op;
	} else {
		return datosConfigBroker->tamano_minimo_particion;
	}

}

uint32_t obtener_tam_particion(uint32_t tam_msg){

	double exponente_mensaje_buddy = exponente_buddy(tam_msg);

	if (string_contains(datosConfigBroker->algoritmo_memoria,"BS")){
		return ((uint32_t) pow (2.0,exponente_mensaje_buddy));
	}else{
		return tam_msg;
	}

}
