#ifndef CHATQUEUE_H
#define CHATQUEUE_H

#include "MutualStructs.h"

struct Queue* makeQueue();

struct Node* makeNode(Client *clientIn, char *msgIn);

void enqueue(struct Queue *queue, struct Client *clientIn, char* msgIn);

struct Node* dequeue(struct Queue *queue);

#endif
