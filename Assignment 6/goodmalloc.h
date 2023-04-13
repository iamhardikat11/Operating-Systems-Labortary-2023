#ifndef __GOOD_H__
#define __GOOD_H__

#include <string.h> 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#define MEM_SIZE 250000000
#define VAR_NAME_SIZE 10
#define STACK_SIZE 100
#define NUM_VARIABLES 100
#define STR_SIZE 250
#define LIMIT 100000


#define INT 0
#define CHAR 1
#define MEDIUM_INT 2
#define BOOLEAN 3

int getSizeFromType(int type, int arrlen);
int isValid(int type, char *name);
int max(int a, int b);

typedef struct Node {
    int data;
    struct Node* next;
    struct Node* prev;
}Node;

typedef struct DLL {
  char* name;
  int sz;
  int curr_sz;
  Node** list;
} DDL;

typedef struct _Variable
{
  char *name;
  int type, size, localAddress, arrLen, isTobeCleaned;
} Variable;


// Medium Int's Implementation
typedef struct mediumInt
{
  unsigned char value[3];
} mediumInt;

// Stack's Implementation
typedef struct _Stack
{
  Variable *stck[STACK_SIZE];
  int topIndex;
} Stack;
//
typedef struct
{
  int *pageTable[NUM_VARIABLES];
  int localAddress, maxMemIndex;
  int actualAddressToLocalAdress[MEM_SIZE / 4];
  Variable variableList[NUM_VARIABLES];
  Stack variableStack;
  pthread_mutex_t lock;
  pthread_attr_t attr;
  pthread_t ptid;
} Data;


// Important Data Structures
extern int *memory_;
extern Data *data_;

/*
  Important Functions
*/
void createMem();
int createList(char *name, int type, int sz);
void assignVal(int localAddress, void* value, int type);
void freeElem(int locAddr);

// Utiliy Functions
void pushList(Node** head_ref, int new_data);
void insertAfterList(Node* prev_node, int new_data);
void appendList(Node** head_ref, int new_data);
void printList(Node* node);

Variable *CreateVariable( char *name, int type, int localAddr, int arrLen);

mediumInt CreateMediumInt(int val);
int toInt(mediumInt *mi);

Stack *createStack();
void push(Stack *s, Variable *x);
void pop(Stack *s);
Variable *top(Stack *s);
bool isEmpty(Stack *s);
int getSize(Stack *s);

void init(char *);
bool typeCheck(int localAddress, int type);
int createVar(char *name, int type);
char* getTypeString(int type);
void addToVal(int localAddress, int value);
void multToVal(int localAddress, int value);
void getVal(int localAddr, int type, void* data);
void functionStart();
void endScope();
void *gc_run(void *);
void compact();
#endif 
// __GOOD_H__