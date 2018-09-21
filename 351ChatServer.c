

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "351ChatServer.h"
#include "ChatQueue.h"

#define SAVED_MESSAGE_COUNT 	3
#define MESSAGE_LENGTH 		1024
#define QUEUE_MESSAGE_FRONT	"You're in the waiting queue: "
#define QUEUE_MESSAGE_BACK	" users ahead of you"
#define JOINED_MESSAGE_BACK	" has joined the chat."

static void usage() {
    fprintf( stderr, "\nusage: 351ChatServer <port> <num_clients>\n" );
    fprintf( stderr, "\tport: The port number of the chat server\n" );
    fprintf( stderr, "\tnum_clients: The maximum number of chat clients (10 minimum)\n" );
}

static int err(char * msg){
  printf("%s\n", msg);
  return 0;
}

struct Client* makeClient(int sockIn, char* nameIn, char* pwdIn){
  Client *client = (struct Client*)malloc(sizeof(struct Client));
  char *name = malloc(sizeof(char*));
  char *pwd = malloc(sizeof(char*));
  strcpy(name, nameIn);
  strcpy(pwd, pwdIn);
  client->savedMsgs = makeQueue();
  client->msgCount = 0;
  client->sock = sockIn;
  client->name = name;
  client->pwd = pwd;

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

static int handleNewClient(int clientFd, Client* activeClients[], int activeNum, 
			   int maxActiveNum, Client* savedClients[], int savedNum,
			   struct Queue* clientQueue, int queuedNum){
  Client *client;
  int validInt;
  int retInt = -1;
  int openIndex = -1;
  char buff[MESSAGE_LENGTH];
  int msgSize = recv(clientFd, buff, MESSAGE_LENGTH, 0); 
  char clientName[msgSize];
  char clientPwd[msgSize];

  if(parseLoginInfo(buff, clientName, clientPwd, msgSize) < 0){
    sendMessage(clientFd, "", "Username and password required", 0, activeNum, activeClients);
    printf("parseLogin failed\n");
    return retInt; 
  }

  if(clientDuplicate(clientName, activeClients, activeNum) == 1){
    sendMessage(clientFd, "", "Username already in use", 0, activeNum, activeClients);
    return retInt;
  }

  validInt = validateClient(clientName, clientPwd, savedClients, savedNum);

  switch(validInt){
    case -2:
      sendMessage(clientFd, "", "Incorrect password", 0, activeNum, activeClients);
      return retInt;
    case -1:
      client = makeClient(clientFd, clientName, clientPwd);
      break;
    default:
      client = savedClients[validInt];
      break;
  }

  openIndex = getOpenIndex(activeClients, maxActiveNum);

  if(openIndex > -1){
    activeClients[openIndex] = client;
    retInt = 1;

    if(validInt > -1){
      sendSavedMessages(client);
    }
  }
  else{
    enqueue(clientQueue, client, ""); 
    sendMessage(client->sock, "ChatBot", makeQueueMessage(queuedNum), 1, activeNum, activeClients);
    retInt = 0;
  }
      
