#include "libBroker.h"

void read_and_log_config(){

	t_config* archivo_Config = config_create("broker.config");

	if(archivo_Config == NULL){
		exit(1);
	}

	datosConfigBroker = malloc(sizeof(t_config_broker));

	datosConfigBroker->tamano_memoria = config_get_int_value(archivo_Config, "TAMANO_MEMORIA");
	datosConfigBroker->tamano_minimo_particion = config_get_int_value(archivo_Config, "TAMANO_MINIMO_PARTICION");
	datosConfigBroker->algoritmo_memoria = config_get_string_value(archivo_Config, "ALGORITMO_MEMORIA");
	datosConfigBroker->algoritmo_reemplazo = config_get_string_value(archivo_Config, "ALGORITMO_REEMPLAZO");
	datosConfigBroker->algoritmo_particion_libre = config_get_string_value(archivo_Config, "ALGORITMO_PARTICION_LIBRE");
	datosConfigBroker->ip_broker = config_get_string_value(archivo_Config, "IP_BROKER");
	datosConfigBroker->puerto_broker = config_get_string_value(archivo_Config, "PUERTO_BROKER");
	datosConfigBroker->frecuencia_compactacion = config_get_int_value(archivo_Config, "FRECUENCIA_COMPACTACION");
	datosConfigBroker->log_file = config_get_string_value(archivo_Config, "LOG_FILE");
	datosConfigBroker->debug_mode = config_get_int_value(archivo_Config, "DEBUG_MODE");
	datosConfigBroker->configuracion = archivo_Config;

	if (datosConfigBroker->debug_mode == 1) {
		srand(time(NULL));
		int r = rand() % 1000;
		printf("----------random malloc----------> %d\n", r);
		randomMalloc = malloc(r);
	}
}

void iniciar_logger(){
	if (datosConfigBroker->debug_mode == 1) {
		logger = log_create(datosConfigBroker->log_file , "Broker", 1, LOG_LEVEL_DEBUG);
	} else {
		logger = log_create(datosConfigBroker->log_file , "Broker", 0, LOG_LEVEL_INFO);
	}
}

void inicializar(){
	//inicializar listas de suscriptores
	SUSCRIPTORES_NEWPOK = list_create();
	SUSCRIPTORES_CATCHPOK = list_create();
	SUSCRIPTORES_GETPOK = list_create();
	SUSCRIPTORES_APPEARED = list_create();
	SUSCRIPTORES_CAUGHTPOK = list_create();
	SUSCRIPTORES_LOCALIZED = list_create();

	//inicializar semaforos
	//sem_init(&sem_cache_mensajes, 0, 1);
	pthread_mutex_init(&mutex_mensajes,NULL);

	inicializarMemoria();

	//inicializo el mutex para los ids
	ID_CORRELATIVO=0;
	pthread_mutex_init(&mutex_ids,NULL);
	pthread_mutex_init(&mutex_agregar_msg_a_memoria,NULL);
	//pthread_mutex_init(&esperar_confirmacion,NULL);

}

t_suscriptor* suscribir_cola(t_sol_suscripcion* sol_suscripcion, int socketSuscriptor, bool *seSuscribePor1eraVez) {

	t_list* lista_global = list_segun_mensaje_suscripcion(sol_suscripcion->colaMensaje);

	t_suscriptor* ptr_suscriptor = obtenerSuscriptor(sol_suscripcion,lista_global);

	if (ptr_suscriptor == NULL) {

		ptr_suscriptor = malloc (sizeof(t_suscriptor));
		ptr_suscriptor->ip = NULL;
		ptr_suscriptor->puerto = NULL;

		if(sol_suscripcion->ip_length > 0){

			ptr_suscriptor->ip = malloc(sol_suscripcion->ip_length);
			memcpy(ptr_suscriptor->ip, sol_suscripcion->ip, sol_suscripcion->ip_length);
			ptr_suscriptor->puerto = malloc(sol_suscripcion->puerto_length);
			memcpy(ptr_suscriptor->puerto, sol_suscripcion->puerto, sol_suscripcion->puerto_length);
		}

		ptr_suscriptor->socket = socketSuscriptor;

		list_add(lista_global, ptr_suscriptor);

		*seSuscribePor1eraVez = true;

		log_info(logger, "Nueva suscripcion IP: '%s' puerto: '%s' socket: '%d' cola: '%s'",ptr_suscriptor->ip,ptr_suscriptor->puerto,ptr_suscriptor->socket,sol_suscripcion->colaMensaje);

	} else {

		ptr_suscriptor->socket = socketSuscriptor;

		*seSuscribePor1eraVez = false;

		log_info(logger, "Resuscripcion del proceso caido IP: '%s' puerto: '%s' socket: '%d' cola: '%s'",ptr_suscriptor->ip,ptr_suscriptor->puerto,ptr_suscriptor->socket,sol_suscripcion->colaMensaje);
	}

	return ptr_suscriptor;
}

