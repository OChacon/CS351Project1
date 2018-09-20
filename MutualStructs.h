#ifndef MUTUALSTRUCTS_H
#define MUTUALSTRUCTS_H

struct Queue;

typedef struct Client{
  struct Queue* savedMsgs;
  int msgCount;
  int sock;
  char* name;
  char* pwd;
} Client;

typedef struct Queue{
  struct Node *first;
  struct Node *last;
} Queue;

typedef struct Node{
  struct Client *client;
  struct Node *next;
  char *msg;
} Node;

#endif
