#include "memlab.h"

int *MemLab::memory_;
Data *MemLab::data_;

int gc,no_gc;
FILE *f1,*f2;

int Variable::isValid(int type, string name) {
  if ((type == INT || type == CHAR || type == MEDIUM_INT || type == BOOLEAN) &&
      name.length() < VAR_NAME_SIZE) {
    return 1;
  }
  return 0;
}

int Variable::getSizeFromType(int type, int arrlen) {
  if (type == INT) {
    return (4 * arrlen + 3) / 4;
  } else if (type == CHAR) {
    return (1 * arrlen + 3) / 4;
  } else if (type == MEDIUM_INT) {
    return (3 * arrlen + 3) / 4;
  } else if (type == BOOLEAN) {
    return (1 * arrlen + 3) / 4;
  }
  return 0;
}

Variable::Variable() {
  type = -1;
  size = 0;
}

Variable::Variable(string name, int type, int localAddr, int arrLen = 1) {
  if (!isValid(type, name)) {
    fprintf(stderr, "Invalid Variable\n");
  }
  this->name = name;
  this->type = type;
  this->isTobeCleaned = 0;
  this->arrLen = arrLen;
  this->localAddress = localAddr;
  this->size = getSizeFromType(type, arrLen);
}

mediumInt::mediumInt(int val) {
  if (val >= (1 << 23) || (val < -(1 << 23))) {
    fprintf(stderr, "Overflow in Medium Int\n");
    exit(1);
  }
  value[0] = (val >> 16) & 0xFF;
  value[1] = (val >> 8) & 0xFF;
  value[2] = val & 0xFF;
}

int mediumInt::toInt() {
  int val = 0;
  if (!(this->value[0] >> 7 == 0)) {
    val |= (((1 << 8) - 1) << 24);
  }
  val |= this->value[2] | (this->value[1] << 8) | (this->value[0] << 16);
  return val;
}

Stack::Stack() { topIndex = -1; }

void Stack::push(Variable *x) {
  if (topIndex == STACK_SIZE - 1) {
    fprintf(stderr, "Global Stack Size Limit Reached. Exitting.\n");
  }
  stck[++topIndex] = x;
}

void Stack::pop() {
  if (topIndex == -1) {
    cout << "Error, stack is Empty!\n";
    return;
  }
  topIndex--;
}

Variable *Stack::top() { return stck[topIndex]; }

bool Stack::isEmpty() {
  if (topIndex == -1) return 1;
  return 0;
}

int Stack::getSize() { return topIndex + 1; }

void MemLab::createMem() {
  cout << "Memory Created\n";
  gc=0;
  no_gc=0;
  f1=fopen("gc.txt","w");
  f2=fopen("no_gc.txt","w");
  memory_ = (int *)malloc(MEM_SIZE+sizeof(Data));
  data_ = (Data *)(memory_+MEM_SIZE/4);

  pthread_mutex_init(&data_->lock, NULL);

  pthread_attr_init(&data_->attr);

  pthread_create(&data_->ptid, &data_->attr, MemLab::gc_run, NULL);

  for (int i = 0; i < MEM_SIZE / 4; i++) {
    data_->actualAddressToLocalAdress[i] = -1;
  }
  data_->localAddress = 0;
  data_->maxMemIndex = -1;
}

int MemLab::createVar(string name, int type) {
  cout << "Creating Variable of name: " << name << " and type "
       << MemLab::getTypeString(type) << "\n";

  if (data_->localAddress / 4 >= NUM_VARIABLES) {
    fprintf(stderr, "Variable count limit reached. Exitting.\n");
    exit(1);
  }

  Variable var = Variable(name, type, data_->localAddress);
  data_->variableList[data_->localAddress / 4] = var;
  no_gc+=var.size;
  gc+=var.size;
  // fprintf(f1,"%d,",gc);
  // fprintf(f2,"%d,",no_gc);
  pthread_mutex_lock(&MemLab::data_->lock);
  for (int i = 0; i < MEM_SIZE / 4; i++) {
    if (data_->actualAddressToLocalAdress[i] == -1) {
      data_->maxMemIndex = max(data_->maxMemIndex, i);
      data_->pageTable[data_->localAddress / 4] = memory_ + i;
      data_->actualAddressToLocalAdress[i] = data_->localAddress;
      break;
    }
  }
  pthread_mutex_unlock(&MemLab::data_->lock);
  int temp = data_->localAddress;
  data_->localAddress += 4;
  data_->variableStack.push(&data_->variableList[temp / 4]);
  return temp;
}

