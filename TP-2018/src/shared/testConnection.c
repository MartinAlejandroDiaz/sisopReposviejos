#include "testConnection.h"

void questionWithBasicProtocol(int socket, void * question, int sizeOfQuestion, void *(*showAnswer)(void * answer))
{
	sendWithBasicProtocol(socket, question, sizeOfQuestion);

	puts("QUESTION_SENT");

	void * answer;
	int sizeOfAnswer = recvWithBasicProtocol(socket, &answer);

	puts("ANSWER_RECEIVED");

	if( sizeOfAnswer != 0)
		showAnswer(answer);
	else
	{
		puts("CONNECTION_TEST_FAILED");
		exit(1);
	}
}

void answerWithBasicProtocol(int socket, void * answer, int sizeOfAnswer, void *(*showQuestion)(void * question))
{
	void * question = NULL;
	int sizeOfQuestion = recvWithBasicProtocol(socket, &question);

	puts("QUESTION_RECEIVED");

	if( sizeOfQuestion != 0)
		showQuestion(question);
	else
	{
		puts("CONNECTION_TEST_FAILED");
		exit(1);
	}

	sendWithBasicProtocol(socket, answer, sizeOfAnswer);

	puts("ANSWER_SENT");
}