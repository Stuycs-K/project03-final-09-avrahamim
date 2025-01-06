# The Math Game

## Group Members
1. Michael Avrahami

## Intentions
My project is to code a math tournament between as many players as are able to join. Each player will face one opponent, single elimination style until there is one winner. 
The idea is that the first player will be given two random numbers, 1-500. They will have 5-10 seconds to add those two numbers and type in the result. If they do so successfully, the game will go to the other player. This time, they will receive the sum the last player just returned and another random number, 1-200. They will have to type in the sum of those two numbers in 5-10 seconds. This goes on until someone makes the first mistake, and they lose.
The tournament progresses single-elimination until there is one final victor.

The above is my MVP. If the project goes well, I will look into adding more math operations like subtraction, multiplication, division, exponentiation and modulo. 

## Intended Usage
Users will connect to a server, which will eventually start a game at a user's prompting. Then, users simply will have to type in and enter their answers. Ctrl+c can disconnect midgame. For a user who wants to test the program/play it on their own, they will initialize a server and connect several clients to it using make commands proscribed in README.md

## Technical Details
All users will act as clients, using pipes to send questions and answers back and forth with the central game server. The game server will use an allocated integer and "/dev/urandom" to store game information and generate new random numbers for the game.

I will begin by simply ensuring clients can connect to the server and receive information back and forth.

I will also use semaphores to ensure only one client (out of the two clients in a single 1v1) out of the two clients is playing the game with the server. I will also use signal handling to ensure smooth exiting out of the game.

As an optional addition if the project goes well, I will extend the project to sockets because I'd like to learn about how they work.

## Intended Pacing
* Wednesday 1/8 - Two clients can connect to a server, they will proceed in a 1v1 game of math addition. Semaphores used to ensure only one of the two clients is playing at a time.
* Friday 1/10 - Server will distribute the game to different 1v1s, and a tournament can progress
* Tuesday 1/14 - Signal handling will allow smooth exiting
* Friday 1/17 - Additional math operations added, and sockets experimented with if there is time.
