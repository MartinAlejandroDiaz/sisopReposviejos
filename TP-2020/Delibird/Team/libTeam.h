#ifndef LIBTEAM_H_
#define LIBTEAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include "../utilCommons/clientSide/clientUtils.h"
#include "../utilCommons/serverSide/serverUtils.h"
#include "../utilCommons/compartidos.h"
#include "serverTeam.h"

#define GET_POKEMON "GET_POKEMON"

typedef enum {NEW, READY, EXECUTE, BLOCKED,FINISHED} t_estados;

// Estructuras Definidas
typedef struct {
	char** posiciones_entrenadores; // POSICIONES_ENTRENADORES
	char** pokemon_entrenadores; // POKEMON_ENTRENADORES
	char** objetivos_entrenadores; // OBJETIVOS_ENTRENADORES
	int tiempo_reconexion; // TIEMPO_RECONEXION
	int retardo_ciclo_cpu; // RETARDO_CPU
	char* algoritmo_planificacion; // ALGORITMO_PLANIFICACION
	int quantum; // QUANTUM
	double alpha; // ALPHA
	char* ip_team; // IP_PROPIO
	char* puerto_team; // PUERTO_PROPIO
	char* ip_broker; // IP_BROKER
	char* puerto_broker; // PUERTO_BROKER
	double estimacion_inicial;
	char* log_file; // LOG_FILE
	t_config* configuracion;
}t_config_team;

typedef struct {
	char* nombre;
	int posicion_x;
	int posicion_y;
} t_pokemon;

typedef struct{
	int cantidad_pokemon;
	char* nombre_pokemon;
} t_node_pokemon;

typedef struct{
	int identificador;
	int posicion_x;
	int posicion_y;
} t_entrenador_intercambio;

typedef struct{
	int identificador;
	int cantidad_maxima_a_capturar;
	int posicion_x;
	int posicion_y;
	t_list* pokemon_obtenidos;
	t_list* pokemon_objetivos;
	t_list* pokemon_no_necesitados;
	int quantumOtorgado;
	int contadorIntercambio;
	double estimadorAnterior;
	double estimador;
	int cantidadCPUEjecutado;
	int totalCPUEjecutado;
	bool objetivosCumplidos;
	bool estaOcupado;
	bool necesitaContinuar;
	t_estados estado;
	sem_t puedeEjecutar;
	t_pokemon* pokemonCaptura;
	t_entrenador_intercambio* entrenadorIntercambio;
	char* pokemonACambiar;
	t_msg_1* catchEnviado;
} t_entrenador;

typedef struct{
	uint32_t idMensajeCorrelativo;
	char* confirmacion;
} t_msg_CAUP;

// Variables Globales
t_log* logger;
t_config* config;
t_config_team* datosConfigTeam;
t_entrenador* contextoEntrenador;
int contadorCC; //Contador de Cambio de Contexto
int contadorEntrenadoresEnTeam;
int deadlockResueltos;

// Cola de Estados
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExecute;
t_queue* colaBlocked;
t_queue* colaFinished;

// Semaforos
// Mutex
pthread_mutex_t m_colaNew;
pthread_mutex_t m_colaReady;
pthread_mutex_t m_colaExecute;
pthread_mutex_t m_colaBlocked;
pthread_mutex_t m_colaFinished;
pthread_mutex_t m_busqueda;

// Estos mutex tendrian como idea intentar agregar un Pokemon a Capturar en orden
// y posibilidad que llegasen múltiples mensajes sobre el mismo
pthread_mutex_t m_mensajeLocalized;
pthread_mutex_t m_mensajeAppeared;
pthread_mutex_t soloActualizaUno;

// Contador
sem_t puedeSeleccionarEntrenador;
sem_t pokemonsACapturar;
sem_t cantidadEntrenadoresEnReady;
sem_t puedeEncolarEntrenador;
sem_t cantidadEntrenadoresEnTeam;
sem_t entrenadoresEnDeadlock;
sem_t puedeIniciarAlgoritmo;
sem_t deadlocksRealizados;
sem_t esperarCapturas;

t_list* objetivosGlobalesTeam;
t_list* listaPokemonRequeridos;
t_list* listaPokemonDeRespaldo;
t_list* pokemonesYaRecibidos;
int identificadorEntrenador;
bool puede;

pthread_t thread;

// Funciones Definidas
void creacion_logger();
t_config_team* read_and_log_config();
void liberar_infoConfig();
void establecerCaracteristicasDeLosEntrenadores();
int retornarCantidadDeEntrenadores(char**);
void reintentarConexionConBroker();
void establecerLaCantidadMaximaQuePuedeCapturar(t_entrenador*);
void establecerLosPokemonObjetivoDelTeam(t_entrenador*);
void removerDeLaListaDeObjetivos(t_entrenador*, char*);
void planificarEntrenador(void*);
void moverEntrenadorSinRestriccion(int*, int, int*);
void moverEntrenadorLimitado(int*, int, int*, double*, int*);
void moverEntrenador(t_entrenador*);
void enviarMensajeSobreSentenciaGetPokemon();
void enviarMensajeGetPokemon(void*);
int enviarMensajeParaCapturarPokemon(t_entrenador*);
t_buffer* serializar_sobre_nombre_pokemon(char*);
int enviar_paquete_GET(t_buffer*);
t_buffer* serializar_msg_CATP(t_pokemon* pokemonCatch);
int enviar_paquete_CATCH(t_buffer* buffer, t_entrenador* entrenador);
t_msg_CAUP* deserializar_msg_CAUP (t_buffer*);
void agregarPokemonComoObtenido(t_entrenador*, char*);
void validarSiSePudoCapturar(t_msg_CAUP* caughtPokemon);
bool seMueveDeFormaLimitada();
bool seEstaEjecutandoElSJF();
void calcularNuevoEstimadorSiEsSJF(t_entrenador*);
double calcularEstimador(t_entrenador*);
void reducirElEstimadorDelEntrenador(double*);

t_pokemon* retornarEstructuraDePokemon(char*, uint32_t, uint32_t);
void eliminarLosPokemonsDeRespaldo(char*);
t_msg_1* retornarMensajeCatch(t_entrenador*, uint32_t);
void iterarAccionesAlCapturarConExito(t_entrenador*);

bool tienenLaMismaEstimacion(double, double);

void actualizarObjetivosGlobalesDelTeam(char* pokemonAtrapado);
void liberarMensajeSobreCatch(t_entrenador* entrenador);

int distanciaPokemonEntrenador(t_entrenador*, t_pokemon*);
int distanciaEntreEntrenadores(t_entrenador*);

bool loNecesitoGlobalmente(char* nombrePokemon);
bool necesitoAgregarPokemon(char* pokemon);

void establecerMetricasAlFinalizarTeam();
void validacionSobreCambioDeContextoDeEntrenador(t_entrenador* entrenadorAEjecutar);
int calcularCantidadTotalDeCiclosDeCPU();
void mostrarCPUEjecutadoPorEntrenador();

void liberarEstructuraEntrenador(t_entrenador*);
void liberarPokemonObjetivos(t_node_pokemon*);

// Funcion a eliminar despues
void imprimirPosiciones(t_entrenador* entrenador);

#endif /* LIBTEAM_H_ */
