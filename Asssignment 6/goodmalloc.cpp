#include "goodmalloc.h"

int *memory_;
Data *data_;
int gc, no_gc;
FILE *f1, *f2;

int isValid(int type, char *name)
{
  return ((type == INT || type == CHAR || type == MEDIUM_INT || type == BOOLEAN) && strlen(name) < VAR_NAME_SIZE);
}

int getSizeFromType(int type, int arrlen)
{
  switch (type)
  {
  case INT:
    return (4 * arrlen + 3) / 4;
  case CHAR:
    return (1 * arrlen + 3) / 4;
  case MEDIUM_INT:
    return (3 * arrlen + 3) / 4;
  case BOOLEAN:
    return (1 * arrlen + 3) / 4;
  default:
    return 0;
  }
}

int toInt(mediumInt *m)
{
  int val = 0;
  if (!(m->value[0] >> 7 == 0))
    val |= (((1 << 8) - 1) << 24);
  val |= m->value[2] | (m->value[1] << 8) | (m->value[0] << 16);
  return val;
}

Variable *CreateVariable(char *name, int type, int localAddr, int arrLen)
{
  if (!isValid(type, name))
  {
    fprintf(stderr, "Invalid Variable\n");
    exit(1);
  }
  Variable *a = (Variable *)malloc(sizeof(Variable));
  strcpy(a->name, name);
  a->type = type;
  a->isTobeCleaned = 0;
  a->arrLen = arrLen;
  a->localAddress = localAddr;
  a->size = getSizeFromType(type, arrLen);
  return a;
}

mediumInt CreateMediumInt(int val)
{
  if (val >= (1 << 23) || (val < -(1 << 23)))
  {
    fprintf(stderr, "Overflow in Medium Int\n");
    exit(1);
  }
  mediumInt mi;
  mi.value[0] = (val >> 16) & 0xFF;
  mi.value[1] = (val >> 8) & 0xFF;
  mi.value[2] = val & 0xFF;
  return mi;
}

Stack *createStack()
{
  Stack *s = (Stack *)malloc(sizeof(Stack));
  s->topIndex = -1;
  return s;
}

void push(Stack *s, Variable *x)
{
  if (s->topIndex == STACK_SIZE - 1)
  {
    fprintf(stderr, "Global Stack Size Limit Reached. Exitting.\n");
    exit(1);
  }
  s->stck[++s->topIndex] = x;
}
void pop(Stack *s)
{
  if (s->topIndex == -1)
  {
    fprintf(stderr, "Error, stack is Empty!\n");
    exit(1);
  }
  s->topIndex--;
}
Variable *top(Stack *s)
{
  return s->stck[s->topIndex];
}
bool isEmpty(Stack *s)
{
  if (s->topIndex == -1)
    return 1;
  return 0;
}
int getSize(Stack *s)
{
  return s->topIndex + 1;
}

void init(char *n)
{
  n = (char *)malloc(STR_SIZE * sizeof(char));
}
void createMem()
{
  printf("Memory Created\n");
  gc = 0;
  no_gc = 0;
  f1 = fopen("gc.txt", "w");
  f2 = fopen("no_gc.txt", "w");
  memory_ = (int *)malloc(MEM_SIZE + sizeof(Data));
  data_ = (Data *)(memory_ + MEM_SIZE / 4);
  pthread_mutex_init(&data_->lock, NULL);
  pthread_attr_init(&data_->attr);
  pthread_create(&data_->ptid, &data_->attr, gc_run, NULL);
  for (int i = 0; i < MEM_SIZE / 4; i++)
    data_->actualAddressToLocalAdress[i] = -1;
  data_->localAddress = 0;
  data_->maxMemIndex = -1;
}

