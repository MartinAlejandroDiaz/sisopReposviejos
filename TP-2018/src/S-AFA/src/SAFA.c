#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "consolaSAFA.h"
#include "libSAFA.h"

void disconnect(socket_connection* socketInfo);

int main(void){      
        cantSentenciasEjecutadas = 0;
        cantSentConDiego = 0;
        unCpuConectado = false;
        damConectado = false;
        generadorDeIdsCPU = 1;
        //señal para que al cortar el flujo, se libere memoria
        signal(SIGINT, cerrarPrograma);

     // diccionarios para la funcion runFunction....
     fns = dictionary_create();
     dictionary_put(fns,"identificarProcesoEnSAFA",&identificarProceso);
     dictionary_put(fns, "identificarNuevaConexion", &newConnection); 
     dictionary_put(fns, "finalizacionProcesamientoCPU", &finalizacionProcesamientoCPU);
     dictionary_put(fns, "avisoDamDTB", &avisoDeDamDeResultadoDTB);
     dictionary_put(fns, "DAM_SAFA_desbloquearDTB", &desbloquearDTB);
     dictionary_put(fns, "DAM_SAFA_pasarDTBAExit", &pasarDTBAExit);
     dictionary_put(fns, "CPU_SAFA_pasarDTBAExit", &pasarDTBAExit);
     dictionary_put(fns, "waitRecurso", &waitRecurso);
     dictionary_put(fns, "signalRecurso", &signalRecurso);
     dictionary_put(fns, "CPU_SAFA_verificarEstadoArchivo", &verificarEstadoArchivo);
     

        
        colaReady = queue_create();
        colaBloqueados = queue_create();
        colaFinalizados = queue_create();
        colaNew = queue_create();
        listaEjecutando = list_create();
        hilos = list_create();
        listaCPUs = list_create();
        listaDeRecursos = list_create();

        pthread_t hiloConsola, hiloPLP;
        list_add(hilos, &hiloConsola);
        list_add(hilos, &hiloPLP);


        //configuracion de loggers
	configure_logger();
        read_and_log_config("S-AFA.config");
	log_info(logger, "Voy a escuchar por el puerto: %d", datosConfigSAFA->puerto);

        sem_init(&puedeEntrarAlSistema, 0, datosConfigSAFA->gradoMultiprog);
        sem_init(&cantProcesosEnNew, 0, 0);
        sem_init(&cantProcesosEnReady, 0, 0);
        pthread_mutex_init(&m_puedePlanificar, NULL);
        pthread_mutex_init(&m_colaReady, NULL);
	pthread_mutex_init(&m_colaBloqueados, NULL);
        pthread_mutex_init(&m_colaFinalizados, NULL);
        pthread_mutex_init(&m_colaNew, NULL);
        pthread_mutex_init(&m_listaEjecutando, NULL);
        pthread_mutex_init(&m_listaDeRecursos, NULL);

        pthread_mutex_init(&m_busqueda, NULL);
        pthread_mutex_init(&m_recurso, NULL);
        pthread_mutex_init(&m_verificacion, NULL);
        pthread_mutex_init(&m_cantSent, NULL);
        pthread_mutex_init(&m_cantDiego, NULL);

        pthread_create(&hiloConsola, NULL, (void*)&consolaSAFA, NULL);       
        pthread_create(&hiloPLP, NULL, (void*)&planificadorLargoPlazo, NULL);
       
        pthread_detach(hiloConsola);   
        pthread_detach(hiloPLP);

        estadoCorrupto = true;
        //pongo a escuchar el server
	createListen(datosConfigSAFA->puerto, NULL, fns, &disconnect, NULL);


        
  	// bloqueo el thread, dejando la coneccion abierta
 	pthread_mutex_init(&mx_main, NULL);
	pthread_mutex_lock(&mx_main);
	pthread_mutex_lock(&mx_main);       

        
        //cerrar hilos, no se como se hace

    return EXIT_SUCCESS;
}



void disconnect(socket_connection* socketInfo) {
  log_info(logger,"socket n° %d se ha desconectado.\n", socketInfo->socket);
}

void cerrarPrograma() {
    log_info(logger, "Voy a cerrar SAFA");
    pthread_mutex_destroy(&m_colaReady);
    pthread_mutex_destroy(&m_colaBloqueados);
    pthread_mutex_destroy(&m_colaNew);
    pthread_mutex_destroy(&m_listaEjecutando);
    pthread_mutex_destroy(&m_busqueda);
    pthread_mutex_destroy(&m_colaFinalizados);
    pthread_mutex_destroy(&m_listaDeRecursos);
    pthread_mutex_destroy(&m_recurso);
    pthread_mutex_destroy(&m_verificacion);
    pthread_mutex_destroy(&m_cantSent);
    pthread_mutex_destroy(&m_cantDiego);

    close_logger();
    dictionary_destroy(fns); 
    free(datosConfigSAFA->algoritmoPlanif);
    free(datosConfigSAFA);

    queue_destroy_and_destroy_elements(colaReady, (void*)free);
    queue_destroy_and_destroy_elements(colaFinalizados, (void*)free);
    queue_destroy_and_destroy_elements(colaNew, (void*)free);
    queue_destroy_and_destroy_elements(colaBloqueados, (void*)free);
    list_destroy(hilos);
    list_destroy_and_destroy_elements(listaCPUs, (void*)free);
    list_destroy(listaEjecutando);
    list_destroy_and_destroy_elements(listaDeRecursos, (void*)&destruirRecurso);


    pthread_mutex_unlock(&mx_main);
    pthread_mutex_destroy(&mx_main);
}
