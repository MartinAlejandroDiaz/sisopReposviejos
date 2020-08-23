sources = src/SAFA.c  src/consolaSAFA.c  src/libSAFA.c src/planificadores.c
socketLibraries = ../sample-socket/socket.c  
pathLibraries = -lreadline -lcommons -lpthread 

SAFA: $(sources)
	gcc -g -o SAFA $(sources) $(socketLibraries) $(pathLibraries)

clean:
	rm SAFA
	rm *.o