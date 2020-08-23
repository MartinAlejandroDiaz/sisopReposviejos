sources = src/MDJ.c  src/libMDJ.c   src/consolaMDJ.c  src/interfaz.c
sharedLibraries = ../sample-socket/socket.c
pathLibraries = -lreadline -lcommons -lpthread -lssl -lcrypto 

MDJ: $(sources)
	gcc -g -o MDJ $(sources) $(sharedLibraries) $(pathLibraries)

clean:
	rm MDJ  *.log