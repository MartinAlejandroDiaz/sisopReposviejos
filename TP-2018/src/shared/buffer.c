#include "buffer.h"

tBuffer * newBuffer(void)
{
	tBuffer * pBuffer = malloc(sizeof(tBuffer));

	pBuffer->data = NULL;
	pBuffer->size = 0;
	pBuffer->readingPos = 0;

	return pBuffer;
}

void addToBuffer(  tBuffer * pBuffer, void * newData, int sizeofData )
{
	pBuffer->data = realloc( pBuffer->data, pBuffer->size +  sizeofData );
	memcpy(pBuffer->data + pBuffer->size, newData, sizeofData);
	pBuffer->size += sizeofData;
}

void addIntToBuffer( tBuffer * pBuffer, int value)
{
	int auxValue = value;
	addToBuffer(pBuffer, (void*)&auxValue, sizeof(int));
}

void addStringToBuffer( tBuffer * pBuffer, char * str)
{
	addToBuffer(pBuffer, str, strlen(str) + 1);
}

tBuffer * makeBuffer(void * data, int size)
{
	tBuffer * pMadeBuffer = newBuffer();
	addToBuffer(pMadeBuffer, data, size);
	return pMadeBuffer;
}

void * readFromBuffer( tBuffer * pBuffer, int size)
{
	if(pBuffer->readingPos + size > pBuffer->size)
	{
		puts("Error: Tried to read beyond a tBuffer's size");
		exit(1);
	}
	void * readData = malloc(size);
	memcpy(readData, pBuffer->data + pBuffer->readingPos, size);

	pBuffer->readingPos += size;

	return readData;
}

int readIntFromBuffer( tBuffer * pBuffer)
{
	int * pValue = (int*)readFromBuffer(pBuffer, sizeof(int));
	int value = *pValue;
	free(pValue);
	return value;
}

char * readStringFromBuffer( tBuffer * pBuffer)
{
	int stringLength = strlen(pBuffer->data + pBuffer->readingPos);
	char * str = (char *)readFromBuffer(pBuffer, stringLength + 1);
	return str;
}

void freeBuffer(tBuffer * pBuffer)
{
	free(pBuffer->data);
	free(pBuffer);
}