  return retInt;
}

static int parseLoginInfo(char msgIn[], char name[], char pwd[], int msgLen){
  int nameLen = 0;
  int pwdLen = 0;
  int index = 0;
  while(index < msgLen){
    if((int)msgIn[index] == 32){
      index++;
      break;
    }
    else{
      nameLen++;
      index++;
    }
  }
  while(index < msgLen){
    if((int)msgIn[index] == 32){
      break;
    }
    else{
      pwdLen++;
      index++;
    }
  } 

  if(nameLen > 0 && pwdLen > 0){
    memcpy(name, &msgIn[0], nameLen); 
    memcpy(pwd, &msgIn[nameLen + 1], pwdLen);
    name[nameLen] = '\0';
    pwd[pwdLen] = '\0';
    return 1;
  }
  else{
    return -1;
  }
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
    if(clients[i] == NULL){
      continue;
    }
    else if(strcmp(clients[i]->name, name) == 0){
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

static int getOpenIndex(Client* clients[], int maxClients){
  int index = -1;

  for(int i = 0; i < maxClients; i++){
    if(!clients[i]){
      index = i;
      break;
    }
  }

  return index;
}

static int getClientIndexFromFd(int clientFd, int numClients, Client* clients[]){
  int index = -1; 

  for(int i = 0; i < numClients; i++){
    if(clients[i] && clients[i]->sock == clientFd){
      index = i;
      break;
    }
  }

  return index;
}

static int sendMessage(int clientFd, char* senderName, char* msg, int useName, int numClients, Client* clients[]){
  int len = strlen(senderName) + strlen(msg) + 15;
  char wMsg[len];
  int bytesSent = 0;
  int bytesRemaining;

  if(useName){
    time_t currentTime = time(NULL);
    char* dateTimeString = ctime(&currentTime);
    char timeStamp[8];
    memcpy(timeStamp, &dateTimeString[11], 8); 
    strcpy(wMsg, "[");
    strcat(wMsg, senderName);
    strcat(wMsg, " (");
    strcat(wMsg, timeStamp);
    strcat(wMsg, ")] ");
    strcat(wMsg, msg);
    wMsg[len - 1] = '\0';
  }
  else{
    strcpy(wMsg, msg);
  }

  bytesSent = send(clientFd, &wMsg[0], len, 0);

  if(bytesSent > 0 && useName){
    int index = getClientIndexFromFd(clientFd, numClients, clients);

    if(index > -1){
      addSavedMessage(clients[index], wMsg);
    }
  }

  return bytesSent;
}

static void addSavedMessage(Client* client, char* msg){
  enqueue(client->savedMsgs, NULL, msg);

  if(client->msgCount > SAVED_MESSAGE_COUNT){
    dequeue(client->savedMsgs);
  } 
  else{
    client->msgCount = client->msgCount++;
  }
}

static void sendSavedMessages(Client* client){
  while(client->msgCount > 0){
    sendMessage(client->sock, "", dequeue(client->savedMsgs)->msg, 0, 0, NULL);
    client->msgCount = client->msgCount--;
  }
}

static char* makeQueueMessage(int num){
  char buff[10];
  sprintf(buff, "%d", num);
  int len = strlen(QUEUE_MESSAGE_FRONT) + strlen(buff) + strlen(QUEUE_MESSAGE_BACK) + 1;
  char msg[len];
  strcpy(msg, QUEUE_MESSAGE_FRONT);
  strcat(msg, buff);
  strcat(msg, QUEUE_MESSAGE_BACK);
  msg[len - 1] = '\0';
  char *wMsg = &msg[0];

  return wMsg;
}

static char* makeJoinedMessage(char* name){
  int len = strlen(name) + strlen(JOINED_MESSAGE_BACK) + 1;
  char msg[len];
  strcpy(msg, name);
  strcat(msg, JOINED_MESSAGE_BACK);
  msg[len - 1] = '\0';
  char *wMsg = &msg[0];

  return wMsg;
}

static void broadcastMessage(char* senderName, fd_set activeClientSet, char* msg, 
			     int numClients, Client* clients[], int maxActiveClients){
  for(int i = 0; i < maxActiveClients; i++){
    if(clients[i]){
      sendMessage(clients[i]->sock, senderName, msg, 1, numClients, clients);
    }
  } 
}

int main(int argc, char * argv[]){
  int servSock, portIn, sinSize, maxActiveClients, maxSavedClients, activeClientCount;
  int openIndex = 0;
  int queuedCount = 0;
  struct sockaddr_in servIpAddr, clientIpAddr;
  struct Queue *clientQueue;
  fd_set activeClientSet, sendingClientSet;

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

  memset(&(servIpAddr.sin_zero), '\0', 8);

  servSock = socket(PF_INET, SOCK_STREAM, 0);

  if(servSock < 0){
    return err("Socket error");
  }

  if(bind(servSock, (struct sockaddr *)&servIpAddr, sinSize) < 0){
    return err("Bind error");
  }

  if(listen(servSock, 1000) < 0){
    return err("Listen error");
  }

  clientQueue = makeQueue();
  FD_ZERO(&activeClientSet);
  FD_SET(servSock, &activeClientSet);

  while(1){
    sendingClientSet = activeClientSet;

    if(select(FD_SETSIZE, &sendingClientSet, NULL, NULL, NULL) < 0){
      return err("Select error");
    }

    for(int i = 0; i < FD_SETSIZE; i++){
      if(FD_ISSET(i, &sendingClientSet)){
        if(i == servSock){
	  int clientFd = accept(servSock, (struct sockaddr *)&clientIpAddr, &sinSize);
          
	  if(clientFd < 0){
  	    return err("Accept error");
	  }
	  else{
	    int newC = handleNewClient(clientFd, activeClients, activeClientCount,
			   	       maxActiveClients, savedClients, maxSavedClients,
				       clientQueue, queuedCount);	
	    if(newC == 1){
	      FD_SET(clientFd, &activeClientSet);
	      activeClientCount++;
	      int index = getClientIndexFromFd(clientFd, activeClientCount, activeClients); 
	      if(index > -1){
  	        int len = strlen(activeClients[index]->name) + strlen(JOINED_MESSAGE_BACK) + 1;
  	        char msg[len];
  	        strcpy(msg, activeClients[index]->name);
  	        strcat(msg, JOINED_MESSAGE_BACK);
	        msg[len - 1] = '\0';
	        broadcastMessage("ChatBot", activeClientSet, &msg[0], 
			         activeClientCount, activeClients, maxActiveClients);
	      }
	    }
	    else if(newC == 0){
	      queuedCount++;
	    }
	    else{
	      err("New client error");
	    }
	  }
	}
	else{
	  char buff[MESSAGE_LENGTH];
	  int msgSize = recv(i, buff, MESSAGE_LENGTH, 0); 
	  buff[msgSize] = '\0';

	  if(msgSize < 0){
	    err("Client read error");
	  }
	  else if(msgSize == 0){
	    printf("Client disconnected\n");
	    close(i);
	    FD_CLR(i, &activeClientSet);
	    activeClientCount--;
	  }
	  else{
	    int index = getClientIndexFromFd(i, activeClientCount, activeClients);

	    if(index > -1){
	      Client *client = activeClients[index];
	      char *name = client->name;
	      broadcastMessage(name, activeClientSet, &buff[0], 
			       activeClientCount, activeClients, maxActiveClients);
	    }
	  }
	}
      }
    }
  }
}
