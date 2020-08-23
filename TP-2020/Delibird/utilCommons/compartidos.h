#ifndef UTILCOMMONS_COMPARTIDOS_H_
#define UTILCOMMONS_COMPARTIDOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include<commons/collections/list.h>

// Enumeradores
typedef enum {
	SERVER_MENSAJE=1,
	broker_NP,
	broker_AP,
	broker_CATP,
	broker_CAUP,
	broker_GP,
	broker_LP,
	team_AP,
	gamecard_NP,
	gamecard_CP,
	gamecard_GP,
	modo_suscriptor,
	subscripcion_inicial,
	confirmacion_recepcion
} op_code;

// Estructuras Definidas
// Para que las Conozcan Los Procesos Servidor

typedef struct {
	uint32_t pokemon_length;
	char* pokemon;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t idMensaje;
} __attribute__((packed)) t_msg_1; // APPEARED CATCH

typedef struct {
	uint32_t pokemon_length;
	char* pokemon;
	uint32_t posicionX;
	uint32_t posicionY;
	uint32_t cantidad;
	uint32_t idMensaje;
} __attribute__((packed)) t_msg_2; // NEW

typedef struct {
	uint32_t pokemon_length;
	char* pokemon;
	uint32_t idMensaje;
} __attribute__((packed)) t_msg_3; // GET

typedef struct {
	uint32_t pokemon_length;
	char* pokemon;
} __attribute__((packed)) t_msg_3b; // GET

typedef struct {
	uint32_t idMensaje;
	uint32_t confirmacion;
} __attribute__((packed)) t_msg_4; // CAUGHT

typedef struct {
	uint32_t idMensaje;
	uint32_t pokemon_length;
	char* pokemon;
	uint32_t cantidad_coordenadas;
	t_list *coordenadas;
} __attribute__((packed)) t_msg_5; // LOCALIZED

typedef struct {
	uint32_t colaMensaje_length;
	char* colaMensaje;
	uint32_t ip_length;
	char* ip;
	uint32_t puerto_length;
	char* puerto;
} __attribute__((packed)) t_sol_suscripcion;

typedef struct{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct{
	uint8_t codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	int socket;
	t_paquete* paquete;
} t_socketPaquete;

#endif /* UTILCOMMONS_COMPARTIDOS_H_ */