t_suscriptor* obtenerSuscriptor (t_sol_suscripcion* sol_suscripcion, t_list* lista_global) {
//retorna NULL si no se encuentra suscripto o si es un gameboy
	if (sol_suscripcion->ip_length == 0)
		return NULL;

	t_suscriptor* ptr_suscriptor;

	for(int i = 0; i < list_size(lista_global); i++){
		ptr_suscriptor = list_get(lista_global,i);

		if (string_equals_ignore_case(ptr_suscriptor->ip,sol_suscripcion->ip) && string_equals_ignore_case(ptr_suscriptor->puerto,sol_suscripcion->puerto))
			return ptr_suscriptor;
	}

	return NULL;
}


t_list* list_segun_mensaje_suscripcion(char* colaMensaje) {

	if (string_contains(colaMensaje, "NEW_POKEMON")) {
		return SUSCRIPTORES_NEWPOK;
	}

	else if (string_contains(colaMensaje, "CATCH_POKEMON")) {
		return SUSCRIPTORES_CATCHPOK;
	}

	else if (string_contains(colaMensaje, "GET_POKEMON")) {
		return SUSCRIPTORES_GETPOK;
	}

	else if (string_contains(colaMensaje, "APPEARED_POKEMON")) {
		return SUSCRIPTORES_APPEARED;
	}

	else if (string_contains(colaMensaje, "CAUGHT_POKEMON")) {
		return SUSCRIPTORES_CAUGHTPOK;
	}

	else {
		return SUSCRIPTORES_LOCALIZED;
	}
}

void controlador_de_senial (int n){

	switch (n) {
	case SIGUSR1:
		dump();
		break;
	case SIGINT:
		liberarMemoria();
		break;
	}

}

char* retornarFechaHora (){

	char* output = malloc(128);

    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(output,128,"%d-%m-%y %H:%M:%S",tlocal);

    return output;
}


