#ifndef __MEMLAB_H__
#define __MEMLAB_H__

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

#define INT 0
#define CHAR 1
#define MEDIUM_INT 2
#define BOOLEAN 3

int getSizeFromType(int type, int arrlen);
int isValid(int type, char *name);
int max(int a, int b);

typedef struct _Variable
{
  char *name;
  int type, size, localAddress, arrLen, isTobeCleaned;
} Variable;

Variable *CreateVariable(char *name, int type, int localAddr, int arrLen);

typedef struct mediumInt
{
  unsigned char value[3];
} mediumInt;

mediumInt CreateMediumInt(int val);
int toInt(mediumInt *mi);

typedef struct _Stack
{
  Variable *stck[STACK_SIZE];
  int topIndex;
} Stack;

Stack *createStack();
void push(Stack *s, Variable *x);
void pop(Stack *s);
Variable *top(Stack *s);
bool isEmpty(Stack *s);
int getSize(Stack *s);

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

void init(char *);
extern int *memory_;
extern Data *data_;
void createMem();
int createVar(char *name, int type);
bool typeCheck(int localAddress, int type);
char* getTypeString(int type);
void assignVarInt(int localAddress, int value);
void assignVarChar(int localAddress, char value);
void assignVarBool(int localAddress, bool value);
void assignVarMedium(int localAddress, mediumInt value);
void addToVar(int localAddress, int value);
void multToVar(int localAddress, int value);
int getValueVarInt(int localAddr);
char getValueVarChar(int localAddr);
bool getValueVarBool(int localAddr);
mediumInt getValueVarMedInt(int localAddr);
int createArr(char *name, int type, int arrLen);
void assignArrInt(int localAddr, int index, int value);
void assignArrChar(int localAddr, int index, char value);
void assignArrBool(int localAddr, int index, bool value);
void assignArrMedium(int localAddr, int index, mediumInt value);
int getValueArrInt(int localAddr, int index);
char getValueArrChar(int localAddr, int index);
bool getValueArrBool(int localAddr, int index);
mediumInt getValueArrMedInt(int localAddr, int index);
void addToArr(int localAddress, int index, int value);
void multToArr(int localAddress, int index, int value);
void freeElem(int locAddr);
void functionStart();
void endScope();
void *gc_run(void *);
void compact();
#endif 
// __MEMLAB_H__