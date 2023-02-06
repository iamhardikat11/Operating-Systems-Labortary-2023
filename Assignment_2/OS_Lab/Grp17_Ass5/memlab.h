#ifndef __MEMLAB_H__
#define __MEMLAB_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

#define MEM_SIZE 250000000
#define VAR_NAME_SIZE 10
#define STACK_SIZE 100
#define NUM_VARIABLES 100

#define INT 0
#define CHAR 1
#define MEDIUM_INT 2
#define BOOLEAN 3

class Variable {
 public:
  string name;
  int type, size, localAddress, arrLen, isTobeCleaned;

  int isValid(int type, string name);

  int getSizeFromType(int type, int arrlen);

  Variable();

  Variable(string name, int type, int localAddr, int arrLen);
};

class mediumInt {
  unsigned char value[3];

 public:
  mediumInt(int val);

  int toInt();
};

class Stack {
 private:
  Variable *stck[STACK_SIZE];
  int topIndex;

 public:
  Stack();

  void push(Variable *x);

  void pop();

  Variable *top();

  bool isEmpty();

  int getSize();
};

typedef struct {
  int *pageTable[NUM_VARIABLES];
  int localAddress, maxMemIndex;
  int actualAddressToLocalAdress[MEM_SIZE / 4];
  Variable variableList[NUM_VARIABLES];
  Stack variableStack;
  pthread_mutex_t lock;
  pthread_attr_t attr;
  pthread_t ptid;

} Data;

namespace MemLab {
extern int *memory_;
extern Data *data_;

void createMem();

int createVar(string name, int type);

bool typeCheck(int localAddress, int type);
string getTypeString(int type);

void assignVar(int localAddress, int value);

void assignVar(int localAddress, char value);

void assignVar(int localAddress, bool value);

void assignVar(int localAddress, mediumInt value);

void addToVar(int localAddress, int value);

void multToVar(int localAddress, int value);

int getValueVarInt(int localAddr);

char getValueVarChar(int localAddr);

bool getValueVarBool(int localAddr);

mediumInt getValueVarMedInt(int localAddr);

int createArr(string name, int type, int arrLen);

void assignArr(int localAddr, int index, int value);

void assignArr(int localAddr, int index, char value);

void assignArr(int localAddr, int index, bool value);

void assignArr(int localAddr, int index, mediumInt value);

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
};  // namespace MemLab

void debugFreeSpace();

void testCreate();

void testAssign();
void testArr();

void testCompact();

#endif  // __MEMLAB_H__