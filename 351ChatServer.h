
#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "MutualStructs.h"

static void usage();

static int err(char* msg);

struct Client* makeClient(int sockIn, char* nameIn, char* pwdIn);

static void saveClient(Client* client, Client* savedClients[], int savedNum,
		       Client* msgClients[], char* msgs[], int msgNum);

static void enqueueMsg(Client* sender, Client* receiver, char* msg);

static int sendMessage(int clientFd, char* senderName, char* msg, 
		       int useName, int numClients, Client* clients[]);

static int clientDuplicate(char* name, Client* clients[], int numClients);

static int validateClient(char* name, char *pwd, Client* clients[], int numClients);

static int getOpenIndex(Client* clients[], int maxClients);

static int getClientIndexFromFd(int clientFd, int numClients, Client* clients[]);

static void addSavedMessage(Client* client, char* msg);

static void sendSavedMessages(Client* client);

static int parseLoginInfo(char msgIn[], char name[], char pwd[], int msgLen);

static char* makeQueueMessage(int num);

static char* makeJoinedMessage(char* name);

static void broadcastMessage(char* senderName, fd_set activeClientSet, char* msg,
                             int numClients, Client* clients[]);

static int handleNewClient(int clientFd, Client* activeClients[], int activeNum,
                           int maxActiveNum, Client* savedClients[], int savedNum,
                           struct Queue* clientQueue, int queuedNum);
#endif
