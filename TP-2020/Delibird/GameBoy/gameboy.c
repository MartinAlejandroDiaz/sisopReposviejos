#include "libGameBoy.h"

int main(int argc, char** argv) {

	creacion_logger();
	read_and_log_config();

	ejecutarParametrosSobreScript(argc, argv);

	config_destroy(datosConfigGameBoy->configuracion);
	free(datosConfigGameBoy);
	log_destroy(logger);

	return 0;
}
