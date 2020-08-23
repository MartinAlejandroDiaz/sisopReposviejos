#include "libMDJ.h"

//PATH RELATIVO PQ SINO NO ANDA EN OTRAS COMPUS
char * path = "MDJ.config";



int main(void) {
      
     signal(SIGINT, cerrarPrograma);
     configure_logger();
     datosConfMDJ = read_and_log_config(path);
     portServer = datosConfMDJ->puerto;
     fns = dictionary_create();
     dictionary_put(fns,"validarArchivo",&validarArchivo);
     dictionary_put(fns,"crearArchivo",&crearArchivo);
     dictionary_put(fns,"borrarArchivo",&borrarArchivo);
     dictionary_put(fns,"identificarProcesoEnMDJ",&identificarProceso);
     //dictionary_put(fns,"verificarSiExisteArchivo",&verificarSiExisteArchivo);

       //Pongo a escuchar el server en el puerto elegido
       int listener =  createListen(portServer, NULL ,fns, &disconnect ,NULL);
        if(listener == -1)
	{ 
        log_error(logger,"Error al crear escucha en puerto %d.\n", portServer);
		exit(1);
	}

     
	printf("Escuchando nuevos clientes en puerto %d.\n", portServer);

	
	pthread_mutex_init(&mx_main, NULL);
	pthread_mutex_lock(&mx_main);
	pthread_mutex_lock(&mx_main);

  
	return EXIT_SUCCESS;
}

void cerrarPrograma() {
    log_info(logger, "Voy a cerrar MDJ");
    close_logger();
    dictionary_destroy(fns);
    //free(datosConfMDJ->ptoMontaje);
    free(datosConfMDJ);
    pthread_mutex_unlock(&mx_main);
    pthread_mutex_destroy(&mx_main);
}



