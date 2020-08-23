#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "libDAM.h"

void disconnect(socket_connection* socketInfo);

int main(void){

	 signal(SIGINT, cerrarPrograma);
	
       	configure_logger();
	datosConfigDAM =read_and_log_config("DAM.config");
	//diccionarios

        callableRemoteFunctionsMDJ = dictionary_create();
	callableRemoteFunctionsFM9 = dictionary_create();
	 callableRemoteFunctionsSAFA = dictionary_create();
	callableRemoteFunctionsCPU = dictionary_create();
	
	//--------------------------------------------------------------------
	//Dicionarios del mdj
    dictionary_put(callableRemoteFunctionsMDJ, "MDJ_DAM_existeArchivo", &MDJ_DAM_existeArchivo);
    dictionary_put(callableRemoteFunctionsMDJ, "MDJ_DAM_verificarArchivoCreado",&MDJ_DAM_verificarArchivoCreado);
    dictionary_put(callableRemoteFunctionsMDJ, "MDJ_DAM_verificameSiArchivoFueBorrado",&MDJ_DAM_verificameSiArchivoFueBorrado);
    //dictionary_put(callableRemoteFunctionsMDJ, "MDJ_DAM_avisoSAFAResultadoDTBDummy",&MDJ_DAM_avisarSAFAResultadoDTBDummy);
	//--------------------------------------------------------------------
  //dicionarios de CPU
      dictionary_put(callableRemoteFunctionsCPU, "CPU_DAM_solicitudCargaGDT", &solicitudCargaGDT);
      dictionary_put(callableRemoteFunctionsCPU, "CPU_DAM_existeArchivo", &existeArchivo);
      dictionary_put(callableRemoteFunctionsCPU, "identificarProcesoEnDAM", &identificarProceso);
      dictionary_put(callableRemoteFunctionsCPU, "CPU_DAM_crearArchivo", &crearArchivo);
    
	socketSAFA = connectServer("172.17.0.1", 8001,callableRemoteFunctionsSAFA, &disconnect, NULL);
  //CUANDO ME CONECTO AL SAFA LE DIGO QUE SOY EL PROCESO DAM (para manejar estadoCorrupto)
  runFunction(socketSAFA,"identificarNuevaConexion",1,"DAM"); 
	socketFM9 = connectServer("172.17.0.1",8003, callableRemoteFunctionsFM9, &disconnect, NULL);
	socketMDJ = connectServer("172.17.0.1", 8002, callableRemoteFunctionsMDJ, &disconnect, NULL);
         
	
	if(socketSAFA== -1  ){
	log_error(logger,"no se pudo conectar con SAFA");
	 }
        else  {
        log_info(logger,"me conecto al SAFA");
         runFunction(socketSAFA,"identificarProcesoEnSAFA",1,"DAM");        
         }

     if (socketFM9 == -1) {
      log_error(logger,"no se pudo conectar con FM9");
       }
       else  {
        log_info(logger,"me conecto al FM9");
         runFunction(socketFM9,"identificarProcesoEnFM9",1,"DAM");
         }
      

        if(socketMDJ == -1){
        log_error(logger,"no se pudo conectar con MDJ");
        }
       else {      
        log_info(logger,"me conecto al MDJ");
        runFunction(socketMDJ,"identificarProcesoEnMDJ",1,"DAM");
        runFunction(socketMDJ,"crearArchivo",2,"juanito.txt","200");
        
       }
      

        log_info(logger,"voy a escuchar el puerto %d ",datosConfigDAM->puertoEscucha);

	  createListen(datosConfigDAM->puertoEscucha, NULL,callableRemoteFunctionsCPU, &disconnect, NULL);
       

 	pthread_mutex_init(&mx_main, NULL);
	pthread_mutex_lock(&mx_main);
	pthread_mutex_lock(&mx_main);

	return EXIT_SUCCESS;
}

//FUNCIONES
void disconnect(socket_connection* socketInfo) {
  log_info(logger,"socket n° %d se ha desconectado.\n", socketInfo->socket);
}



void cerrarPrograma() {
    log_info(logger, "Voy a cerrar DAM");
    close_logger();
     dictionary_destroy(callableRemoteFunctionsSAFA);
     dictionary_destroy(callableRemoteFunctionsFM9);
     dictionary_destroy(callableRemoteFunctionsMDJ);
     free(datosConfigDAM);
    pthread_mutex_unlock(&mx_main);
    pthread_mutex_destroy(&mx_main);
}
