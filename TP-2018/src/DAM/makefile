sources = src/DAM.c	 src/libDAM.c
socketLibraries	=  ../sample-socket/socket.c    
pathLibraries	= -lreadline -lcommons -lpthread				
DAM: $(sources)	
	gcc -g -o DAM $(sources) $(socketLibraries) $(pathLibraries)		
clean:	
	rm DAM	
	rm *.o