int createVar(char *name, int type)
{
  printf("Creating Variable of name: %s and type %s\n", name, getTypeString(type));
  if (data_->localAddress / 4 >= NUM_VARIABLES)
  {
    fprintf(stderr, "Variable count limit reached. Exitting.\n");
    exit(1);
  }
  Variable *var = CreateVariable(name, type, data_->localAddress, 1);
  data_->variableList[data_->localAddress / 4] = *var;
  no_gc += var->size;
  gc += var->size;
  pthread_mutex_lock(&data_->lock);
  for (int i = 0; i < MEM_SIZE / 4; i++)
  {
    if (data_->actualAddressToLocalAdress[i] == -1)
    {
      data_->maxMemIndex = max(data_->maxMemIndex, i);
      data_->pageTable[data_->localAddress / 4] = memory_ + i;
      data_->actualAddressToLocalAdress[i] = data_->localAddress;
      break;
    }
  }
  pthread_mutex_unlock(&data_->lock);
  int temp = data_->localAddress;
  data_->localAddress += 4;
  push(&data_->variableStack, &data_->variableList[temp / 4]);
  return temp;
}
int max(int a, int b)
{
  return a > b ? a : b;
}
bool typeCheck(int localAddress, int type)
{
  return (data_->variableList[localAddress / 4].type == type);
}

char *getTypeString(int type)
{
  char *ans;
  init(ans);
  if (type == INT)
    strcpy(ans, "INT");
  else if (type == BOOLEAN)
    strcpy(ans, "BOOLEAN");
  else if (type == CHAR)
    strcpy(ans, "CHAR");
  else
    strcpy(ans, "MEDIUM_INT");
  return ans;
}

