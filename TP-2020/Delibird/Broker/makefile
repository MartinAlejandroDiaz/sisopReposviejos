sources = broker.c libBroker.c ../utilCommons/clientSide/clientUtils.c ../utilCommons/serverSide/serverUtils.c serverBroker.c memoria.c
pathLibraries =  -lcommons -lpthread -lm

default: Broker
all: default

Broker: $(sources)
	gcc -g -o Broker $(sources) $(pathLibraries)

clean: 
	rm Broker
	rm *.o