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

void disconnect(){
  printf("se desconecto al servidor :(");
}

int main() {
  /* Agregamos las funciones que podrán ser llamadas por mis conexiones */
	t_dictionary * callableRemoteFunctions = dictionary_create();

  int socket = connectServer("127.0.0.1", 8080, callableRemoteFunctions, &disconnect, "TE SALUDO CD");

  if (socket == -1) {
    printf("no me pude conectar");
    return -1;
  }

  sleep(4);
  // este metodo no existe en el protocolo del servidor, por eso va a putear el server
  runFunction(socket, "hola", 0);

  sleep(4);
  runFunction(socket, "write", 2, "hola", "mundo");

}
