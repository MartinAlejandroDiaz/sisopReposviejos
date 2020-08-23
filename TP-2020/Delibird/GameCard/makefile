sources = gameCard.c libGameCard.c serverGameCard.c  ../utilCommons/clientSide/clientUtils.c ../utilCommons/serverSide/serverUtils.c
pathLibraries = -lcommons -lpthread

default: GameCard
all: default

GameCard: $(sources)
	gcc -g -o GameCard $(sources) $(pathLibraries)

clean: 
	rm GameCard
	rm *.o