bool MemLab::typeCheck(int localAddress, int type) {
  return (data_->variableList[localAddress / 4].type == type);
}

string MemLab::getTypeString(int type) {
  if (type == INT) {
    return "INT";
  } else if (type == BOOLEAN) {
    return "BOOLEAN";
  } else if (type == CHAR) {
    return "CHAR";
  } else {
    return "MEDIUM_INT";
  }
}

void MemLab::assignVar(int localAddress, int value) {
  cout << "Assigning integer value to variable "
       << MemLab::data_->variableList[localAddress].name << "\n";
  if (!typeCheck(localAddress, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::assignVar(int localAddress, char value) {
  cout << "Assigning char value to variable "
       << MemLab::data_->variableList[localAddress].name << "\n";

  if (!typeCheck(localAddress, CHAR)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::assignVar(int localAddress, bool value) {
  cout << "Assigning boolean value to variable "
       << MemLab::data_->variableList[localAddress].name << "\n";

  if (!typeCheck(localAddress, BOOLEAN)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value;
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::assignVar(int localAddress, mediumInt value) {
  cout << "Assigning medium int value to variable "
       << MemLab::data_->variableList[localAddress].name << "\n";

  if (!typeCheck(localAddress, MEDIUM_INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = value.toInt();
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::addToVar(int localAddress, int value) {
  if (!typeCheck(localAddress, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) + value;
}

void MemLab::multToVar(int localAddress, int value) {
  if (!typeCheck(localAddress, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  *physicalAddress = (*physicalAddress) * value;
}

int MemLab::getValueVarInt(int localAddr) {
  if (!typeCheck(localAddr, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

char MemLab::getValueVarChar(int localAddr) {
  if (!typeCheck(localAddr, CHAR)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

bool MemLab::getValueVarBool(int localAddr) {
  if (!typeCheck(localAddr, BOOLEAN)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return *physicalAddress;
}

mediumInt MemLab::getValueVarMedInt(int localAddr) {
  if (!typeCheck(localAddr, MEDIUM_INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  return mediumInt(*physicalAddress);
}

int MemLab::createArr(string name, int type, int arrLen) {
  cout << "Creating Array of name: " << name << " and type "
       << MemLab::getTypeString(type) << " and length " << arrLen << "\n";

  if (data_->localAddress / 4 >= NUM_VARIABLES) {
    fprintf(stderr, "Variable count limit reached. Exitting.\n");
    exit(1);
  }

  Variable var = Variable(name, type, data_->localAddress, arrLen);
  data_->variableList[data_->localAddress / 4] = var;
  no_gc+=var.size;
  gc+=var.size;
  // fprintf(f1,"%d,",gc);
  // fprintf(f2,"%d,",no_gc);
  int len = var.size;  
  pthread_mutex_lock(&MemLab::data_->lock);
  for (int i = 0; i < MEM_SIZE / 4; i++) {
    int flag = 1;
    for (int j = 0; j < len; j++) {
      if (j + i < MEM_SIZE / 4) {
        if (!(data_->actualAddressToLocalAdress[i + j] == -1)) {
          flag = 0;
          break;
        }
      } else {
        break;
      }
    }
    if (flag == 1) {
      for (int j = 0; j < len; j++) {
        if (j + i < MEM_SIZE / 4) {
          data_->maxMemIndex = max(data_->maxMemIndex, i + j);
          data_->actualAddressToLocalAdress[j + i] = data_->localAddress;
        }
      }
      data_->pageTable[data_->localAddress / 4] = memory_ + i;
      break;
    }
  }
  pthread_mutex_unlock(&MemLab::data_->lock);

  int temp = data_->localAddress;
  data_->localAddress += 4;
  data_->variableStack.push(&data_->variableList[temp / 4]);
  return temp;
}

void MemLab::assignArr(int localAddr, int index, int value) {
  if (!typeCheck(localAddr, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);
  int *physicalAddress = data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::assignArr(int localAddr, int index, char value) {
  if (!typeCheck(localAddr, CHAR)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);
  char *physicalAddress = (char *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&MemLab::data_->lock);
}

void MemLab::assignArr(int localAddr, int index, bool value) {
  if (!typeCheck(localAddr, BOOLEAN)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  bool *physicalAddress = (bool *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value;
  pthread_mutex_unlock(&MemLab::data_->lock);

}

void MemLab::assignArr(int localAddr, int index, mediumInt value) {
  if (!typeCheck(localAddr, MEDIUM_INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  pthread_mutex_lock(&MemLab::data_->lock);

  mediumInt *physicalAddress = (mediumInt *)data_->pageTable[localAddr / 4];
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Assigning value at Invalid Index.\n");
    exit(1);
  }
  physicalAddress[index] = value.toInt();
  pthread_mutex_unlock(&MemLab::data_->lock);

}

int MemLab::getValueArrInt(int localAddr, int index) {
  if (!typeCheck(localAddr, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddr / 4];
  int val=physicalAddress[index];
  return val;
}

char MemLab::getValueArrChar(int localAddr, int index) {
  if (!typeCheck(localAddr, CHAR)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  char *physicalAddress = (char *)data_->pageTable[localAddr / 4];
  char val=physicalAddress[index];  
  return val;
}

bool MemLab::getValueArrBool(int localAddr, int index) {
  if (!typeCheck(localAddr, BOOLEAN)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  bool *physicalAddress = (bool *)data_->pageTable[localAddr / 4];
  return physicalAddress[index];
}

mediumInt MemLab::getValueArrMedInt(int localAddr, int index) {
  if (!typeCheck(localAddr, MEDIUM_INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddr / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  mediumInt *physicalAddress = (mediumInt *)data_->pageTable[localAddr / 4];
  return physicalAddress[index];
}

void MemLab::addToArr(int localAddress, int index, int value) {
  if (!typeCheck(localAddress, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddress / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  physicalAddress[index] = physicalAddress[index] + value;
}

void MemLab::multToArr(int localAddress, int index, int value) {
  if (!typeCheck(localAddress, INT)) {
    fprintf(stderr, "ERROR: Type Mismatch\n");
    exit(1);
  }
  if (index < 0 || index >= data_->variableList[localAddress / 4].arrLen) {
    fprintf(stderr, "Accessing invalid index\n");
    exit(1);
  }
  int *physicalAddress = data_->pageTable[localAddress / 4];
  physicalAddress[index] = physicalAddress[index] * value;
}

void MemLab::freeElem(int locAddr) {
  int len = data_->variableList[locAddr / 4].size;
  int index = data_->pageTable[locAddr / 4] - memory_;
  cout << "Freeing variable " << data_->variableList[locAddr / 4].name << "\n";
  gc-=data_->variableList[locAddr / 4].size;
  // fprintf(f1,"%d,",gc);
  // fprintf(f2,"%d,",no_gc);
  for (int i = 0; i < len; i++) {
    if (index + i < MEM_SIZE / 4) {
      data_->actualAddressToLocalAdress[index + i] = -1;
    } else {
      fprintf(stderr, "Freeing invalid index.\n");
      exit(1);
    }
  }
}

void MemLab::functionStart() { data_->variableStack.push(NULL); }

void clean() {
  int flag = 0;
  for (int i = 0; i < MemLab::data_->localAddress; i += 4) {
    if (MemLab::data_->variableList[i / 4].isTobeCleaned) {
      flag = 1;
      MemLab::freeElem(i);

      MemLab::data_->variableList[i / 4].isTobeCleaned=0;
    }
  }
  pthread_mutex_lock(&MemLab::data_->lock);
  if (flag) MemLab::compact();
  pthread_mutex_unlock(&MemLab::data_->lock);
  // fprintf(f1,"%d,",gc);
  // fprintf(f2,"%d,",no_gc);
}

void MemLab::endScope() {
  Variable *curr = data_->variableStack.top();

  while (curr != NULL && !data_->variableStack.isEmpty()) {
    cout<<"Marking variable "<<curr->name<<" for deletion\n";
    curr->isTobeCleaned = 1;
    curr->type=-1;
    data_->variableStack.pop();
    curr = data_->variableStack.top();
  }
  if (curr == NULL) {
    data_->variableStack.pop();
  }
  if (data_->variableStack.isEmpty()) {
    clean();
  }
}

void *MemLab::gc_run(void *) {
  while (true) {
    usleep(50 * 1000);
    clean();
  }
}

void MemLab::compact() {
  cout << "Running Compaction...\n";
  int compactIndex = 0;
  for (int i = 0; i < data_->maxMemIndex;) {
    int localAddr = data_->actualAddressToLocalAdress[i];
    int len = data_->variableList[localAddr / 4].size;
    if (data_->actualAddressToLocalAdress[i] != -1) {
      if (compactIndex != i) {
        int *p1 = memory_ + compactIndex;
        p1 = memory_ + i;
        data_->pageTable[localAddr / 4] = p1;
        for (int j = 0; j < len; j++) {
          data_->actualAddressToLocalAdress[compactIndex + j] = localAddr;
          data_->actualAddressToLocalAdress[i + j] = -1;
        }
      }
      i += len;
      compactIndex += len;
    } else {
      i++;
    }
  }
  cout << "Compaction Done\n";
}

void debugFreeSpace() {
  for (int i = 0; i < 15; i++) {
    int locAddr = MemLab::data_->actualAddressToLocalAdress[i];
    if (MemLab::data_->actualAddressToLocalAdress[i] == -1) {
      cout << locAddr << " ";
    } else {
      cout << locAddr << " ";
    }
  }
  cout << "\n";
}

void testCreate() {
  MemLab::createMem();

  int loc = MemLab::createVar("Test", INT);

  cout << loc << "\n";

  int loc1 = MemLab::createVar("Test", INT);
  cout << loc1 << "\n";

  debugFreeSpace();

  MemLab::freeElem(loc);

  debugFreeSpace();

  int loc2 = MemLab::createVar("Test", INT);
  cout << loc2 << "\n";

  debugFreeSpace();

  int loc3 = MemLab::createArr("TestArr", INT, 5);

  cout << loc3 << "\n";

  debugFreeSpace();

  int loc4 = MemLab::createVar("TestArr", INT);

  cout << loc4 << "\n";

  debugFreeSpace();

  MemLab::freeElem(loc3);

  debugFreeSpace();

  int loc5 = MemLab::createVar("TestArr", INT);

  cout << loc5 << "\n";

  debugFreeSpace();

  int loc6 = MemLab::createArr("TestArr", INT, 7);

  cout << loc6 << "\n";

  debugFreeSpace();
}

void testAssign() {
  MemLab::createMem();

  int loc = MemLab::createVar("Test", INT);

  cout << loc << "\n";

  MemLab::assignVar(loc, 10);

  cout << MemLab::getValueVarInt(loc) << "\n";

  int loc1 = MemLab::createArr("Test", INT, 10);

  cout << loc1 << "\n";

  MemLab::assignArr(loc1, 5, 15);

  cout << MemLab::getValueArrInt(loc1, 5) << "\n";

  int loc2 = MemLab::createArr("Test", CHAR, 10);

  cout << loc2 << "\n";

  MemLab::assignArr(loc2, 5, 'a');

  cout << MemLab::getValueArrChar(loc2, 5) << "\n";

  int loc3 = MemLab::createVar("Test", BOOLEAN);

  cout << loc3 << "\n";

  MemLab::assignVar(loc3, true);

  cout << MemLab::getValueVarBool(loc3) << "\n";

  int loc4 = MemLab::createVar("Test", MEDIUM_INT);

  cout << loc4 << "\n";

  MemLab::assignVar(loc4, mediumInt(-1512));

  cout << MemLab::getValueVarMedInt(loc4).toInt() << "\n";
}

void testArr() {
  MemLab::createMem();

  int loc1 = MemLab::createArr("Arr1", CHAR, 10);

  debugFreeSpace();

  MemLab::assignArr(loc1, 5, 'a');

  cout << MemLab::getValueArrChar(loc1, 5) << "\n";

  int loc2 = MemLab::createArr("Arr1", BOOLEAN, 10);

  debugFreeSpace();

  MemLab::assignArr(loc2, 5, true);

  cout << MemLab::getValueArrBool(loc2, 5) << "\n";

  MemLab::freeElem(loc1);

  debugFreeSpace();

  int loc3 = MemLab::createArr("Arr1", MEDIUM_INT, 10);

  debugFreeSpace();

  MemLab::assignArr(loc3, 5, mediumInt(52));

  cout << MemLab::getValueArrMedInt(loc3, 5).toInt() << "\n";
}

void testCompact() {
  MemLab::createMem();

  int loc1 = MemLab::createArr("arr1", INT, 5);

  MemLab::assignArr(loc1, 3, 10);

  int loc2 = MemLab::createArr("arr2", CHAR, 10);

  int loc3 = MemLab::createArr("arr2", BOOLEAN, 5);

  MemLab::assignArr(loc3, 2, true);

  MemLab::freeElem(loc2);
  cout << "\n";
  debugFreeSpace();

  MemLab::compact();

  debugFreeSpace();

  cout << MemLab::getValueArrInt(loc1, 3) << "\n";
  cout << MemLab::getValueArrBool(loc3, 2) << "\n";
}

void testScope() {
  MemLab::createMem();

  MemLab::functionStart();

  MemLab::createVar("X", INT);
  MemLab::createVar("Y", INT);
  MemLab::createVar("Z", INT);

  MemLab::endScope();
}

// int main() {
//   // testAssign();
//   // testCreate();
//   // testArr();
//   // testCompact();
//   // testScope();

//   return 0;
// }
