

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "351ChatServer.h"
#include "ChatQueue.h"

#define SAVED_MESSAGE_COUNT 3

static void usage() {
    fprintf( stderr, "\nusage: 351ChatServer <port> <num_clients>\n" );
    fprintf( stderr, "\tport: The port number of the chat server\n" );
    fprintf( stderr, "\tnum_clients: The maximum number of chat clients (10 minimum)\n" );
}

static int err(char * msg){
  printf("%s\n", msg);
  return 0;
}

struct Client* makeClient(int sockIn, int indexIn, char* nameIn, char* pwdIn){
  Client *client = (struct Client*)malloc(sizeof(struct Client));
  client->savedMsgs = makeQueue();
  client->msgCount = 0;
  client->sock = sockIn;
  client->index = indexIn;
  client->name = nameIn;
  client->pwd = pwdIn;

  return client;
}

static void saveClient(Client* client, Client* savedClients[], int savedNum,
	               Client* msgClients[], char* msgs[], int msgNum){
  for(int i = 0; i < msgNum; i++){
    enqueueMsg(client, msgClients[i], msgs[i]);
  }

  for(int i = 0; i < savedNum; i++){
    if(savedClients[i] == NULL){
      savedClients[i] = client;
    }
  } 
}

static void enqueueMsg(Client* sender, Client* receiver, char* msg){
  enqueue(receiver->savedMsgs, sender, msg);

  if(receiver->msgCount == SAVED_MESSAGE_COUNT){
    dequeue(receiver->savedMsgs);
  } 
  else{
    receiver->msgCount = receiver->msgCount + 1;
  }
}

struct Client* handleNewClient(int sock, Client* activeClients[], int activeNum, 
			       int maxNum, Client* savedClients[], int savedNum,
			       struct Queue* clientQueue, int openIndex){
  Client *client;
  int validInt;
  // get name and password from recv()
  char* clientName = "Bob";
  char* clientPwd = "password";

  if(clientDuplicate(clientName, activeClients, activeNum) == 1){
    send_message(sock, "Username already in use");
    return NULL;
  }

  validInt = validateClient(clientName, clientPwd, savedClients, savedNum);

  switch(validInt){
    case -2:
      send_message(sock, "Incorrect password");
      return NULL;
    case -1:
      client = makeClient(sock, openIndex, clientName, clientPwd);
      break;
    default:
      client = savedClients[validInt];
      break;
  }

  // get open index

  if(activeNum < maxNum){
    activeClients[activeNum] = client;
  }
  else{
    enqueue(clientQueue, client, ""); 
  }

  return client;
}

static int clientDuplicate(char* name, Client* clients[], int numClients){
  int isDup = 0;

  if(numClients == 0){
    return isDup;
  }

  for(int i = 0; i < numClients; i++){
    if(clients[i] == NULL){
      continue;
    }
    else if(strcmp(clients[i]->name, name) == 0){
      isDup = 1;
      break;
    }
  }

  return isDup; 
}

static int validateClient(char* name, char *pwd, Client* clients[], int numClients){
  int isValid = -1;
  
  for(int i = 0; i < numClients; i++){
    if(strcmp(clients[i]->name, name) == 0){
      if(strcmp(clients[i]->pwd, pwd) != 0){
	isValid = -2;
      }
      else{
	isValid = i;
      }

      break;
    }
  }

  return isValid;  
}

static int send_message(int sock, char * msg){
  int len = strlen(msg);
  int bytesSent = 0;
  int bytesRemaining;

  while(bytesSent < len){
    bytesRemaining = len - bytesSent;
    char msgChunk[bytesRemaining];
    memcpy(msgChunk, &msg[bytesSent], bytesRemaining);
    bytesSent += send(sock, msgChunk, bytesRemaining, 0);

    if(bytesSent == 0){
      break;
    }
  }

  return bytesSent;
}

int main(int argc, char * argv[]){
  int servSock, clientSock, portIn, sinSize, maxActiveClients, maxSavedClients, activeClientCount;
  int openIndex = 0;
  struct sockaddr_in servIpAddr, clientIpAddr;

  if(argc != 3){
    usage();
    return 0;
  }

  portIn = (int)strtol(argv[1], NULL, 10);
  maxActiveClients = (int)strtol(argv[2], NULL, 10);

  if(portIn == 0 || maxActiveClients == 0){
    usage();
    return 0;
  }

  maxSavedClients = maxActiveClients * 10;
  activeClientCount = 0;
  Client* activeClients[maxActiveClients];
  Client* savedClients[maxSavedClients];

  for(int i = 0; i < maxActiveClients; i++){
    activeClients[i] = NULL;
  }

  for(int i = 0; i < maxSavedClients; i++){
    savedClients[i] = NULL;
  }

  sinSize = sizeof(servIpAddr);

  servIpAddr.sin_family = AF_INET;
  servIpAddr.sin_port = htons(portIn);
  servIpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  // servIpAddr.sin_addr.s_addr = inet_addr("10.12.110.57");

  memset(&(servIpAddr.sin_zero), '\0', 8);

  servSock = socket(PF_INET, SOCK_STREAM, 0);

  if(servSock < 0){
    return err("Socket error");
  }

  // printf("sin_family: %d\n", servIpAddr.sin_family);
  // printf("sin_port: %d\n", servIpAddr.sin_port);
  // printf("sin_addr.s_addr: %d\n", servIpAddr.sin_addr.s_addr);
  // printf("servSock: %d\n", servSock);
  // printf("sizeof(servIpAddr): %ld\n", sizeof(servIpAddr));
  // printf("bind: %d\n", bind(servSock, (struct sockaddr *)&servIpAddr, sizeof(servIpAddr)));
  if(bind(servSock, (struct sockaddr *)&servIpAddr, sinSize) < 0){
     // printf("%d\n", errno);
    return err("Bind error");
  }

  if(listen(servSock, 1000) < 0){
    return err("Listen error");
  }
  
  Queue *clientQueue = makeQueue();

  while(1){
    clientSock = accept(servSock, (struct sockaddr *)&clientIpAddr, &sinSize);
    printf("client connected: %d", clientSock);

  }

  for(int i = 0; i < activeClientCount; i++){
    close(activeClients[i]->sock);
  }

  close(servSock);
}
