/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#include "tp0.h"

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
	int conexion = 1;
	char* ip;
	char* puerto;
	char* mensaje_a_enviar;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	//Loggear "soy un log"
	log_debug(logger, "soy un log");

	config = leer_config(logger);


	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion
	//guardo ip y puerto
	ip = config_get_string_value(config,"IP");
	puerto = config_get_string_value(config,"PUERTO");
	conexion = crear_conexion(ip, puerto);

	//enviar mensaje
char * stream1 = "PIKACHU 5 10 2";
	mensaje_a_enviar = malloc(strlen(stream1) + 1); // reservo memoria para la cadena harcodeada "hola mundo!"
	memcpy(mensaje_a_enviar, stream1, strlen(stream1) + 1); // copio la cadena a mensaje_a_enviar
	enviar_mensaje(mensaje_a_enviar, conexion); // envio el mensaje
	free(mensaje_a_enviar); // apenas termino de usar el string lo libero

	//recibir mensaje
	char* mensaje_recibido = NULL;
	mensaje_recibido = recibir_mensaje(conexion);

	//loguear mensaje recibido
	log_debug(logger, mensaje_recibido);
	free(mensaje_recibido); // apenas termino de usar el string lo libero

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void)
{
	bool mostrarEnPantalla = true;
	int log_info_level = 1;

	return log_create("tp0.log", "tp0", mostrarEnPantalla, log_info_level);
}

t_config* leer_config(t_log* logger)
{
	log_debug(logger, "leer config");
	return config_create("tp0.config");
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
	liberar_conexion(conexion);
	log_debug(logger, "liberar todo");
	log_destroy(logger);
	config_destroy(config);
}
