#ifndef _MYSOCKET_H_
#define _MYSOCKET_H_

#include "mySocket.h"

int mySocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int listenOn( int ip, int port )
{
	struct sockaddr_in myAddr;
	setAddrStruct( &myAddr, ip, port);

	int listener = mySocket();

	if( listener == -1)
		normalErrorHandling("LISTENER_SOCKET_CREATION_FAILED");

	letReUseAddr(listener);

	if( bind( listener, (struct sockaddr *)&myAddr, sizeof(struct sockaddr) ) == -1)
		normalErrorHandling("LISTENER_BINDING_FAILED");
	if( listen( listener, MAX_BACKLOG) == -1)
		normalErrorHandling("LISTENER_LISTEN_FAILED");


	return listener;
}

int connectTo( int ip, int port )
{
	struct sockaddr_in theirAddr;
	setAddrStruct( &theirAddr, ip, port);

	int clientSocket = mySocket();

	if( clientSocket == -1)
		normalErrorHandling("CLIENT_SOCKET_CREATION_FAILED");

	if( connect( clientSocket, (struct sockaddr *)&theirAddr,  sizeof(struct sockaddr) ) == -1)
		normalErrorHandling("CONNECTION_TO_SERVER_FAILED");

	return clientSocket;
}


int acceptClient( int listener)
{
	struct sockaddr_in their_addr;
	int sin_size = sizeof(struct sockaddr_in);
	int new_socket = accept(listener, (struct sockaddr *)&their_addr, &sin_size);
	if (new_socket == -1 )
		normalErrorHandling("CLIENT_ACCEPT_FAILED");

	return new_socket;
}

int recvWithBasicProtocol( int socket, void ** pBuffer ) // el protocolo basico es que primero recibo un entero de 4 bytes con el tamanio del mensaje a recibir y luego envio el mensaje. Asi : [ SIZE | ACTUAL_MESSAGE ]
{
	int tam, bytes;
	bytes = recv( socket, &tam, sizeof(int32_t), 0);
	if( bytes == -1)
	{
		if( errno == ECONNRESET )
		{
			perror("MSG_SIZE_RECEPTION_FAILED");
			return 0;
		}
		else
			normalErrorHandling("MSG_SIZE_RECEPTION_FAILED");
	}
	else if( bytes == 0)
		return bytes;

	*pBuffer = malloc(tam);

	if( recv( socket, *pBuffer, tam, 0) == -1)
		normalErrorHandling("MSG_CONTENTS_RECEPTION_FAILED");

	return tam;
}

void sendWithBasicProtocol( int socket, void * msg, int size)
{
	tBuffer * pBuffer = newBuffer();
	addToBuffer( pBuffer, (void*)&size, sizeof(int32_t));
	addToBuffer( pBuffer, msg, size);

	send( socket, pBuffer->data, pBuffer->size, 0);

	freeBuffer(pBuffer);
}

void sendToAll( int maxFd, fd_set master, void * msg, int size)
{
	int i;
	for( i = 0; i <= maxFd; i++)
	{
		if( FD_ISSET( i, &master) )
			sendWithBasicProtocol( i, msg, size);
	}
}

void setAddrStruct( struct sockaddr_in * myAddr, int ip, int port)
{
	myAddr->sin_family = AF_INET;
	myAddr->sin_addr.s_addr = ip;
	myAddr->sin_port = port;
	memset(&(myAddr->sin_zero), '\0', sizeof(myAddr->sin_zero) );
}

void letReUseAddr(int sockAddr)
{
	int yes=1;
 	// Olvidemonos del error "Address already in use" [La direccion ya se esta usando]
	if (setsockopt(sockAddr,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
		normalErrorHandling("SETSOCKOPT_FAILED");
}

uint32_t getIntegerIP(char * strIP)
{
	struct in_addr ipContainer; //ipContainer.s_addr ---> uint32

	if( inet_aton( strIP, &ipContainer) )
		return ipContainer.s_addr;
	else
		normalErrorHandling("ERROR OBTENIENDO EL IP ENTERO");

	return 0;
}

void normalErrorHandling(char * errorMsg)
{
	perror(errorMsg);
	exit(1);
}

#endif
