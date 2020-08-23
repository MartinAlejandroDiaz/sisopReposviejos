#include  <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../../sample-socket/socket.h"
#include "consolaMDJ.h"
#include "interfaz.h"



//ESTRUCTURAS
typedef struct {
	int puerto;
	char * ptoMontaje;
	int  retardo;
} t_config_MDJ;




//VARIABLES
t_log * logger;
t_dictionary *  fns;	/* Funciones de socket */
pthread_mutex_t mx_main;	/* Semaforo de main */
int portServer;
t_config * conf;
t_config_MDJ * datosConfMDJ;



//FUNCIONES
void configure_logger();
void close_logger();
void cerrarPrograma();



//CONFIG

t_config_MDJ * read_and_log_config(char*);

//SOCKETS
void disconnect(socket_connection *);
void identificarProceso(socket_connection * connection ,char** args);
void validarExistencia(socket_connection * , char ** );
t_metadata_filesystem * obtenerMetadata ();