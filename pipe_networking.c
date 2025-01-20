#include "pipe_networking.h"
//UPSTREAM = to the server / from the client
//DOWNSTREAM = to the client / from the server
/*=========================
  server_setup

  creates the WKP and opens it, waiting for a  connection.
  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  mkfifo(WKP, 0666);
  chmod(WKP, 0666);

  int fifoFD = open(WKP, O_RDONLY);
  //printf("from_client fd: %d\n", fifoFD);

  remove(WKP);
  return fifoFD;
}

int subserver_connect(int from_client){
  int x = 0;
  int *to_client = &x;
// Reading to_client from from_client
  char clientFifoPath[HANDSHAKE_BUFFER_SIZE + 1];
  int readResult = read(from_client, clientFifoPath, HANDSHAKE_BUFFER_SIZE);
  if (readResult == -1){ printf("Server reading from WKP error\n"); exit(1);}
 // printf("readResult: %d\n", readResult);

  // Opening to_client
  *to_client = open(clientFifoPath, O_WRONLY);
  printf("to_client fd: %d\n", *to_client);

  // Writing SYN_ACK randnum to client
  char synack[HANDSHAKE_BUFFER_SIZE];
  int randFd = open("/dev/random", O_RDONLY);
  int randNum;
  int randReadResult = read(randFd, &randNum, sizeof(int));
  if (randReadResult == -1){ printf("uhoh:\n"); exit(1);}
  sprintf(synack, "%d", randNum);
  int writeResult = write(*to_client, synack, HANDSHAKE_BUFFER_SIZE);
  if (writeResult == -1){ printf("Server writing to client error\n"); exit(1);}
  //printf("writeResult: %d\n", writeResult);

  // Reading ack from client, should be randNum + 1

  readResult = read(from_client, synack, HANDSHAKE_BUFFER_SIZE);
  if (readResult == -1){ printf("Server reading from client error\n"); exit(1);}
  //printf("readResult: %d\n", readResult);
  if (atoi(synack) != randNum + 1){
     printf("ack doesn't match. handshake failed"); exit(1);
  }

  return *to_client;
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe (Client's private pipe).

  returns the file descriptor for the upstream pipe (see server setup).
  =========================*/
int server_handshake(int *to_client) {
  // Creating and opening from_client
int x = 0;
  int * from_client = &x;
  *from_client = server_setup();

  // Reading to_client from from_client
  char clientFifoPath[HANDSHAKE_BUFFER_SIZE + 1];
  int readResult = read(*from_client, clientFifoPath, HANDSHAKE_BUFFER_SIZE);
  if (readResult == -1){ printf("Server reading from WKP error\n"); exit(1);}
  //printf("readResult: %d\n", readResult);

  // Opening to_client
  *to_client = open(clientFifoPath, O_WRONLY);
  //printf("to_client fd: %d\n", *to_client);

  // Writing SYN_ACK randnum to client
  char synack[HANDSHAKE_BUFFER_SIZE];
  int randFd = open("/dev/random", O_RDONLY);
  int randNum;
  int randReadResult = read(randFd, &randNum, sizeof(int));
  if (randReadResult == -1){ printf("uhoh:\n"); exit(1);}
  sprintf(synack, "%d", randNum);
  int writeResult = write(*to_client, synack, HANDSHAKE_BUFFER_SIZE);
  if (writeResult == -1){ printf("Server writing to client error\n"); exit(1);}
  //printf("writeResult: %d\n", writeResult);

  // Reading ack from client, should be randNum + 1

  readResult = read(*from_client, synack, HANDSHAKE_BUFFER_SIZE);
  if (readResult == -1){ printf("Server reading from client error\n"); exit(1);}
  //printf("readResult: %d\n", readResult);
  if (atoi(synack) != randNum + 1){
     printf("ack doesn't match. handshake failed"); exit(1);
  }

  return *from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  // Opening PP
  int pid = getpid();
  int x = 0;
  int * from_server = &x;

  // Making path for from_server
  char fifoPath[HANDSHAKE_BUFFER_SIZE];
  sprintf(fifoPath, "%d", pid);
  mkfifo(fifoPath, 0666);
  chmod(fifoPath, 0666);
  // for (int i = 0; i < strlen(fifoPath); i++){
  //   printf("%c\n", fifoPath[i]);
  // }

  // Opening WKP
  *to_server = open(WKP, O_WRONLY);
  //printf("to_server fd: %d\n", *to_server);

  // Writing SYN
  int writeResult = write(*to_server, fifoPath, HANDSHAKE_BUFFER_SIZE);
  if (writeResult == -1){ printf("Client writing to WKP error\n"); exit(1);}
  //printf("writeResult: %d\n", writeResult);

  // Opening PP to read from server
  *from_server = open(fifoPath, O_RDONLY);
//  printf("from_server fd: %d\n", *from_server);

  remove(fifoPath);

  // Reading SYN_ACK
  char synack[HANDSHAKE_BUFFER_SIZE];
  int readResult = read(*from_server, synack, HANDSHAKE_BUFFER_SIZE);
  if (readResult == -1){ printf("Client reading from server error\n"); exit(1);}
  //printf("readResult: %d\n", readResult);

  // Writing ACK to server
  char ack[HANDSHAKE_BUFFER_SIZE];
  int ackToWrite = atoi(synack) + 1;
  sprintf(ack, "%d", ackToWrite);
  writeResult = write(*to_server, ack, HANDSHAKE_BUFFER_SIZE);
  if (writeResult == -1){ printf("Client writing to WKP error 2\n"); exit(1);}
  //printf("writeResult 2: %d\n", writeResult);




  return *from_server;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
