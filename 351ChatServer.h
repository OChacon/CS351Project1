
#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "MutualStructs.h"

static void usage();

static int err(char* msg);

struct Client* makeClient(int sockIn, int indexIn, char* nameIn, char* pwdIn);

static void saveClient(Client* client, Client* savedClients[], int savedNum,
		       Client* msgClients[], char* msgs[], int msgNum);

static void enqueueMsg(Client* sender, Client* receiver, char* msg);

static int send_message(int sock, char * msg);

static int clientDuplicate(char* name, Client* clients[], int numClients);

static int validateClient(char* name, char *pwd, Client* clients[], int numClients);

struct Client* handleNewClient(int sock, struct Client* activeClients[], int activeNum, 
		               int maxNum, struct Client* savedClients[], int savedNum,
			       struct Queue* clientQueue, int openIndex);

#endif