void assignVarInt(int localAddress, int value)
{
  printf("Assigning integer value to variable %s\n", data_->variableList[localAddress].name);
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignVarChar(int localAddress, char value)
{
  printf("Assigning char value to variable %s\n", data_->variableList[localAddress].name);
  if (!typeCheck(localAddress, CHAR))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignVarBool(int localAddress, bool value)
{
  printf("Assigning boolean value to variable %s\n", data_->variableList[localAddress].name);
  if (!typeCheck(localAddress, BOOLEAN))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignVarMedium(int localAddress, mediumInt value)
{
  printf("Assigning medium int value to variable %s\n", data_->variableList[localAddress].name);
  if (!typeCheck(localAddress, MEDIUM_INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = toInt(&value);
  pthread_mutex_unlock(&data_->lock);
}

void addToVar(int localAddress, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) + value;
}

void multToVar(int localAddress, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) * value;
}

int getValueVarInt(int localAddr)
{
  if (!typeCheck(localAddr, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

char getValueVarChar(int localAddr)
{
  if (!typeCheck(localAddr, CHAR))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

bool getValueVarBool(int localAddr)
{
  if (!typeCheck(localAddr, BOOLEAN))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

mediumInt getValueVarMedInt(int localAddr)
{
  if (!typeCheck(localAddr, MEDIUM_INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return CreateMediumInt(*physicalAddress);
}

int createArr(char *name, int type, int arrLen)
{
  printf("Creating Array of name: %s and type %s and length %d\n", name, getTypeString(type), arrLen);
  if (data_->localAddress / 4 >= NUM_VARIABLES)
  {
    fprintf(stderr, "Variable count limit reached. Exitting.\n");
    exit(1);
  }
  Variable *var = CreateVariable(name, type, data_->localAddress, arrLen);
  data_->variableList[data_->localAddress / 4] = *var;
  no_gc += var->size;
  gc += var->size;
  int len = var->size;
  pthread_mutex_lock(&data_->lock);
  for (int i = 0; i < MEM_SIZE / 4; i++)
  {
    int flag = 1;
    for (int j = 0; j < len; j++)
    {
      if (j + i < MEM_SIZE / 4)
      {
        if (!(data_->actualAddressToLocalAdress[i + j] == -1))
        {
          flag = 0;
          break;
        }
      }
      else
        break;
    }
    if (flag == 1)
    {
      for (int j = 0; j < len; j++)
      {
        if (j + i < MEM_SIZE / 4)
        {
          data_->maxMemIndex = max(data_->maxMemIndex, i + j);
          data_->actualAddressToLocalAdress[j + i] = data_->localAddress;
        }
      }
      data_->pageTable[data_->localAddress / 4] = memory_ + i;
      break;
    }
  }
  pthread_mutex_unlock(&data_->lock);
  int temp = data_->localAddress;
  data_->localAddress += 4;
  push(&data_->variableStack, &data_->variableList[temp / 4]);
  return temp;
}

void assignArrInt(int localAddr, int index, int value)
{
  if (!typeCheck(localAddr, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  int *physicalAddress = data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignArrChar(int localAddr, int index, char value)
{
  if (!typeCheck(localAddr, CHAR))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  char *physicalAddress = (char *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignArrBool(int localAddr, int index, bool value)
{
  if (!typeCheck(localAddr, BOOLEAN))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  bool *physicalAddress = (bool *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&data_->lock);
}

void assignArrMedium(int localAddr, int index, mediumInt value)
{
  if (!typeCheck(localAddr, MEDIUM_INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&data_->lock);
  mediumInt *physicalAddress = (mediumInt *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&data_->lock);
}

int getValueArrInt(int localAddr, int index)
{
  if (!typeCheck(localAddr, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  int val = physicalAddress[index];
  return val;
}

char getValueArrChar(int localAddr, int index)
{
  if (!typeCheck(localAddr, CHAR))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  char *physicalAddress = (char *)data_->pageTable[localAddr / 4];
  char val = physicalAddress[index];
  return val;
}

bool getValueArrBool(int localAddr, int index)
{
  if (!typeCheck(localAddr, BOOLEAN))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  bool *physicalAddress = (bool *)data_->pageTable[localAddr / 4];
  return physicalAddress[index];
}

mediumInt getValueArrMedInt(int localAddr, int index)
{
  if (!typeCheck(localAddr, MEDIUM_INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  mediumInt *physicalAddress = (mediumInt *)data_->pageTable[localAddr / 4];
  return physicalAddress[index];
}

void addToArr(int localAddress, int index, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddress / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  physicalAddress[index] = physicalAddress[index] + value;
}

void multToArr(int localAddress, int index, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddress / 4].arrLen)
  {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  physicalAddress[index] = physicalAddress[index] * value;
}

void freeElem(int locAddr)
{
  int len = data_->variableList[locAddr / 4].size;
  int index = data_->pageTable[locAddr / 4] - memory_;
  printf("Freeing variable %s\n", data_->variableList[locAddr / 4].name);
  gc -= data_->variableList[locAddr / 4].size;
  for (int i = 0; i < len; i++)
  {
    if (index + i < MEM_SIZE / 4)
      data_->actualAddressToLocalAdress[index + i] = -1;
    else
    {
      fprintf(stderr, "Freeing invalid index.\n");
      exit(1);
    }
  }
}

void functionStart()
{
  push(&data_->variableStack, NULL);
}
void clean()
{
  int flag = 0;
  for (int i = 0; i < data_->localAddress; i += 4)
  {
    if (data_->variableList[i / 4].isTobeCleaned)
    {
      flag = 1;
      freeElem(i);
      data_->variableList[i / 4].isTobeCleaned = 0;
    }
  }
  pthread_mutex_lock(&data_->lock);
  if (flag)
    compact();
  pthread_mutex_unlock(&data_->lock);
}

void endScope()
{
  Variable *curr = top(&data_->variableStack);
  while (curr != NULL && !isEmpty(&data_->variableStack))
  {
    printf("Marking variable %s for detection\n", curr->name);
    curr->isTobeCleaned = 1;
    curr->type = -1;
    pop(&data_->variableStack);
    curr = top(&data_->variableStack);
  }
  if (curr == NULL)
    pop(&data_->variableStack);
  if (isEmpty(&data_->variableStack))
    clean();
}

void *gc_run(void *p)
{
  while (true)
  {
    usleep(50 * 1000);
    clean();
  }
}

void compact()
{
  printf("Running Compaction...\n");
  int compactIndex = 0;
  for (int i = 0; i < data_->maxMemIndex;)
  {
    int localAddr = data_->actualAddressToLocalAdress[i];
    int len = data_->variableList[localAddr / 4].size;
    if (data_->actualAddressToLocalAdress[i] != -1)
    {
      if (compactIndex != i)
      {
        int *p1 = memory_ + compactIndex;
        p1 = memory_ + i;
        data_->pageTable[localAddr / 4] = p1;
        for (int j = 0; j < len; j++)
        {
          data_->actualAddressToLocalAdress[compactIndex + j] = localAddr;
          data_->actualAddressToLocalAdress[i + j] = -1;
        }
      }
      i += len;
      compactIndex += len;
    }
    else
      i++;
  }
  printf("Compaction Done\n");
}