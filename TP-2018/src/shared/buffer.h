#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
	{
		int size;
		void * data;
		int readingPos;
	}
	tBuffer;

tBuffer * newBuffer(void);
// crea y devuelve un puntero al buffer

tBuffer * makeBuffer(void * data, int size);
// crea un buffer a partir de un puntero ya reservado con size bytes

void addToBuffer(  tBuffer * pBuffer, void * newData, int sizeofData );
// toma el puntero al buffer y le mete el contenido que ocupa sizeofData a partir del puntero newData

void addIntToBuffer( tBuffer * pBuffer, int value);
// agrega un entero al final del buffer

void addStringToBuffer( tBuffer * pBuffer, char * str);
// agrega una cadena al final del buffer

void * readFromBuffer( tBuffer * pBuffer, int size);
// devuelve un puntero con una copia de la informacion de size bytes leidos desde la posicion pos en el buffer

int readIntFromBuffer( tBuffer * pBuffer);
// devuelve un int de la posicion pos del buffer

char * readStringFromBuffer( tBuffer * pBuffer);

void freeBuffer(tBuffer * pBuffer);
//libera y destruye todo el buffer con todo lo que le meti

#endif

/*
	Ejemplo:
	tBuffer * pBuffer = newBuffer();
	addToBuffer( pBuffer, 325, sizeof(325));
	addToBuffer( pBuffer, 'a', sizeof('a'));
	addToBuffer( pBuffer, "hola loco", strlen(hola loco) + 1);

	Con eso consegui un buffer con todos esos datos puestos uno al lado del otro.
	325|'a'|"hola loco"
	Ese chorro de bytes se guarda en el puntero void * data de la estructura a la que apunta pBuffer.
	O sea que para acceder a la data tengo que escribir pBuffer->data. Ejemplo:

		void * chorroDeBytes = pBuffer->data;

	Cuando no use mas el buffer, lo libero:

	freeBuffer(pBuffer);
*/
