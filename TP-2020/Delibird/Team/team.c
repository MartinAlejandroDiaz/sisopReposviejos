#include "planificador.h"
#include "libTeam.h"

int main() {

	pthread_t hiloReConexionBroker;
	pthread_t hiloSeleccionEntrenador;
	pthread_t hiloDeadlock;

	identificadorEntrenador = 1;
	contadorCC = 0;
	contextoEntrenador = NULL;
	contadorEntrenadoresEnTeam = 0;
	deadlockResueltos = 0;
	puede = true;

	read_and_log_config();
	creacion_logger();

	 //Inicializacion de Colas
	colaNew = queue_create();
	colaReady = queue_create();
	colaExecute = queue_create();
	colaBlocked = queue_create();
	colaFinished = queue_create();

	// Inicializacion de Semaforos Mutex
	pthread_mutex_init(&m_colaNew, NULL);
	pthread_mutex_init(&m_colaReady, NULL);
	pthread_mutex_init(&m_colaExecute, NULL);
	pthread_mutex_init(&m_colaBlocked, NULL);
	pthread_mutex_init(&m_colaFinished, NULL);
	pthread_mutex_init(&m_busqueda, NULL);
	pthread_mutex_init(&m_mensajeAppeared, NULL);
	pthread_mutex_init(&m_mensajeLocalized, NULL);
	pthread_mutex_init(&soloActualizaUno, NULL);

	// Inicializacion de Semaforos Contador
	sem_init(&puedeSeleccionarEntrenador, 0, 0);
	sem_init(&puedeEncolarEntrenador, 0, 1);
	sem_init(&cantidadEntrenadoresEnReady, 0, 0);
	sem_init(&pokemonsACapturar, 0, 0);
	sem_init(&cantidadEntrenadoresEnTeam, 0, 0);
	sem_init(&entrenadoresEnDeadlock, 0, 0);
	sem_init(&puedeIniciarAlgoritmo, 0, 0);
	sem_init(&deadlocksRealizados, 0, 0);
	sem_init(&esperarCapturas, 0, 0);

	// Lista Global del TEAM
	objetivosGlobalesTeam = list_create();
	listaPokemonRequeridos = list_create();
	listaPokemonDeRespaldo = list_create();
	pokemonesYaRecibidos = list_create();

	//Creacion de Planificadores
	pthread_create(&hiloSeleccionEntrenador, NULL, (void*) &planificadorSeleccionEntrenador, NULL);
	pthread_create(&hiloDeadlock, NULL, (void*) &algoritmoDeteccionDeadlock, NULL);
	crearHiloSobreAlgoritmoDePlanificacion();

	pthread_detach(hiloDeadlock);
	pthread_detach(hiloSeleccionEntrenador);

	//Funcion para crear a los entrenadores y guardar posicion,pokemon,objetivos por separado
	establecerCaracteristicasDeLosEntrenadores();

	//Creacion de Hilo para dar Finalizacion a las Capturas
	pthread_t hiloFinCapturas;
	pthread_create(&hiloFinCapturas, NULL, (void*) &seFinalizaronLasCapturas, NULL);
	pthread_detach(hiloFinCapturas);

	// Creacion de Suscripciones
	pthread_t hiloAP, hiloCP, hiloLP, hiloMetricas;

	pthread_create(&hiloAP, NULL, (void*) &suscribir_proceso_team, "APPEARED_POKEMON");
	pthread_detach(hiloAP);

	pthread_create(&hiloCP, NULL, (void*) &suscribir_proceso_team, "CAUGHT_POKEMON");
	pthread_detach(hiloCP);

	pthread_create(&hiloLP, NULL, (void*) &suscribir_proceso_team, "LOCALIZED_POKEMON");
	pthread_detach(hiloLP);

	pthread_create(&hiloMetricas, NULL, (void*) &establecerMetricasAlFinalizarTeam, NULL);
	pthread_detach(hiloMetricas);

	enviarMensajeSobreSentenciaGetPokemon();

	iniciar_servidor_team();

	config_destroy(datosConfigTeam->configuracion);
	free(datosConfigTeam);
	log_destroy(logger);

	return 0;
}
