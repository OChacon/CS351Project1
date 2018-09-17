#include <stdlib.h>
#include "ChatQueue.h"

struct Queue* makeQueue(){
  struct Queue *queue = (struct Queue*)malloc(sizeof(struct Queue));
  queue->first = NULL;
  queue->last = NULL;
  return queue;
}

struct Node* makeNode(Client *clientIn, char *msgIn){
  struct Node *node = (struct Node*)malloc(sizeof(struct Node));
  node->client = clientIn;
  node->msg = msgIn;
  node->next = NULL;
  return node; 
}

void enqueue(struct Queue *queue, struct Client *clientIn, char* msgIn){
  struct Node *node = makeNode(clientIn, msgIn);
      
  if(queue->first == NULL){
    queue->first = node;
  }
  else{
    queue->last->next = node;
  }

  queue->last = node;
}

struct Node* dequeue(struct Queue *queue){
  if(queue->first == NULL){
    return NULL;
  }
  else{
    struct Node *oldNode = queue->first;
    queue->first = oldNode->next;

    if(queue->first == NULL){
      queue->last = NULL;
    }

    return oldNode;
  }
}
