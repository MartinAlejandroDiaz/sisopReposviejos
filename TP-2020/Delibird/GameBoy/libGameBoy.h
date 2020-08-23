#ifndef GAMEBOY_LIBGAMEBOY_H_
#define GAMEBOY_LIBGAMEBOY_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include "../utilCommons/clientSide/clientUtils.h"

// Estructuras Definidas

typedef struct {
	char* ipBroker;
	char* puertoBroker;
	char* ipTeam;
	char* puertoTeam;
	char* ipGameCard;
	char* puertoGameCard;
	t_config* configuracion;
}t_config_gameBoy;

typedef struct {
	int socket;
	unsigned int tiempo;
}t_suscripto;

// Variables Globales
t_log* logger;
t_config* config;
t_config_gameBoy* datosConfigGameBoy;

typedef enum {BROKER, TEAM, GAMECARD, SUSCRIPTOR, SALIR} palabra;

// Funciones Definidas
void creacion_logger();
t_config_gameBoy* read_and_log_config();
void ejecutarParametrosSobreScript(int, char**);
void iniciarModoSuscripcion(char*, char*);
void recibirMensajesPorSuscripcion(t_suscripto*);
void liberar_infoConfig();
int retornar_header_para_paquete(char*, char*);
int retornar_socket_sobre_proceso(char*);
int retornar_por_ok_o_fail(char*);
void recibir_mensaje(int);
void server_procesar_mensaje_GB(t_socketPaquete*);
void loggearSobreEnvioAlBroker(char*, char*, char*);
void recibirConfirmacionBroker(char *proceso, int socketConexion);
palabra transformarStringToEnum(char* palReservada);

// Envio de Mensajes
// Armado Buffer
// Con 7 Parametros
void enviar_sobre_NP(char*, char*, char*, char*, char*, char*, char*);
// Con 6 Parametros
void enviar_sobre_CATP_o_AP(char*, char*, char*, char*, char*, char*);
void enviar_a_GameCard_sobre_NP(char*, char*, char*, char*, char*, char*);
// Con 4 Parametros
void enviar_a_Broker_sobre_CAUP(char*, char*, char*, char*);
void enviar_sobre_GP(char*, char*, char*, char*);

// Serializacion
t_buffer* serializar_suscripcion(char*);
t_buffer* serializar_sobre_dos_parametros(char*, uint32_t);
t_buffer* serializar_sobre_tres_parametros(uint32_t, uint32_t);
t_buffer* serializar_sobre_cuatro_parametros(char*, uint32_t, uint32_t, uint32_t);
t_buffer* serializar_sobre_cinco_parametros(char*, uint32_t, uint32_t, uint32_t, uint32_t);

// Armado Paquete
void enviar_paquete(t_buffer*, char*, char*);
int enviar_paquete_GB(t_buffer*, char*);

#endif
