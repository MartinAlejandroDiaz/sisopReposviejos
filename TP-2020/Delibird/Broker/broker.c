#include "libBroker.h"

int main() {

	read_and_log_config();

	iniciar_logger();

	log_debug(logger,"Iniciando broker... (PID = %d) \n",getpid());

	inicializar();

	signal(SIGUSR1,controlador_de_senial);
	signal(SIGINT,controlador_de_senial);

	iniciar_servidor();


	config_destroy(datosConfigBroker->configuracion);
	free(datosConfigBroker);
	log_destroy(logger);

	return EXIT_SUCCESS;
}
