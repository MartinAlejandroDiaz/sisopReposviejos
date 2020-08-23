#ifndef SERVERTEAM_H_
#define SERVERTEAM_H_


#include <commons/collections/list.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include "libTeam.h"
#include "../utilCommons/compartidos.h"
#include "../utilCommons/clientSide/clientUtils.h"
#include "../utilCommons/serverSide/serverUtils.h"

pthread_t thread;

void iniciar_servidor_team();
void server_aceptar_clientes_team(int socket_servidor);
void* server_atender_cliente_team(void* socket);
void server_procesar_mensaje_team(t_socketPaquete*);
uint8_t recibir_cod_op_team(int socket);
int server_bind_listen_team(char* ip, char* puerto);
void suscribir_proceso_team(char* colaMensaje);
int enviar_paquete_team(t_buffer* buffer, char* colaMensaje, char* accion);
void recibirMensajesSuscripcion(void* socket);
int devolverEncabezadoMensajeTeam(char* colaMensaje, char* accion);
void requieroGuardarNombrePokemon(char* nombrePokemon);

bool estePokemonYaLoRecibiPorAlgunMensaje(char* nombrePokemon);
void guardarMensajeLocalized(t_msg_5* localizedPokemon);

t_msg_1* deserializar_msg_AP(t_buffer* buffer);
void inclusionPokemonACapturar(char* nombre, uint32_t posicion_x, uint32_t posicion_y);

char* devolverResultadoComoString(uint32_t);
void actualizarObjetivosDelEntrenador(t_list*, char*, int);
void mandarAPlanificarUnPokemonAuxiliar(char*);

void liberarCantidadDeCoordenadas(t_msg_5*);
int retornarCantidadTotalDeCaptura();
void seFinalizaronLasCapturas();
//void encolarEntrenador(t_queue*, t_entrenador*, pthread_mutex_t);
//t_entrenador* desencolarEntrenadorCercano(t_queue*, t_entrenador*, pthread_mutex_t);

#endif /* SERVERTEAM_H_ */