void dump (){

	log_info(logger,"Se solicito un dump de la cache");

	char* fechaHora = retornarFechaHora();//hay que agregar un semaforo

	char nombreArchivo [133] = "Dump ";
	strcat(nombreArchivo,fechaHora);

	FILE * file_dump = fopen(nombreArchivo,"w");

	ordenarParticionesLibresSegunInicio();

	int count_particiones = 1;

	int count_mensajes = 0;
	int count_particiones_libres = 0;

	int inicio_particion_decimal = 0;
	int fin_particion_decimal = 1;

	int cantMensajes = list_size(MENSAJES);
	int cantParticionesLibres = list_size(PARTICIONES_LIBRES);

	t_msg* mensaje = list_get(MENSAJES,0);
	t_particion_libre* particion_libre =  list_get(PARTICIONES_LIBRES,0);

	fprintf(file_dump,"Dump: %s \n", fechaHora);

	while (cantMensajes > count_mensajes || cantParticionesLibres > count_particiones_libres) {

		if((cantParticionesLibres > count_particiones_libres && cantMensajes > count_mensajes)){

			if(mensaje->mensaje > particion_libre->ptr_cache){

				fin_particion_decimal = inicio_particion_decimal + particion_libre->tamanio -1;

				fprintf(file_dump,"Particion %d : 0x%03x- 0x%03x.	[L]		Size: %db \n",count_particiones,inicio_particion_decimal,fin_particion_decimal,particion_libre->tamanio);

				count_particiones++;
				count_particiones_libres++;
				inicio_particion_decimal = fin_particion_decimal + 1;
				particion_libre =  list_get(PARTICIONES_LIBRES,count_particiones_libres);

			}else{

				fin_particion_decimal = inicio_particion_decimal + mensaje->particion_length - 1;

				fprintf(file_dump,"Particion %d : 0x%03x- 0x%03x.	[X]		Size: %db		LRU: %d 	ID:%d \n",count_particiones,inicio_particion_decimal,fin_particion_decimal,mensaje->particion_length,(int) mensaje->timestamp,(int) mensaje->id_mensaje); // falta que el dump sea de 6 digitos

				count_particiones++;
				count_mensajes++;
				inicio_particion_decimal = fin_particion_decimal + 1;
				mensaje = list_get(MENSAJES,count_mensajes);
			}

		}else if ((cantParticionesLibres <= count_particiones_libres && cantMensajes > count_mensajes)){

			fin_particion_decimal = inicio_particion_decimal + mensaje->particion_length - 1;

			fprintf(file_dump,"Particion %d : 0x%03x- 0x%03x.	[X]		Size: %db		LRU: %d 	ID:%d \n",count_particiones,inicio_particion_decimal,fin_particion_decimal,mensaje->particion_length,(int) mensaje->timestamp,(int) mensaje->id_mensaje); // falta que el dump sea de 6 digitos

			count_particiones++;
			count_mensajes++;
			inicio_particion_decimal = fin_particion_decimal + 1;
			mensaje = list_get(MENSAJES,count_mensajes);

		}else if (cantParticionesLibres > count_particiones_libres && cantMensajes <= count_mensajes){

				fin_particion_decimal = inicio_particion_decimal + particion_libre->tamanio -1;

				fprintf(file_dump,"Particion %d : 0x%03x- 0x%03x.	[L]		Size: %db \n",count_particiones,inicio_particion_decimal,fin_particion_decimal,particion_libre->tamanio);

				count_particiones++;
				count_particiones_libres++;
				inicio_particion_decimal = fin_particion_decimal + 1;
				particion_libre =  list_get(PARTICIONES_LIBRES,count_particiones_libres);

			}
		}

	log_debug(logger, "El dump de la cache ha sido realizado");
	fclose(file_dump);
	free(fechaHora);

}

void liberarMemoria() {

	// Liberacion sobre cola NEW
	list_iterate(SUSCRIPTORES_NEWPOK, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_NEWPOK);

	// Liberacion sobre cola CATCH
	list_iterate(SUSCRIPTORES_CATCHPOK, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_CATCHPOK);

	// Liberacion sobre cola GET
	list_iterate(SUSCRIPTORES_GETPOK, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_GETPOK);

	// Liberacion sobre cola APPEARED
	list_iterate(SUSCRIPTORES_APPEARED, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_APPEARED);

	// Liberacion sobre cola CAUGHT
	list_iterate(SUSCRIPTORES_CAUGHTPOK, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_CAUGHTPOK);

	// Liberacion sobre cola LOCALIZED
	list_iterate(SUSCRIPTORES_LOCALIZED, (void*) liberarEstructuraSuscriptor);
	list_destroy(SUSCRIPTORES_LOCALIZED);

	list_iterate(PARTICIONES_LIBRES, (void*) free);
	list_destroy(PARTICIONES_LIBRES);

	// Liberacion lista de Mensajes
	list_iterate(MENSAJES, (void*) liberarEstructuraMensaje);
	list_destroy(MENSAJES);

	if (datosConfigBroker->debug_mode == 1) {
		free(randomMalloc);
	}
	config_destroy(datosConfigBroker->configuracion);
	free(datosConfigBroker);
	log_destroy(logger);
	exit(0);
}

void liberarEstructuraSuscriptor(t_suscriptor* suscriptor){
	free(suscriptor->ip);
	free(suscriptor->puerto);
	free(suscriptor);
}

void liberarEstructuraMensaje(t_msg* mensaje) {
	list_iterate(mensaje->suscriptores, (void*) free);
	list_destroy(mensaje->suscriptores);
	free(mensaje);
}
