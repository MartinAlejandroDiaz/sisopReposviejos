#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "libFM9.h"

void cerrarPrograma() {
	log_info(logger, "Voy a cerrar FM9");

	close_logger();
	free(datosConfigFM9);
	dictionary_destroy(callableRemoteFunctions);
	pthread_mutex_unlock(&mx_main);
	pthread_mutex_destroy(&mx_main);
}

int main(void) {
	//Esto es para liberar memoria despues de ctrl-c
	signal(SIGINT, cerrarPrograma);

	configure_logger();

	datosConfigFM9 = read_and_log_config("FM9.config");

	callableRemoteFunctions = dictionary_create();

	dictionary_put(callableRemoteFunctions, "identificarProcesoEnFM9", &identificarProceso);
	dictionary_put(callableRemoteFunctions, "DAM_FM9_cargarArchivo", &solicitudCargaArchivo);
	dictionary_put(callableRemoteFunctions, "CPU_FM9_actualizarLosDatosDelArchivo", &actualizarDatosDTB);
	dictionary_put(callableRemoteFunctions, "CPU_FM9_cerrarElArchivo", &cerrarArchivoDelDTB);
	//Funcion para iniciar la ejecucion del Flush
	//dictionary_put(callableRemoteFunctions, "DAM_FM9_obtenerArchivo", &obtenerArchivo);

	log_info(logger, "Voy a escuchar el puerto: %d", datosConfigFM9->puerto);

	createListen(datosConfigFM9->puerto,NULL,
			callableRemoteFunctions, &disconnect, NULL);

	log_info(logger, "Estoy escuchando el puerto: %d", datosConfigFM9->puerto);

	//conexion al servidor-----------------------------
	
	
	pthread_mutex_init(&mx_main, NULL);
	pthread_mutex_lock(&mx_main);
	pthread_mutex_lock(&mx_main);
	return EXIT_SUCCESS;
}

