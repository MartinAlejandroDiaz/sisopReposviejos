#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define MAX_BACKLOG 10

int mySocket();
//devuelve un socket creado en modo default

int connectTo( int ip, int port );
//devuelve un socket conectado al ip y puerto indicados

int listenOn( int ip, int port );
//devuelve el socket "escucha" listo para aceptar conexiones que busquen ese IP y ese puerto

int acceptClient( int listener);
//acepta conexion entrante y devuelve el socket con la nueva conexion

void setAddrStruct( struct sockaddr_in * myAddr, int ip, int port);
//completa la estructura con la direccion completa de puerto e IP

int recvWithBasicProtocol( int socket, void ** pBuffer );
//recibe datos del socket conectado de la forma [	SIZE | DATOS ] y devuelve el tamanio de los datos recibidos

void sendWithBasicProtocol( int socket, void * msg, int size);
//envia datos al socket conectado de la forma [ SIZE | DATOS ]

void sendToAll( int maxFd, fd_set master, void * msg, int size);
//envia a todos los sockets del masterSet el mensaje en formato de protocolo basico

void letReUseAddr(int sockAddr);
//evita que salte un error molesto de "puerto todavia en uso"

uint32_t getIntegerIP(char * strIP);
//devuelve la direccion IP como int

void normalErrorHandling(char * errorMsg);
//muestra por pantalla el error y detiene el programa

#endif
