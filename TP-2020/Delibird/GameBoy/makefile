sources = gameboy.c libGameBoy.c  ../utilCommons/clientSide/clientUtils.c
pathLibraries = -lreadline -lcommons -lpthread

default: GameBoy
all: default

GameBoy: $(sources)
	gcc -g -o gameboy $(sources) $(pathLibraries)

clean: 
	rm GameBoy
	rm *.o
