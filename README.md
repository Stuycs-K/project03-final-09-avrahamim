[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Vh67aNdh)
# PROJECT NAME

The Math Melee

### GROUP NAME

Me, Michael, and I

### Group Members

1. Michael Avrahami
       
### Project Description:

My project is to code a math tournament between as many players as are able to join. Each player will face one opponent, single elimination style until there is one winner. 
The idea is that the first player will be given two random numbers, 1-200. They will have 7 seconds to add those two numbers and type in the result. If they do so successfully, the game will go to the other player. The other player will receive the sum the last player just returned and another random number, 1-200. They will have to type in the sum of those two numbers in 7 seconds. This goes on until someone makes the first mistake, and they lose.
The tournament progresses single-elimination until there is one final victor. If there are an odd number of players standing, one player receives a bye and automatically advances to the next round.
  
### Instructions:

How does the user install/compile/run the program?
* User must clone the repo from github, and compile it with the command `make compile`
* The user then has to intialize the program, with the command `make server`. The user will be prompted how many players they will wait on until starting the game. This number must be a positive integer, and must be entered before any players can enter the game.
* Next, the user has to open several more terminals to connect several clients to the server. For however many players the server user entered, in each terminal he will enter the command `make player`.
* Once the correct number of users have connected, the game will begin.

How does the user interact with this program?
* The server should not interact any more with the program, unless the server wants to shut down the whole game. The server can do this with ctrl+c.
* The users will receive two numbers which they will be instructed to add together. At the time of receiving their numbers, the users will have 7 seconds to mentally add them together and enter the sum into stdin. That is the only way the users should ever have to interact with the program.
* Users can forfeit their game with ctrl+c
