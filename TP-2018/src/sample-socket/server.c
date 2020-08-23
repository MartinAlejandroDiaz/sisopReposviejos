#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/process.h>
#include <commons/txt.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "socket.h"

pthread_mutex_t mx_main;

t_dictionary * callableRemoteFunctions;	/* Diccionario de funciones que pueden ser llamadas */


void connectionNew(socket_connection* socketInfo) {
	printf("Se ha conectado un usuario con ip %s en socket n°%d\n", socketInfo->ip, socketInfo->socket);
  printf("%s\n",socketInfo->data);
}

void startProcess(socket_connection * connection, char ** args){
  printf("start\n");
}

void readIns(socket_connection * connection, char ** args){
  printf("read\n");
}

void writeIns(socket_connection * connection, char ** args){
  printf("write\n");

  printf("%s\n", args[0]);
  printf("%s\n", args[1]);

}

void disconnect(socket_connection* socketInfo) {
  printf("CPU (socket n°%d) se ha desconectado.\n", socketInfo->socket);
}

void main() {
  callableRemoteFunctions = dictionary_create();
  //cpu
  dictionary_put(callableRemoteFunctions, "startProcess", &startProcess);
  dictionary_put(callableRemoteFunctions, "read", &readIns);
  dictionary_put(callableRemoteFunctions, "write", &writeIns);

  createListen(8080, &connectionNew, callableRemoteFunctions, &disconnect, NULL);

  printf("Escuchando puerto 8080");

  // bloqueo el thread, dejando la coneccion abierta
  pthread_mutex_init(&mx_main, NULL);
	pthread_mutex_lock(&mx_main);
	pthread_mutex_lock(&mx_main);

}
