#include "goodmalloc.hxx"
#include <iostream>
#include <map>
using namespace std;

int *memory_;
Data *data_;
int gc, no_gc;
map<char*, int> mp;
int isValid(int type, char *name)
{
  return ((type == INT || type == CHAR || type == LL_INT || type == BOOLEAN) && strlen(name) < VAR_NAME_SIZE);
}

int getSizeFromType(int type, int arrlen)
{
  switch (type)
  {
  case INT:
    return (4 * arrlen + 3) / 4;
  case CHAR:
    return (1 * arrlen + 3) / 4;
  case LL_INT:
    return (4 * arrlen + 3) / 4;
  case BOOLEAN:
    return (1 * arrlen + 3) / 4;
  default:
    return 0;
  }
}

Variable *CreateVariable(char *name, int type, int localAddr, int arrLen)
{
  if (!isValid(type, name))
  {
    fprintf(stderr, "Invalid Variable\n");
    exit(1);
  }
  Variable *a = (Variable *)malloc(sizeof(Variable));
  a->name = name;
  a->type = type;
  a->isTobeCleaned = 0;
  a->arrLen = arrLen;
  a->localAddress = localAddr;
  if(type = LL_INT)
    a->size = arrLen;
  else 
    a->size = getSizeFromType(type, arrLen);
  return a;
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
    ans = "INT";
  else if (type == BOOLEAN)
    ans = "BOOLEAN";
  else if (type == CHAR)
    ans = "CHAR";
  else
    ans = "LINKED_LIST";
  return ans;
}

void assignVal(char* name, int offset, int num, int arr[])
{
  DLL* dll = ((DDL *)data_->pageTable[mp[name]/4]);
  if(((offset + num - dll->curr_sz) > 0))
  {
    printf("OPERATION NOT POSSIBLE\n");
    exit(1);
  }
  Node* temp = dll->list;
  for(int i = 0; i< offset; i++)
  {
    ;
  }
  // for(int i = 0; i < num; i++)
  // {
  //   Node *temp = (Node *)malloc(sizeof(Node));
  //   temp->data = rand() % LIMIT + 1;
  //   temp->next = temp->prev = NULL;
  //   dll->curr_sz++;
  //   if (!(dll->list))
  //     (dll->list) = temp;
  //   else
  //   {
  //     temp->next = dll->list;
  //     (dll->list)->prev = temp;
  //     (dll->list) = temp;
  //   }
  // }
  // printList(((DDL *)data_->pageTable[mp[name]/4])->list, "output_test.txt");
  // printList(dll->list, "output.txt");
}
void List(char *name, int size)
{
  DLL* dll = ((DDL *)data_->pageTable[mp[name]/4]);
  // dll->list
  for(int i = 0; i < size; i++)
  {
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->data = rand() % LIMIT + 1;
    temp->next = temp->prev = NULL;
    dll->curr_sz++;
    if (!(dll->list))
      (dll->list) = temp;
    else
    {
      temp->next = dll->list;
      (dll->list)->prev = temp;
      (dll->list) = temp;
    }
  }
  // printList(((DDL *)data_->pageTable[mp[name]/4])->list, "output_test.txt");
  // printList(dll->list, "output.txt");
}
void addToVal(int localAddress, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) + value;
}

void multToVal(int localAddress, int value)
{
  if (!typeCheck(localAddress, INT))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) * value;
}

void getVal(int localAddr, int type, void *data)
{
  if (!typeCheck(localAddr, type))
  {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  data = &(*physicalAddress);
}

// A utility function to print a doubly linked list in both forward and backward directions
void printList(Node *head, char* out)
{
  FILE* fp = fopen(out, "w");
  Node *temp = head;
  fprintf(fp,"Forward Traversal using next pointer\n");
  while (head)
  {
    fprintf(fp,"%d ", head->data);
    temp = head;
    head = head->next;
  }
  fprintf(fp,"\nBackward Traversal using prev pointer\n");
  while (temp)
  {
    fprintf(fp,"%d ", temp->data);
    temp = temp->prev;
  }
  fclose(fp);
}

int createList(char *name, int type, int sz)
{
  printf("Creating List of name: %s and type %s and length %d\n", name, getTypeString(type), sz);
  DLL *dll = (DLL *)malloc(sizeof(DLL));
  dll->name = name;
  dll->sz = sz;
  dll->curr_sz = 0;
  dll->list = (Node *)malloc(sz * sizeof(Node));
  // Add the newly created DLL to the Data variable
  Variable *var = CreateVariable(name, type, data_->localAddress, sz);
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
  dll = ((DDL *)data_->pageTable[mp[name]/4]);
  int temp = data_->localAddress;
  data_->localAddress += 4;
  push(&data_->variableStack, &data_->variableList[temp / 4]);
  mp[name] = temp;
  List(name, sz);
  return temp;
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