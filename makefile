.PHONY: compile server player

compile compileMe: gameServer.o gamePlayer.o
	gcc -o runServer gameServer.o -g -Wall -lm
	gcc -o runPlayer gamePlayer.o -g -Wall -lm
gameServer.o: gameServer.c gameServer.h
	gcc -c gameServer.c -g -Wall -lm
gamePlayer.o: gamePlayer.c gamePlayer.h
	gcc -c gamePlayer.c -g -Wall -lm

server: compileMe
	./runServer

player: compileMe
	./runPlayer

clean:
	rm -f *.o
	rm -f runServer
	rm -f runPlayer
