#include <stdio.h>
#include <stdlib.h>
#include "libFM9.h"
#include <string.h>


//LOG
void configure_logger() {

	char * nombrePrograma = "FM9.log";
	char * nombreArchivo = "FM9";

	logger = log_create(nombrePrograma, nombreArchivo, true, LOG_LEVEL_TRACE);
	log_info(logger, "Se genero log de FM9");
}

void close_logger() {
	log_info(logger, "Cierro log de FM9");
	log_destroy(logger);
}


//SOCKETS
void  identificarProceso(socket_connection * connection ,char** args)
{
     log_info(logger,"Se ha conectado %s en el socket NRO %d  con IP %s,  PUERTO %d\n", args[0],connection->socket,connection->ip,connection-> port);
} 



void disconnect(socket_connection* socketInfo) {
	log_info(logger, "El socket n°%d se ha desconectado.", socketInfo->socket);
}


//CONFIG
t_config_FM9* read_and_log_config(char* path) {
	log_info(logger, "Voy a leer el archivo FM9.config");

	t_config* archivo_Config = config_create(path);
	if (archivo_Config == NULL) {
		log_error(logger, "No existe archivo de configuracion");
		exit(1);
	}

	t_config_FM9* _datosFM9 = malloc(sizeof(t_config_FM9));

	_datosFM9->puerto = config_get_int_value(archivo_Config, "PUERTO");
	char* modo = string_new();
	string_append(&modo, config_get_string_value(archivo_Config, "MODO"));
	_datosFM9->modo = modo;
	_datosFM9->tamanio = config_get_int_value(archivo_Config, "TAMANIO");
	_datosFM9->maximoLinea = config_get_int_value(archivo_Config, "MAX_LINEA");
	_datosFM9->tamanioPagina = config_get_int_value(archivo_Config,
			"TAM_PAGINA");

	log_info(logger, "	PUERTO: %d", _datosFM9->puerto);
	log_info(logger, "	MODO: %s", _datosFM9->modo);
	log_info(logger, "	TAMANIO: %d", _datosFM9->tamanio);
	log_info(logger, "	MAX_LINEA: %d", _datosFM9->maximoLinea);
	log_info(logger, "	TAM_PAGINA: %d", _datosFM9->tamanioPagina);

	log_info(logger, "Fin de lectura");

	config_destroy(archivo_Config);
	free(modo);

	return _datosFM9;
}

//args[0]: idGDT
//Comunicacion para Desarrollar Cuando El DAM pida a FM9 cargar el archivo ya sea un DTB o el Dummy
void solicitudCargaArchivo(socket_connection* connection, char** args){

	if(1){ runFunction(connection->socket, "FM9_DAM_cargueElArchivoCorrectamente",2,args[0], "ok");}

	else{ runFunction(connection->socket, "FM9_DAM_cargueElArchivoCorrectamente", 2, args[0], "error");}


}

//args[0]: idGDT, args[1]: Path, args[2]:Linea, args[3]:Datos
void actualizarDatosDTB(socket_connection* connection, char** args){

	int idGDT = atoi(args[0]);
	char* path = args[1];
	size_t linea = atoi(args[2]);
	char* datos = args[3];

	log_info(logger, "Del GDT: %d, recibi los siguientes Datos: %s, %d, %s", idGDT, path, linea, datos);

}

//args[0]: idGDT, args[1]: rutaScript
void cerrarArchivoDelDTB(socket_connection* connection, char** args){

	//Realizar Operacion de Cerrado

	runFunction(connection->socket, "FM9_CPU_resultadoDeClose", 2 , args[0], "1");

}
