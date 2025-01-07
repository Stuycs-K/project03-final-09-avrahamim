.PHONY: compile server player

compile compileMe: gameServer.o gamePlayer.o pipe_networking.o
	@gcc -o run_server game_server.o pipe_networking.o -g -Wall -lm
	@gcc -o run_player game_player.o pipe_networking.o -g -Wall -lm
gameServer.o: game_server.c game_server.h
	@gcc -c game_server.c -g -Wall -lm
gamePlayer.o: game_player.c game_player.h
	@gcc -c game_player.c -g -Wall -lm
pipe_networking.o: pipe_networking.c pipe_networking.h
	@gcc -c pipe_networking.c -g -Wall

server: compileMe
	@./run_server

player: compileMe
	@./run_player

clean:
	rm -f *.o
	rm -f run_server
	rm -f run_player
