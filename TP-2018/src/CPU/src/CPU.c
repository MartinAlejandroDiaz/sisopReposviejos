#include "libCPU.h"
#include <signal.h>

void cerrarPrograma(); 

int main() {
  signal(SIGINT, cerrarPrograma);

  pthread_mutex_init(&m_busqueda, NULL);
  pthread_mutex_init(&m_puedeEjecutar, NULL);
  sem_init(&sem_esperaAbrir, 0, 0);
  sem_init(&sem_esperaEjecucion, 0, 0);
  sem_init(&sem_esperaHilo, 0, 0);
  configure_loggerCPU();
  datosCPU = read_and_log_configCPU("CPU.config");

  callableRemoteFunctionsCPU = dictionary_create();
  dictionary_put(callableRemoteFunctionsCPU,"ejecutarCPU",&permisoConcedidoParaEjecutar);
  dictionary_put(callableRemoteFunctionsCPU,"establecerQuantumYID",&establecerQuantumYID);
  dictionary_put(callableRemoteFunctionsCPU,"SAFA_CPU_pausarPlanificacion",&pausarPlanificacion);
  dictionary_put(callableRemoteFunctionsCPU,"SAFA_CPU_continuarPlanificacion",&continuarPlanificacion);
  dictionary_put(callableRemoteFunctionsCPU,"SAFA_CPU_continuarEjecucionFlush",&ejecucionFlush);
  dictionary_put(callableRemoteFunctionsCPU, "SAFA_CPU_continuarEjecucionAbrir", &ejecucionAbrir);
  dictionary_put(callableRemoteFunctionsCPU, "CPU_DAM_continuacionExistenciaAbrir", &ejecucionAbrirExistencia);
  dictionary_put(callableRemoteFunctionsCPU, "SAFA_CPU_continuarEjecucionAsignar", &ejecucionAsignar);
  dictionary_put(callableRemoteFunctionsCPU, "SAFA_CPU_continuarEjecucionWait", &ejecucionWait);
  dictionary_put(callableRemoteFunctionsCPU, "SAFA_CPU_continuarEjecucionClose", &ejecucionClose);
  dictionary_put(callableRemoteFunctionsCPU, "FM9_CPU_resultadoDeClose", &finalizacionClose);

  socketDAM = connectServer(datosCPU->ipD, datosCPU->puertoD, callableRemoteFunctionsCPU, &disconnect, NULL);

  intentandoConexionConDAM(socketDAM);

  socketSAFA = connectServer(datosCPU->ipS, datosCPU->puertoS, callableRemoteFunctionsCPU, &disconnect, NULL);
  //CUANDO ME CONECTO AL SAFA LE DIGO QUE SOY UN PROCESO CPU
  runFunction(socketSAFA,"identificarNuevaConexion",1,"CPU");
  
  intentandoConexionConSAFA(socketSAFA);

  socketFM9 = connectServer(datosCPU->ipF, datosCPU->puertoF, callableRemoteFunctionsCPU, &disconnect, NULL);

  intentandoConexionConFM9(socketFM9);
  
  pthread_mutex_init(&m_main, NULL);
	pthread_mutex_lock(&m_main);
	pthread_mutex_lock(&m_main); 

  return 0;

}

void cerrarPrograma() {
  disconnect();
  close_loggerCPU();
  free(datosCPU->ipD);
  free(datosCPU->ipS);
  free(datosCPU->ipF);
  free(datosCPU);

  dictionary_destroy(callableRemoteFunctionsCPU);
  pthread_mutex_destroy(&m_busqueda);
  pthread_mutex_destroy(&m_puedeEjecutar);
  pthread_mutex_unlock(&m_main);
  pthread_mutex_destroy(&m_main);
}
