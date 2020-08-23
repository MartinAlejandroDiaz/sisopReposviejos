#ifndef TEST_CONNECTION_H
#define TEST_CONNECTION_H

#include <stdio.h>
#include "mySocket.h"

void questionWithBasicProtocol(int socket, void * question, int sizeOfQuestion, void *(*showAnswer)(void * answer));
void answerWithBasicProtocol(int socket, void * answer, int sizeOfAnswer, void *(*showQuestion)(void * question));

#endif
