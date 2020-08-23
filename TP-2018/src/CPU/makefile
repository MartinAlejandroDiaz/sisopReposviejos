sources = src/CPU.c  src/libCPU.c  
sharedLibraries = ../sample-socket/socket.c 
pathLibraries = -lreadline -lcommons -lpthread 

CPU: $(sources)
	gcc -g -o CPU $(sources) $(sharedLibraries) $(pathLibraries)

clean:
	rm CPU
	rm *.o
