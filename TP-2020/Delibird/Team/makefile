sources = team.c planificador.c libTeam.c serverTeam.c  ../utilCommons/clientSide/clientUtils.c ../utilCommons/serverSide/serverUtils.c
pathLibraries = -lcommons -lpthread

default: Team
all: default

Team: $(sources)
	gcc -g -o Team $(sources) $(pathLibraries)

clean: 
	rm Team
	rm *.o