#ifndef LIB_SAFA_H
#define LIB_SAFA_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include  "../../sample-socket/socket.h"

//ESTRUCTURAS
typedef enum {NEW, READY, BLOCKED, RUNNING, FINISHED } status_t;

typedef struct {
	int puerto;
	char* algoritmoPlanif;
	int quantum;
	int gradoMultiprog;
	int retardo;
} t_config_SAFA;

typedef struct{
	int id;
	char* rutaScript;
	int PC; //program counter
	int flagInicializado;
	t_list* archivosAbiertos;
	status_t status;
	int quantumFaltante; //sentencias que le faltan ejecutar para terminar su quantum ( para VRR )
	int cantSentEsperadasEnNew; //para las metricas
	int cantIOs; //cant de entradas y salidas ( para IOBF )
}DTB;

typedef struct{
	int socket; //socket de la conexion con una CPU determinada
	int id;
	sem_t aviso; //cuando CPU termina con un GDT me avisa mediante este semaforo
}CPU;

typedef struct{
	char* nombre;
	int valor;
	t_list* GDTsEsperandoRecurso;
}recurso;

//--------------------------------------//



//VARIABLES GLOBALES
int cantSentenciasEjecutadas;
int cantSentConDiego;
int generadorDeIdsCPU;
int idCpuABuscar;
char* nombreRecursoABuscar;
int idCPU;
//semaforos
sem_t puedeEntrarAlSistema;
sem_t cantProcesosEnReady;
sem_t cantProcesosEnNew;
pthread_mutex_t m_puedePlanificar;
//mutex para las colas y listas
pthread_mutex_t m_colaReady;
pthread_mutex_t m_colaBloqueados;
pthread_mutex_t m_colaFinalizados;
pthread_mutex_t m_colaNew;
pthread_mutex_t m_listaEjecutando;
pthread_mutex_t m_listaDeRecursos;

pthread_mutex_t m_busqueda;

pthread_mutex_t m_recurso;

pthread_mutex_t m_verificacion;

pthread_mutex_t m_cantSent;
pthread_mutex_t m_cantDiego;

t_log* logger;
t_config_SAFA* datosConfigSAFA;
pthread_mutex_t mx_main;
t_dictionary* fns;

t_queue* colaNew;
t_queue* colaReady;
t_queue* colaBloqueados;
t_queue* colaFinalizados;
t_list* listaEjecutando;

t_list* hilos;

t_list* listaCPUs; //lista de CPUs

t_list* listaDeRecursos;

char* archAVerificar;

//booleanos para manejar el estado corrupto
bool estadoCorrupto; 
bool damConectado;
bool unCpuConectado;

//para las metricas
int cantGDTsEjecutados;
int cantSentEsperadasASumar;

//--------------------------------------------//

//FUNCIONES UTILES
void encolarDTB(t_queue* c, DTB* d, pthread_mutex_t m);
DTB* desencolarDTB(t_queue* c, pthread_mutex_t m);
DTB* get_and_remove_DTB_by_ID( t_list * lista, int id );
DTB * quitarDTBDeSuListaActual(int idDTB);

CPU* buscarCPU(int id);
bool closureIdCPU(CPU* cpu);

DTB* buscarDTB(t_list* lista,int id);

DTB* buscarDTBEnElSistema(int idGDT);

recurso* buscarRecurso(char* nombre);
bool closureBusquedaRecurso(void* r);

void destruirRecurso(recurso* r);

void eliminarRecurso(recurso* r);
void crearRecurso(char* nombre, int valor);

bool condicionArchivoAbierto(void* arch);

//LOGS
void configure_logger();
void close_logger();
t_config_SAFA * read_and_log_config(char*);

//--------------------------------------------//


//PROTOTIPOS
void cerrarPrograma();

//--------------------------------------------//

void finalizarDTB(DTB* dtb);

//CallableRemoteFunctions
void newConnection(socket_connection* socketInfo, char** msg);
void finalizacionProcesamientoCPU(socket_connection* socketInfo, char** msg);
void avisoDeDamDeResultadoDTB(socket_connection* socketInfo, char** msg);
void desbloquearDTB(socket_connection* connection, char** msgs);
void pasarDTBAExit(socket_connection* connection, char** msgs);
void waitRecurso(socket_connection* socketInfo, char** msg);
void signalRecurso(socket_connection* socketInfo, char** msg);
void verificarEstadoArchivo(socket_connection*, char**);
//fin CallableRemoteFunctions

void aumentarCantSentenciasEsperadasEnNew(DTB* dtb);
//Funciones Para La Funcion De Status
void mostrarInformacionDTB(DTB*);
void* statusDTB(int);
void buscarDTBEnColasMostrandoInfo(t_queue*);
static inline char *stringFromState(status_t);
#endif
